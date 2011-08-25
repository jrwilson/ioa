#include <ioa/udp_sender_automaton.hpp>

#include <algorithm>
#include <fcntl.h>

namespace ioa {

  void udp_sender_automaton::schedule () const {
    if (schedule_write_ready_precondition ()) {
      ioa::schedule (&udp_sender_automaton::schedule_write_ready);
    }
    if (error_precondition ()) {
      ioa::schedule (&udp_sender_automaton::error);
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
    
    m_complete_set.erase (aid);
  }

  void udp_sender_automaton::add_to_complete_set (const aid_t aid) {
    m_complete_set.insert (aid);
    // Schedule the send complete.
    ioa::schedule (&udp_sender_automaton::send_complete, aid);
  }

  udp_sender_automaton::udp_sender_automaton (const size_t send_buf_size) :
    m_state (SCHEDULE_WRITE_READY),
    m_fd (-1),
    m_errno (0),
    m_error_reported (false)
  {
    add_observable (&send);
    add_observable (&send_complete);

    const int val = 1;
    int res;
    int flags;

    try {
      // Open a socket.
      m_fd = socket (AF_INET, SOCK_DGRAM, 0);
      if (m_fd == -1) {
	m_errno = errno;
	throw;
      }
      
      // Get the flags.
      flags = fcntl (m_fd, F_GETFL, 0);
      if (flags < 0) {
	m_errno = errno;
	throw;
      }
      
      // Set non-blocking.
      flags |= O_NONBLOCK;
      res = fcntl (m_fd, F_SETFL, flags);
      if (res < 0) {
	m_errno = errno;
	throw;
      }
      
      // Set broadcasting.
      if (setsockopt (m_fd, SOL_SOCKET, SO_BROADCAST, &val, sizeof (val)) == -1) {
	m_errno = errno;
	throw;
      }
      
      if (send_buf_size != 0) {
	if (setsockopt (m_fd, SOL_SOCKET, SO_SNDBUF, &send_buf_size, sizeof (send_buf_size)) == -1) {
	  m_errno = errno;
	  throw;
	}
      }
    } catch (...) { }

    schedule ();
  }

  udp_sender_automaton::~udp_sender_automaton () {
    if (m_fd != -1) {
      close (m_fd);
    }
  }

  void udp_sender_automaton::send_effect (const send_arg& arg,
					  aid_t aid) {
    // There is no error, no outstanding send, and no outstanding complete.
    if (m_errno == 0 &&
	m_send_set.count (aid) == 0 &&
	m_complete_set.count (aid) == 0) {
      // Succeed immediately for a bad address or no data.
      // Both of these can be prevented by the user.
      if (arg.address.get_errno () != 0 || arg.buffer.get () == 0) {
	add_to_complete_set (aid);
      }
      else {
	// No error.  Add to the send queue and set.
	m_send_queue.push_back (std::make_pair (aid, arg));
	m_send_set.insert (aid);
      }
    }
  }

  void udp_sender_automaton::send_schedule (aid_t) const {
    schedule ();
  }

  bool udp_sender_automaton::send_complete_precondition (aid_t aid) const {
    return m_complete_set.count (aid) != 0 && 
      binding_count (&udp_sender_automaton::send_complete, aid) != 0;
  }

  void udp_sender_automaton::send_complete_effect (aid_t aid) {
    m_complete_set.erase (aid);
  }

  void udp_sender_automaton::send_complete_schedule (aid_t) const {
    schedule ();
  }

  bool udp_sender_automaton::error_precondition () const {
    return m_errno != 0 && m_error_reported == false && binding_count (&udp_sender_automaton::error) != 0;
  }

  int udp_sender_automaton::error_effect () {
    m_error_reported = true;
    return m_errno;
  }
  
  void udp_sender_automaton::error_schedule () const {
    schedule ();
  }

  bool udp_sender_automaton::schedule_write_ready_precondition () const {
    // No error and something in the send queue.
    return m_errno == 0 && !m_send_queue.empty () && m_state == SCHEDULE_WRITE_READY;
  }

  void udp_sender_automaton::schedule_write_ready_effect () {
    ioa::schedule_write_ready (&udp_sender_automaton::write_ready, m_fd);
    m_state = WRITE_READY_WAIT;
  }

  void udp_sender_automaton::schedule_write_ready_schedule () const {
    schedule ();
  }

  bool udp_sender_automaton::write_ready_precondition () const {
    return m_state == WRITE_READY_WAIT;
  }

  void udp_sender_automaton::write_ready_effect () {
    m_state = SCHEDULE_WRITE_READY;

    if (!m_send_queue.empty ()) {
      std::pair<aid_t, send_arg> item = m_send_queue.front ();
      m_send_queue.pop_front ();
      m_send_set.erase (item.first);
      
      if (sendto (m_fd, item.second.buffer->data (), item.second.buffer->size (), 0, item.second.address.get_sockaddr (), item.second.address.get_socklen ()) != -1) {
	// Success.
	add_to_complete_set (item.first);
      }
      else {
	// Fail.
	m_errno = errno;
      }
    }
  }

  void udp_sender_automaton::write_ready_schedule () const {
    schedule ();
  }

}
