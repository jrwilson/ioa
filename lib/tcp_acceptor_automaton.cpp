#include <ioa/tcp_acceptor_automaton.hpp>

namespace ioa {

  tcp_acceptor_automaton::tcp_acceptor_automaton (const ioa::inet_address& address) :
    m_state (SCHEDULE_READ_READY),
    m_errno (-1)
  {
    add_observable (&accept);

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
      close (m_fd);
      m_fd = -1;
      return;
    }
      
    // Set non-blocking.
    flags |= O_NONBLOCK;
    if (fcntl (m_fd, F_SETFL, flags) == -1) {
      m_errno = errno;
      close (m_fd);
      m_fd = -1;
      return;
    }
      
    const int val = 1;
#ifdef SO_REUSEADDR
    // Set reuse.
    if (setsockopt (m_fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof (val)) == -1) {
      m_errno = errno;
      close (m_fd);
      m_fd = -1;
      return;
    }
#endif
      
#ifdef SO_REUSEPORT
    // Set reuse.
    if (setsockopt (m_fd, SOL_SOCKET, SO_REUSEPORT, &val, sizeof (val)) == -1) {
      m_errno = errno;
      close (m_fd);
      m_fd = -1;
      return;
    }
#endif

    // Bind.
    if (::bind (m_fd, address.get_sockaddr (), address.get_socklen ()) == -1) {
      m_errno = errno;
      close (m_fd);
      m_fd = -1;
      return;
    }

    // Listen.
    if (listen (m_fd, BACKLOG) == -1) {
      m_errno = errno;
      close (m_fd);
      m_fd = -1;
      return;
    }

    schedule ();
  }

  tcp_acceptor_automaton::~tcp_acceptor_automaton () {
    if (m_fd != -1) {
      close (m_fd);
    }
  }

  void tcp_acceptor_automaton::schedule () const {
    if (schedule_read_ready_precondition ()) {
      ioa::schedule (&tcp_acceptor_automaton::schedule_read_ready);
    }
    if (create_precondition ()) {
      ioa::schedule (&tcp_acceptor_automaton::create);
    }
    if (accept_precondition ()) {
      ioa::schedule (&tcp_acceptor_automaton::accept);
    }
  }

  void tcp_acceptor_automaton::observe (observable* o) {
    if (o == &accept && accept.recent_op == BOUND && m_fd == -1 && m_state == SCHEDULE_READ_READY) {
      // An automaton has bound to accept but we will never accept because m_fd is bad.
      // Generate an error.
      m_state = ACCEPT_READY;
    }
    else if (o == m_connection) {
      // Sanity checks.
      assert (m_state == CREATED_WAIT);
      assert (m_connection->get_handle () != -1);
	
      // We don't need to observe it anymore.
      remove_observable (m_connection);
      m_handle = m_connection->get_handle ();
      m_state = ACCEPT_READY;
    }

    schedule ();
  }

  bool tcp_acceptor_automaton::schedule_read_ready_precondition () const {
    return m_state == SCHEDULE_READ_READY && m_fd != -1;
  }

  void tcp_acceptor_automaton::schedule_read_ready_effect () {
    ioa::schedule_read_ready (&tcp_acceptor_automaton::read_ready, m_fd);
    m_state = READ_READY_WAIT;
  }

  bool tcp_acceptor_automaton::read_ready_precondition () const {
    return m_state == READ_READY_WAIT;
  }

  void tcp_acceptor_automaton::read_ready_effect () {
    m_connection_fd = ::accept (m_fd, m_address.get_sockaddr_ptr (), m_address.get_socklen_ptr ());
    if (m_connection_fd == -1) {
      m_errno = errno;
      close (m_fd);
      m_fd = -1;
      m_address = inet_address ();
      m_handle = -1;
      m_state = ACCEPT_READY;
      return;
    }

    m_state = CREATE_READY;
  }

  bool tcp_acceptor_automaton::create_precondition () const {
    return m_state == CREATE_READY;
  }

  void tcp_acceptor_automaton::create_effect () {
    m_connection = new automaton_manager<tcp_connection_automaton> (this, make_generator<tcp_connection_automaton> (m_connection_fd));
    // Observe it so we know when its created.
    add_observable (m_connection);
    m_state = CREATED_WAIT;
  }

  bool tcp_acceptor_automaton::accept_precondition () const {
    return m_state == ACCEPT_READY && ioa::bind_count (&tcp_acceptor_automaton::accept) != 0;
  }
    
  tcp_acceptor_automaton::accept_val tcp_acceptor_automaton::accept_effect () {
    m_state = SCHEDULE_READ_READY;
    return accept_val (m_errno, m_address, m_handle);
  }

}
