#ifndef __udp_broadcast_receiver_automaton_hpp__
#define __udp_broadcast_receiver_automaton_hpp__

#include <ioa/ioa.hpp>
#include <ioa/buffer.hpp>
#include <ioa/inet_address.hpp>

#include <fcntl.h>
#include <sys/ioctl.h>

#include <queue>
#include <algorithm>

namespace ioa {

  class udp_broadcast_receiver_automaton :
    public automaton,
    private observer
  {
  public:
    struct receive_val {
      int err_no;
      inet_address address;
      ioa::buffer buffer;

      receive_val (const int e,
		   const inet_address& a,
		   const ioa::buffer& b) :
	err_no (e),
	address (a),
	buffer (b)
      { }

      receive_val (const receive_val& other) :
	err_no (other.err_no),
	address (other.address),
	buffer (other.buffer)
      { }

      receive_val& operator= (const receive_val& other) {
	if (this != &other) {
	  err_no = other.err_no;
	  address = other.address;
	  buffer = other.buffer;
	}
	return *this;
      }
    };

  private:
    int m_fd;
    int m_errno;
    unsigned char* m_buffer;
    size_t m_buffer_size;
    std::queue<receive_val*> m_receive;

  public:

    udp_broadcast_receiver_automaton (const inet_address& address) :
      m_fd (-1),
      m_buffer (0),
      m_buffer_size (0)
    {
      int flags;
      const int val = 1;

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
      if (fcntl (m_fd, F_SETFL, flags) == -1) {
	m_errno = errno;
	goto the_end;
      }

#ifdef SO_REUSEADDR
      // Set reuse.
      if (setsockopt (m_fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof (val)) == -1) {
	m_errno = errno;
	goto the_end;
      }
#endif

#ifdef SO_REUSEPORT
      // Set reuse.
      if (setsockopt (m_fd, SOL_SOCKET, SO_REUSEPORT, &val, sizeof (val)) == -1) {
	m_errno = errno;
	goto the_end;
      }
#endif

      // Bind.
      if (::bind (m_fd, address.get_sockaddr (), address.get_socklen ()) == -1) {
	m_errno = errno;
	goto the_end;
      }
      
      add_observable (&receive);

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

    ~udp_broadcast_receiver_automaton () {
      if (m_fd != -1) {
	close (m_fd);
      }

      delete[] m_buffer;

      while (!m_receive.empty ()) {
	receive_val* r = m_receive.front ();
	m_receive.pop ();
	delete r;
      }
    }

  private:

    bool do_recvfrom_precondition () const {
      return m_fd != -1;
    }

    void do_recvfrom_effect () {

      int expect_bytes;
      int res = ioctl (m_fd, FIONREAD, &expect_bytes);
      if (res == -1) {
	close (m_fd);
	m_fd = -1;
	m_receive.push (new receive_val (errno, inet_address (), buffer ()));
      }

      if (m_fd != -1) {
	// Resize the buffer.
	if (static_cast<int> (m_buffer_size) < expect_bytes) {
	  delete[] m_buffer;
	  m_buffer = new unsigned char[expect_bytes];
	  m_buffer_size = expect_bytes;
	}
	
	inet_address recv_address;
	
	ssize_t actual_bytes = recvfrom (m_fd, m_buffer, m_buffer_size, 0, recv_address.get_sockaddr_ptr (), recv_address.get_socklen_ptr ());
	if (actual_bytes != -1) {
	  m_receive.push (new receive_val (errno, recv_address, buffer (m_buffer, actual_bytes)));
	}
	else {
	  m_receive.push (new receive_val (errno, inet_address (), buffer ()));
	  close (m_fd);
	  m_fd = -1;
	}
      }

      schedule ();
    }
    
    UP_INTERNAL (udp_broadcast_receiver_automaton, do_recvfrom);

  private:

    bool receive_precondition () const {
      return !m_receive.empty ();  // We don't care about being bound because udp is lossy.
    }

    receive_val receive_effect () {
      std::auto_ptr<receive_val> retval (m_receive.front ());
      m_receive.pop ();
      schedule ();
      return *retval;
    }

  public:
    
    V_UP_OUTPUT (udp_broadcast_receiver_automaton, receive, receive_val);

  private:

    void schedule () const {
      if (do_recvfrom_precondition ()) {
	schedule_read_ready (&udp_broadcast_receiver_automaton::do_recvfrom, m_fd);
      }
      if (receive_precondition ()) {
	ioa::schedule (&udp_broadcast_receiver_automaton::receive);
      }
    }

    void observe (observable* o) {
      if (m_fd == -1 && o == &receive && receive.recent_op == BOUND) {
	// An automaton has bound to receive but we will never receive because m_fd is bad.
	// Generate an error.
	m_receive.push (new receive_val (errno, inet_address (), buffer ()));
	schedule ();
      }
    }

  };

}

#endif
