#include <ioa/udp_sender_automaton.hpp>

#include <iostream>

namespace ioa {

  udp_sender_automaton::udp_sender_automaton (const size_t send_buf_size) :
    m_state (SEND_WAIT),
    m_errno (0)
  {
    add_observable (&send);
    add_observable (&send_complete);

    const int val = 1;
    int res;
    int flags;

    // Open a socket.
    m_fd = socket (AF_INET, SOCK_DGRAM, 0);
    if (m_fd == -1) {
      m_errno = errno;
      return;
    }
      
    // Get the flags.
    flags = fcntl (m_fd, F_GETFL, 0);
    if (flags < 0) {
      m_errno = errno;
      close (m_fd);
      m_fd = -1;
      return;
    }

    // Set non-blocking.
    flags |= O_NONBLOCK;
    res = fcntl (m_fd, F_SETFL, flags);
    if (res < 0) {
      m_errno = errno;
      close (m_fd);
      m_fd = -1;
      return;
    }

    // Set broadcasting.
    if (setsockopt (m_fd, SOL_SOCKET, SO_BROADCAST, &val, sizeof (val)) == -1) {
      m_errno = errno;
      close (m_fd);
      m_fd = -1;
      return;
    }

    if (send_buf_size != 0) {
      if (setsockopt (m_fd, SOL_SOCKET, SO_SNDBUF, &send_buf_size, sizeof (send_buf_size)) == -1) {
	m_errno = errno;
	close (m_fd);
	m_fd = -1;
	return;
      }
    }
  }

  udp_sender_automaton::~udp_sender_automaton () {
    if (m_fd != -1) {
      close (m_fd);
    }
  }

  void udp_sender_automaton::schedule () const {
    if (schedule_write_ready_precondition ()) {
      ioa::schedule (&udp_sender_automaton::schedule_write_ready);
    }
    // We do not schedule send_complete because of the auto parameter.
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

  void udp_sender_automaton::purge (const aid_t aid) {
    if (m_send_set.count (aid) != 0) {
      std::list<std::pair<aid_t, send_arg> >::iterator pos = std::find_if (m_send_queue.begin (),
									   m_send_queue.end (),
									   first_aid_equal (aid));
      assert (pos != m_send_queue.end ());
      m_send_queue.erase (pos);
      m_send_set.erase (aid);
    }

    m_complete_map.erase (aid);
  }

  void udp_sender_automaton::add_to_send_queue (const aid_t aid,
						const send_arg& arg) {
    m_send_queue.push_back (std::make_pair (aid, arg));
    m_send_set.insert (aid);
    if (m_state == SEND_WAIT) {
      m_state = SCHEDULE_WRITE_READY;
    }
  }

  void udp_sender_automaton::add_to_complete_map (const aid_t aid,
						  const int err_no) {
    m_complete_map.insert (std::make_pair (aid, m_errno));
    // Schedule the send complete.
    ioa::schedule (&udp_sender_automaton::send_complete, aid);
  }

  void udp_sender_automaton::send_effect (const send_arg& arg,
					  aid_t aid) {
    // Ignore if aid has an outstanding send or send_complete.
    if (m_send_set.count (aid) == 0 && m_complete_map.count (aid) == 0) {
      if (m_fd == -1) {
	// Error.  Add to the complete queue.
	add_to_complete_map (aid, m_errno);
      }
      else if (arg.address.get_errno () != 0) {
	// Bad address.
	add_to_complete_map (aid, EINVAL);
      }
      else if (arg.buffer.get () == 0) {
	// No data.  Succeed immediately.
	add_to_complete_map (aid, 0);
      }
      else {
	// No error.  Add to the send queue and set.
	add_to_send_queue (aid, arg);
      }
    }
  }

  bool udp_sender_automaton::schedule_write_ready_precondition () const {
    return m_state == SCHEDULE_WRITE_READY;
  }

  void udp_sender_automaton::schedule_write_ready_effect () {
    ioa::schedule_write_ready (&udp_sender_automaton::write_ready, m_fd);
    m_state = WRITE_READY_WAIT;
  }

  bool udp_sender_automaton::write_ready_precondition () const {
    return m_state == WRITE_READY_WAIT;
  }

  void udp_sender_automaton::write_ready_effect () {
    if (!m_send_queue.empty ()) {
      std::pair<aid_t, send_arg> item = m_send_queue.front ();
      m_send_queue.pop_front ();
      m_send_set.erase (item.first);
      
      if (sendto (m_fd, item.second.buffer->data (), item.second.buffer->size (), 0, item.second.address.get_sockaddr (), item.second.address.get_socklen ()) != -1) {
	// Success.
	add_to_complete_map (item.first, 0);
      }
      else {
	// Fail.
	m_errno = errno;
	close (m_fd);
	m_fd = -1;
	add_to_complete_map (item.first, m_errno);
	
	// Drain the send queue.
	for (std::list<std::pair<aid_t, send_arg> >::const_iterator pos = m_send_queue.begin ();
	     pos != m_send_queue.end ();
	     ++pos) {
	  add_to_complete_map (pos->first, m_errno);
	}
	m_send_queue.clear ();
	m_send_set.clear ();
      }
    }
      
    if (m_send_queue.empty ()) {
      // No more messages to send.
      m_state = SEND_WAIT;
    }
    else {
      // Go again.
      m_state = SCHEDULE_WRITE_READY;
    }
  }
    
  bool udp_sender_automaton::send_complete_precondition (aid_t aid) const {
    return m_complete_map.count (aid) != 0 && 
      binding_count (&udp_sender_automaton::send_complete, aid) != 0;
  }

  int udp_sender_automaton::send_complete_effect (aid_t aid) {
    int retval = m_complete_map[aid];
    m_complete_map.erase (aid);
    return retval;
  }

}
