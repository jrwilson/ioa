#include <ioa/tcp_connector_automaton.hpp>

namespace ioa {

  tcp_connector_automaton::tcp_connector_automaton () {
    add_observable (&connect_complete);
  }

  tcp_connector_automaton::~tcp_connector_automaton () {
    for (std::map<aid_t, int>::const_iterator pos = m_aid_to_fd.begin ();
	 pos != m_aid_to_fd.end ();
	 ++pos) {
      close (pos->second);
    }
  }

  void tcp_connector_automaton::schedule () const { }

  void tcp_connector_automaton::observe (observable* o) {
    if (o == &connect_complete) {
      if (connect_complete.recent_op == UNBOUND) {
	aid_t aid = connect_complete.recent_parameter;
	// We need to purge aid.
	if (m_aid_to_fd.count (aid) != 0) {
	  close (m_aid_to_fd[aid]);
	  m_aid_to_fd.erase (aid);
	}
	if (m_aid_to_manager.count (aid) != 0) {
	  automaton_manager<tcp_connection_automaton>* manager = m_aid_to_manager[aid];
	  m_aid_to_manager.erase (aid);
	  m_manager_to_aid.erase (manager);
	  remove_observable (manager);
	  manager->destroy ();
	}
	if (m_aid_to_connect.count (aid) != 0) {
	  m_aid_to_connect.erase (aid);
	}
      }
    }
    else {
      automaton_manager<tcp_connection_automaton>* conn = dynamic_cast<automaton_manager<tcp_connection_automaton> *> (o);
      assert (conn != 0);
      assert (conn->get_handle () != -1);
	
      aid_t aid = m_manager_to_aid[conn];
      m_manager_to_aid.erase (conn);
      m_aid_to_manager.erase (aid);
      remove_observable (conn);
      m_aid_to_connect.insert (std::make_pair (aid, connect_val (0, conn->get_handle ())));
      ioa::schedule (&tcp_connector_automaton::connect_complete, aid);
    }
  }

  void tcp_connector_automaton::connect_effect (const inet_address& address,
		       aid_t aid) {
    // Open a socket.
    int fd = socket (AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
      m_aid_to_connect.insert (std::make_pair (aid, connect_val (errno, -1)));
      ioa::schedule (&tcp_connector_automaton::connect_complete, aid);
      return;
    }
      
    // Get the flags.
    int flags = fcntl (fd, F_GETFL, 0);
    if (flags < 0) {
      m_aid_to_connect.insert (std::make_pair (aid, connect_val (errno, -1)));
      ioa::schedule (&tcp_connector_automaton::connect_complete, aid);
      return;
    }
      
    // Set non-blocking.
    flags |= O_NONBLOCK;
    if (fcntl (fd, F_SETFL, flags) == -1) {
      m_aid_to_connect.insert (std::make_pair (aid, connect_val (errno, -1)));
      ioa::schedule (&tcp_connector_automaton::connect_complete, aid);
      return;
    }

    if (::connect (fd, address.get_sockaddr (), address.get_socklen ()) != -1) {
      automaton_manager<tcp_connection_automaton>* conn = new automaton_manager<tcp_connection_automaton> (this, make_generator<tcp_connection_automaton> (m_aid_to_fd[aid]));
      add_observable (conn);
      m_manager_to_aid.insert (std::make_pair (conn, aid));
      m_aid_to_manager.insert (std::make_pair (aid, conn));
      return;
    }
    else {
      if (errno == EINPROGRESS) {
	// Asynchronous connect.
	m_aid_to_fd.insert (std::make_pair (aid, fd));
	ioa::schedule_write_ready (&ioa::tcp_connector_automaton::write_ready, aid, fd);
      }
      else {
	m_aid_to_connect.insert (std::make_pair (aid, connect_val (errno, -1)));
	ioa::schedule (&tcp_connector_automaton::connect_complete, aid);
	return;
      }
    }
  }

  bool tcp_connector_automaton::write_ready_precondition (aid_t aid) const {
    return m_aid_to_fd.count (aid) != 0;
  }

  void tcp_connector_automaton::write_ready_effect (aid_t aid) {
    automaton_manager<tcp_connection_automaton>* conn = new automaton_manager<tcp_connection_automaton> (this, make_generator<tcp_connection_automaton> (m_aid_to_fd[aid]));
    m_aid_to_fd.erase (aid);
    add_observable (conn);
    m_manager_to_aid.insert (std::make_pair (conn, aid));
    m_aid_to_manager.insert (std::make_pair (aid, conn));
  }

  bool tcp_connector_automaton::connect_complete_precondition (aid_t aid) const {
    return m_aid_to_connect.count (aid) != 0 && 
      binding_count (&tcp_connector_automaton::connect_complete, aid) != 0;
  }

  tcp_connector_automaton::connect_val tcp_connector_automaton::connect_complete_effect (aid_t aid) {
    connect_val retval (m_aid_to_connect.find (aid)->second);
    m_aid_to_connect.erase (aid);
    return retval;
  }

}
