#ifndef __udp_broadcast_sender_automaton_hpp__
#define __udp_broadcast_sender_automaton_hpp__

#include <ioa/ioa.hpp>
#include <ioa/buffer.hpp>
#include <ioa/inet_address.hpp>

#include <fcntl.h>

#include <list>
#include <algorithm>

namespace ioa {

  class udp_broadcast_sender_automaton :
    public automaton,
    private observer
  {
  public:
    struct send_arg {
      inet_address address;
      ioa::buffer buffer;

      send_arg (const inet_address& a,
		const ioa::buffer& b) :
	address (a),
	buffer (b)
      { }

      send_arg (const send_arg& other) :
	address (other.address),
	buffer (other.buffer)
      { }

      send_arg& operator= (const send_arg& other) {
	if (this != &other) {
	  address = other.address;
	  buffer = other.buffer;
	}
	return *this;
      }
    };

  private:
    int m_fd;
    int m_errno;

    std::list<std::pair<aid_t, send_arg*> > m_send_queue;
    std::set<aid_t> m_send_set; // Set of aids in send_queue.
    
    std::list<std::pair<aid_t, int> > m_complete_queue;
    std::set<aid_t> m_complete_set; // Set of aids in complete_queue.

  public:

    udp_broadcast_sender_automaton () :
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
      res = setsockopt (m_fd, SOL_SOCKET, SO_BROADCAST, &val, sizeof (val));
      if (res == -1) {
	m_errno = errno;
	goto the_end;
      }

      add_observable (&send);
      add_observable (&send_complete);
      
      // Success.
      m_errno = 0;
      schedule ();

    the_end:	
      if (m_errno != 0) {
	if (m_fd != -1) {
	  close (m_fd);
	  m_fd = -1;
	}
      }
    }

    ~udp_broadcast_sender_automaton () {
      if (m_fd != -1) {
	close (m_fd);
      }

      for (std::list<std::pair<aid_t, send_arg*> >::const_iterator pos = m_send_queue.begin ();
	   pos != m_send_queue.end ();
	   ++pos) {
	delete pos->second;
      }
    }

  private:

    void send_action (const send_arg& arg, aid_t aid) {
      // Ignore if aid has an outstanding send or complete.
      if (m_send_set.count (aid) == 0 && m_complete_set.count (aid) == 0) {
	if (m_fd != -1) {
	  // No error.  Add to the send queue and set.
	  m_send_queue.push_back (std::make_pair (aid, new send_arg (arg)));
	  m_send_set.insert (aid);
	}
	else {
	  // Error.  Add to the complete queue.
	  m_complete_queue.push_back (std::make_pair (aid, m_errno));
	  m_complete_set.insert (aid);
	}
      }

      schedule ();
    }

  public:

    V_AP_INPUT (udp_broadcast_sender_automaton, send, send_arg);

  private:

    bool do_sendto_precondition () const {
      return m_fd != -1 && !m_send_queue.empty ();
    }

    void do_sendto_action () {
      std::pair<aid_t, send_arg*> item = m_send_queue.front ();
      m_send_queue.pop_front ();
      m_send_set.erase (item.first);
      
      if (item.second->address.get_errno () == 0) {
	sendto (m_fd, item.second->buffer.data (), item.second->buffer.size (), 0, item.second->address.get_sockaddr (), item.second->address.get_socklen ());
	// We don't check a return value because we always return errno.
	m_complete_queue.push_back (std::make_pair (item.first, errno));
	m_complete_set.insert (item.first);
      }
      else {
	// Bad address.
	m_complete_queue.push_back (std::make_pair (item.first, EINVAL));
	m_complete_set.insert (item.first);
      }
      
      delete item.second;
      schedule ();
    }
    
    UP_INTERNAL (udp_broadcast_sender_automaton, do_sendto);

    bool send_complete_precondition (aid_t aid) const {
      return !m_complete_queue.empty () &&
	m_complete_queue.front ().first == aid &&
	bind_count (&udp_broadcast_sender_automaton::send_complete, aid) != 0;
    }

    int send_complete_action (aid_t aid) {
      std::pair<aid_t, int> item = m_complete_queue.front ();
      m_send_queue.pop_front ();
      assert (item.first == aid);
      schedule ();
      return item.second;
    }

  public:
    
    V_AP_OUTPUT (udp_broadcast_sender_automaton, send_complete, int);

  private:

    void schedule () const {
      if (do_sendto_precondition ()) {
	schedule_write_ready (&udp_broadcast_sender_automaton::do_sendto, m_fd);
      }
      if (!m_send_queue.empty ()) {
	ioa::schedule (&udp_broadcast_sender_automaton::send_complete, m_send_queue.front ().first);
      }
    }

    struct first_aid_equal {
      const aid_t m_aid;
      
      first_aid_equal (const aid_t aid) :
	m_aid (aid)
      { }

      bool operator() (const std::pair<aid_t, send_arg*>& o) const {
	return m_aid == o.first;
      }

      bool operator() (const std::pair<aid_t, int>& o) const {
	return m_aid == o.first;
      }
    };

    void purge (const aid_t aid) {
      if (m_send_set.count (aid) != 0) {
	std::list<std::pair<aid_t, send_arg*> >::iterator pos = std::find_if (m_send_queue.begin (),
									      m_send_queue.end (),
									      first_aid_equal (aid));
	assert (pos != m_send_queue.end ());
	delete pos->second;
	m_send_queue.erase (pos);
	m_send_set.erase (aid);
      }

      if (m_complete_set.count (aid) != 0) {
	m_complete_set.erase (aid);
	m_complete_queue.remove_if (first_aid_equal (aid));
      }

      schedule ();
    }

    void observe (observable* o) {
      // Purge the automata that are no longer bound.
      if (o == &send && send.recent_op == UNBOUND) {
	purge (send.recent_parameter);
      }
      else if (o == &send_complete && send_complete.recent_op == UNBOUND) {
	purge (send_complete.recent_parameter);
      }
    }

  };

}

#endif
