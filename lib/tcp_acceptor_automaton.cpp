#include <ioa/tcp_acceptor_automaton.hpp>

#include <fcntl.h>

namespace ioa {

  void tcp_acceptor_automaton::schedule () const {
    if (accept_precondition ()) {
      ioa::schedule (&tcp_acceptor_automaton::accept);
    }
    if (error_precondition ()) {
      ioa::schedule (&tcp_acceptor_automaton::error);
    }
    if (schedule_read_ready_precondition ()) {
      ioa::schedule (&tcp_acceptor_automaton::schedule_read_ready);
    }
  }

  void tcp_acceptor_automaton::observe (observable* o) {
    automaton_manager<tcp_connection_automaton>* connection = static_cast<automaton_manager<tcp_connection_automaton>* > (o);

    switch (connection->get_state ()) {
    case automaton_manager_interface::CREATED:
      {
	inet_address address = m_address_map[connection];
	m_address_map.erase (connection);
	m_accept_queue.push (std::make_pair (address, connection));
	remove_observable (connection);
      }
      break;
    default:
      // Do nothing.
      break;
    }
    
    schedule ();
  }

  tcp_acceptor_automaton::tcp_acceptor_automaton (const ioa::inet_address& address,
						  const int backlog) :
    m_state (SCHEDULE_READ_READY),
    m_fd (-1),
    m_errno (0),
    m_error_reported (false),
    m_self (get_aid ())
  {
    try {
      // Open a socket.
      m_fd = socket (AF_INET, SOCK_STREAM, 0);
      if (m_fd == -1) {
	m_errno = errno;
	return;
      }
      
      // Get the flags.
      int flags = fcntl (m_fd, F_GETFL, 0);
      if (flags < 0) {
	m_errno = errno;
	return;
      }
      
      // Set non-blocking.
      flags |= O_NONBLOCK;
      if (fcntl (m_fd, F_SETFL, flags) == -1) {
	m_errno = errno;
	return;
      }
      
      const int val = 1;
#ifdef SO_REUSEADDR
      // Set reuse.
      if (setsockopt (m_fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof (val)) == -1) {
	m_errno = errno;
	return;
      }
#endif
      
#ifdef SO_REUSEPORT
      // Set reuse.
      if (setsockopt (m_fd, SOL_SOCKET, SO_REUSEPORT, &val, sizeof (val)) == -1) {
	m_errno = errno;
	return;
      }
#endif

      // Bind.
      if (::bind (m_fd, address.get_sockaddr (), address.get_socklen ()) == -1) {
	m_errno = errno;
	return;
      }

      // Listen.
      if (listen (m_fd, backlog) == -1) {
	m_errno = errno;
	return;
      }
    } catch (...) { }

    schedule ();
  }

  tcp_acceptor_automaton::~tcp_acceptor_automaton () {
    if (m_fd != -1) {
      close (m_fd);
    }
  }

  bool tcp_acceptor_automaton::accept_precondition () const {
    return !m_accept_queue.empty ();
  }
    
  tcp_acceptor_automaton::accept_val tcp_acceptor_automaton::accept_effect () {
    std::pair<inet_address, automaton_manager<tcp_connection_automaton>*> val = m_accept_queue.front ();
    m_accept_queue.pop ();

    if (binding_count (&tcp_acceptor_automaton::accept) == 0) {
      // No will ever see this connection so destroy it.
      val.second->destroy ();
    }

    return accept_val (val.first, val.second->get_handle ());
  }

  void tcp_acceptor_automaton::accept_schedule () const {
    schedule ();
  }

  bool tcp_acceptor_automaton::error_precondition () const {
    return m_errno != 0 && m_error_reported == false && binding_count (&tcp_acceptor_automaton::error) != 0;
  }

  int tcp_acceptor_automaton::error_effect () {
    m_error_reported = true;
    return m_errno;
  }
  
  void tcp_acceptor_automaton::error_schedule () const {
    schedule ();
  }

  bool tcp_acceptor_automaton::schedule_read_ready_precondition () const {
    return m_errno == 0 && m_state == SCHEDULE_READ_READY;
  }

  void tcp_acceptor_automaton::schedule_read_ready_effect () {
    ioa::schedule_read_ready (&tcp_acceptor_automaton::read_ready, m_fd);
    m_state = READ_READY_WAIT;
  }

  void tcp_acceptor_automaton::schedule_read_ready_schedule () const {
    schedule ();
  }

  bool tcp_acceptor_automaton::read_ready_precondition () const {
    return m_state == READ_READY_WAIT;
  }

  void tcp_acceptor_automaton::read_ready_effect () {
    m_state = SCHEDULE_READ_READY;

    inet_address address;
    int connection_fd = ::accept (m_fd, address.get_sockaddr_ptr (), address.get_socklen_ptr ());
    if (connection_fd != -1) {
      // Create a new connection.
      automaton_manager<tcp_connection_automaton>* connection = new automaton_manager<tcp_connection_automaton> (this, make_generator<tcp_connection_automaton> (connection_fd));
      // Save the address.
      m_address_map.insert (std::make_pair (connection, address));
      // Observe it so we know when its created.
      add_observable (connection);

      make_binding_manager (this,
			    connection, &tcp_connection_automaton::closed,
			    &m_self, &tcp_acceptor_automaton::closed, connection);
    }
    else {
      m_errno = errno;
    }
  }

  void tcp_acceptor_automaton::read_ready_schedule () const {
    schedule ();
  }

  void tcp_acceptor_automaton::closed_effect (automaton_manager<tcp_connection_automaton>* connection) {
    connection->destroy ();
  }

  void tcp_acceptor_automaton::closed_schedule (automaton_manager<tcp_connection_automaton>*) const {
    schedule ();
  }

}
