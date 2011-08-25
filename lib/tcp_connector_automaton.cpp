#include <ioa/tcp_connector_automaton.hpp>

#include <fcntl.h>

namespace ioa {

  void tcp_connector_automaton::schedule () const {
    if (connect_precondition ()) {
      ioa::schedule (&tcp_connector_automaton::connect);
    }
    if (error_precondition ()) {
      ioa::schedule (&tcp_connector_automaton::error);
    }
  }

  void tcp_connector_automaton::observe (observable* o) {
    schedule ();
  }

  tcp_connector_automaton::tcp_connector_automaton (const inet_address& address) :
    m_self (get_aid ()),
    m_fd (-1),
    m_connection (0),
    m_connection_reported (false),
    m_errno (0),
    m_error_reported (false) {
    try {
      // Open a socket.
      m_fd = socket (AF_INET, SOCK_STREAM, 0);
      if (m_fd == -1) {
	m_errno = errno;
	throw;
      }
      
      // Get the flags.
      int flags = fcntl (m_fd, F_GETFL, 0);
      if (flags < 0) {
	m_errno = errno;
	throw;
      }
      
      // Set non-blocking.
      flags |= O_NONBLOCK;
      if (fcntl (m_fd, F_SETFL, flags) == -1) {
	m_errno = errno;
	throw;
      }
      
      if (::connect (m_fd, address.get_sockaddr (), address.get_socklen ()) != -1) {
	m_connection = new automaton_manager<tcp_connection_automaton> (this, make_generator<tcp_connection_automaton> (m_fd));
	m_fd = -1;
	add_observable (m_connection);
	make_binding_manager (this,
			      m_connection, &tcp_connection_automaton::closed,
			      &m_self, &tcp_connector_automaton::closed);

	throw;
      }
      else {
	if (errno == EINPROGRESS) {
	  // Asynchronous connect.
	  ioa::schedule_write_ready (&ioa::tcp_connector_automaton::write_ready, m_fd);
	}
	else {
	  m_errno = errno;
	  throw;
	}
      }
    } catch (...) { }

    schedule ();
  }

  tcp_connector_automaton::~tcp_connector_automaton () {
    if (m_fd != -1) {
      close (m_fd);
    }
  }

  bool tcp_connector_automaton::connect_precondition () const {
    return m_connection_reported == false &&
      m_connection != 0 &&
      m_connection->get_handle () != -1 && 
      binding_count (&tcp_connector_automaton::connect) != 0;
  }

  automaton_handle<tcp_connection_automaton> tcp_connector_automaton::connect_effect () {
    m_connection_reported = true;
    return m_connection->get_handle ();
  }

  void tcp_connector_automaton::connect_schedule () const {
    schedule ();
  }

  bool tcp_connector_automaton::error_precondition () const {
    return m_errno != 0 && m_error_reported == false && binding_count (&tcp_connector_automaton::error) != 0;
  }

  int tcp_connector_automaton::error_effect () {
    m_error_reported = true;
    return m_errno;
  }
  
  void tcp_connector_automaton::error_schedule () const {
    schedule ();
  }

  bool tcp_connector_automaton::write_ready_precondition () const {
    return true;
  }

  void tcp_connector_automaton::write_ready_effect () {
    // See if the connection succeeded.
    int val;
    socklen_t sz = sizeof (val);
    if (getsockopt (m_fd, SOL_SOCKET, SO_ERROR, &val, &sz) == -1) {
      m_errno = errno;
      return;
    }

    if (val == 0) {
      // Success.
      m_connection = new automaton_manager<tcp_connection_automaton> (this, make_generator<tcp_connection_automaton> (m_fd));
      m_fd = -1;
      add_observable (m_connection);
      make_binding_manager (this,
			    m_connection, &tcp_connection_automaton::closed,
			    &m_self, &tcp_connector_automaton::closed);
    }
    else {
      m_errno = val;
    }
  }

  void tcp_connector_automaton::write_ready_schedule () const {
    schedule ();
  }

  void tcp_connector_automaton::closed_effect () {
    m_connection->destroy ();
  }

  void tcp_connector_automaton::closed_schedule () const {
    schedule ();
  }

}
