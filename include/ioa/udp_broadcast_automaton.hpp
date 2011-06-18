#ifndef __udp_broadcast_automaton_hpp__
#define __udp_broadcast_automaton_hpp__

#include <ioa/ioa.hpp>
#include <ioa/buffer.hpp>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include <string>

#include <iostream>

namespace ioa {

  class inet_address
  {
  public:
    virtual ~inet_address () { }
    virtual inet_address* clone () const = 0;
    virtual int get_errno () const = 0;
    virtual const sockaddr* get_sockaddr () const = 0;
    virtual socklen_t get_socklen () const = 0;
  };

  class ipv4_address :
    public inet_address
  {
  private:
    struct sockaddr_in m_address;
    int m_errno;

  public:
    ipv4_address (const std::string& address,
		  const unsigned short port) :
      m_errno (0)
    {
      memset (&m_address, 0, sizeof (m_address));
      m_address.sin_family = AF_INET;
      int res = inet_pton (AF_INET, address.c_str (), &m_address.sin_addr.s_addr);
      if (res < 0) {
      	m_errno = errno;
      }
      else if (res == 0) {
      	m_errno = EINVAL;
      }
      m_address.sin_port = htons (port);
    }    

    ipv4_address* clone () const {
      return new ipv4_address (*this);
    }

    int get_errno () const {
      return m_errno;
    }

    const sockaddr* get_sockaddr () const {
      return reinterpret_cast<const sockaddr*> (&m_address);
    }

    socklen_t get_socklen () const {
      return sizeof (m_address);
    }
  };

  class udp_broadcast_automaton :
    public ioa::automaton
  {
  public:
    struct send_arg {
      inet_address* address;
      ioa::buffer buffer;

      send_arg () :
	address (0)
      { }

      send_arg (const inet_address* a,
		const ioa::buffer& b) :
	address (0),
	buffer (b)
      {
	if (a != 0) {
	  address = a->clone ();
	}
      }

      send_arg (const send_arg& other) {
	if (other.address != 0) {
	  address = other.address->clone ();
	}
	else {
	  address = 0;
	}
	buffer = other.buffer;
      }

      send_arg& operator= (const send_arg& other) {
	if (this != &other) {
	  delete address;
	  if (other.address != 0) {
	    address = other.address->clone ();
	  }
	  else {
	    address = 0;
	  }
	  buffer = other.buffer;
	}
	return *this;
      }

      ~send_arg () {
	delete address;
      }
    };

  private:
    enum state_t {
      OPEN_WAIT,
      OPEN_RESULT_READY,
      OPENED
    };

    enum send_state_t {
      SEND_WAIT,
      DO_SEND_WAIT,
      SEND_RESULT_READY,
    };

    state_t m_state;
    send_state_t m_send_state;
    int m_send_fd;
    int m_open_errno;
    int m_send_errno;
    send_arg m_send_arg;

    void open_action (aid_t aid) {
      std::cout << "Got an open from " << aid << std::endl;

      const int val = 1;

      if (m_state == OPEN_WAIT) {
	int res;

	// Open the send socket.
	m_send_fd = socket (AF_INET, SOCK_DGRAM, 0);
	if (m_send_fd == -1) {
	  m_open_errno = errno;
	  goto the_end;
	}

	// Get the flags.
	int flags = fcntl (m_send_fd, F_GETFL, 0);
	if (flags < 0) {
	  m_open_errno = errno;
	  goto the_end;
	}

	// Set non-blocking.
	flags |= O_NONBLOCK;
	res = fcntl (m_send_fd, F_SETFL, flags);
	if (res < 0) {
	  m_open_errno = errno;
	  goto the_end;
	}

	// Set broadcasting.
	res = setsockopt (m_send_fd, SOL_SOCKET, SO_BROADCAST, &val, sizeof (val));
	if (res == -1) {
	  m_open_errno = errno;
	  goto the_end;
	}

	// Success.
	m_open_errno = 0;

    the_end:	
	if (m_open_errno != 0) {
	  if (m_send_fd != -1) {
	    close (m_send_fd);
	    m_send_fd = -1;
	  }
	}

	m_state = OPEN_RESULT_READY;
      }

      schedule ();
    }
  
    bool open_result_precondition () const {
      return m_state == OPEN_RESULT_READY && ioa::bind_count (&udp_broadcast_automaton::open_result) != 0;
    }

    int open_result_action () {
      if (m_open_errno == 0) {
	m_state = OPENED;
      }
      else {
	m_state = OPEN_WAIT;
      }
      schedule ();
      return m_open_errno;
    }

    void send_action (const send_arg& arg) {
      if (m_state == OPENED && m_send_state == SEND_WAIT) {
	m_send_arg = arg;
	ioa::schedule_write_ready (&udp_broadcast_automaton::do_send, m_send_fd);
	m_send_state = DO_SEND_WAIT;
      }
    }

    bool do_send_precondition () const {
      return true;
    }

    void do_send_action () {
      assert (m_send_state == DO_SEND_WAIT);

      m_send_errno = 0;

      if (m_send_arg.address != 0 && m_send_arg.buffer.size () != 0) {
	ssize_t nbytes = sendto (m_send_fd, m_send_arg.buffer.data (), m_send_arg.buffer.size (), 0, m_send_arg.address->get_sockaddr (), m_send_arg.address->get_socklen ());
	if (nbytes != static_cast<ssize_t> (m_send_arg.buffer.size ())) {
	  m_send_errno = errno;
	}
      }

      m_send_state = SEND_RESULT_READY;
      schedule ();
    }

    UP_INTERNAL (udp_broadcast_automaton, do_send);

    bool send_result_precondition () const {
      return m_send_state == SEND_RESULT_READY && ioa::bind_count (&udp_broadcast_automaton::send_result);
    }

    int send_result_action () {
      m_send_state = SEND_WAIT;
      schedule ();
      return m_send_errno;
    }

    void schedule () const {
      if (open_result_precondition ()) {
	ioa::schedule (&udp_broadcast_automaton::open_result);
      }
      if (send_result_precondition ()) {
	ioa::schedule (&udp_broadcast_automaton::send_result);
      }
    }

  public:

    udp_broadcast_automaton () :
      m_state (OPEN_WAIT),
      m_send_state (SEND_WAIT),
      m_send_fd (-1)
    { }

    ~udp_broadcast_automaton () {
      if (m_send_fd != -1) {
	close (m_send_fd);
      }
    }

    UV_AP_INPUT (udp_broadcast_automaton, open);
    V_UP_OUTPUT (udp_broadcast_automaton, open_result, int);
    V_UP_INPUT (udp_broadcast_automaton, send, send_arg);
    V_UP_OUTPUT (udp_broadcast_automaton, send_result, int);
  };

}

#endif
