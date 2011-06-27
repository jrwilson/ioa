#include <ioa/udp_sender_automaton.hpp>

namespace ioa {

  udp_sender_automaton::udp_sender_automaton () :
    m_state (SCHEDULE_WRITE_READY),
    m_fd (-1)
  {
    const int val = 1;
    int res;
    int flags;

    // Open a socket.
    m_fd = socket (AF_INET, SOCK_DGRAM, 0);
    if (m_fd == -1) {
      m_errno = errno;
      goto the_end;
    }
      
    // Get the flags.
    flags = fcntl (m_fd, F_GETFL, 0);
    if (flags < 0) {
      m_errno = errno;
      goto the_end;
    }

    // Set non-blocking.
    flags |= O_NONBLOCK;
    res = fcntl (m_fd, F_SETFL, flags);
    if (res < 0) {
      m_errno = errno;
      goto the_end;
    }

    // Set broadcasting.
    if (setsockopt (m_fd, SOL_SOCKET, SO_BROADCAST, &val, sizeof (val)) == -1) {
      m_errno = errno;
      goto the_end;
    }
      
    add_observable (&send);
    add_observable (&send_complete);
      
    // Success.
    m_errno = 0;

  the_end:	
    if (m_errno != 0) {
      if (m_fd != -1) {
	close (m_fd);
	m_fd = -1;
      }
    }
  }

  udp_sender_automaton::~udp_sender_automaton () {
    if (m_fd != -1) {
      close (m_fd);
    }

    for (std::list<std::pair<aid_t, send_arg*> >::const_iterator pos = m_send_queue.begin ();
	 pos != m_send_queue.end ();
	 ++pos) {
      delete pos->second;
    }
  }

  void udp_sender_automaton::add_to_send_queue (const aid_t aid,
						const send_arg& arg) {
    m_send_queue.push_back (std::make_pair (aid, new send_arg (arg)));
    m_send_set.insert (aid);
  }

  void udp_sender_automaton::add_to_complete_map (const aid_t aid,
						  const int err_no) {
    m_complete_map.insert (std::make_pair (aid, m_errno));
    ioa::schedule (&udp_sender_automaton::send_complete, aid);
  }

  void udp_sender_automaton::send_effect (const send_arg& arg, aid_t aid) {
    // Ignore if aid has an outstanding send or complete.
    if (m_send_set.count (aid) == 0 && m_complete_map.count (aid) == 0) {
      if (m_fd != -1) {
	// No error.  Add to the send queue and set.
	add_to_send_queue (aid, arg);
	if (m_state == SCHEDULE_WRITE_READY) {
	  ioa::schedule_write_ready (&udp_sender_automaton::write, m_fd);
	  m_state = WRITE_WAIT;
	}
      }
      else {
	// Error.  Add to the complete queue.
	add_to_complete_map (aid, m_errno);
      }
    }
  }

  // Treat like an input.
  bool udp_sender_automaton::write_precondition () const {
    return true;
  }

  void udp_sender_automaton::write_effect () {
    std::pair<aid_t, send_arg*> item = m_send_queue.front ();
    m_send_queue.pop_front ();
    m_send_set.erase (item.first);
      
    if (item.second->address.get_errno () == 0) {
      sendto (m_fd, item.second->buffer.data (), item.second->buffer.size (), 0, item.second->address.get_sockaddr (), item.second->address.get_socklen ());
      // We don't check a return value because we always return errno.
      add_to_complete_map (item.first, errno);
    }
    else {
      // Bad address.
      add_to_complete_map (item.first, EINVAL);
    }

    m_state = SCHEDULE_WRITE_READY;
      
    delete item.second;
  }
    
  bool udp_sender_automaton::send_complete_precondition (aid_t aid) const {
    return m_complete_map.count (aid) != 0 && 
      bind_count (&udp_sender_automaton::send_complete, aid) != 0;
  }

  int udp_sender_automaton::send_complete_effect (aid_t aid) {
    int retval = m_complete_map[aid];
    m_complete_map.erase (aid);
    return retval;
  }

  void udp_sender_automaton::purge (const aid_t aid) {
    if (m_send_set.count (aid) != 0) {
      std::list<std::pair<aid_t, send_arg*> >::iterator pos = std::find_if (m_send_queue.begin (),
									    m_send_queue.end (),
									    first_aid_equal (aid));
      assert (pos != m_send_queue.end ());
      delete pos->second;
      m_send_queue.erase (pos);
      m_send_set.erase (aid);
    }

    m_complete_map.erase (aid);
  }

  void udp_sender_automaton::observe (observable* o) {
    // Purge the automata that are no longer bound.
    if (o == &send && send.recent_op == UNBOUND) {
      purge (send.recent_parameter);
    }
    else if (o == &send_complete && send_complete.recent_op == UNBOUND) {
      purge (send_complete.recent_parameter);
    }
  }

  void udp_sender_automaton::schedule () const {
  }

}
