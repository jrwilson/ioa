#ifndef __tcp_connection_automaton_hpp__
#define __tcp_connection_automaton_hpp__

#include <ioa/buffer.hpp>
#include <sys/ioctl.h>

namespace ioa {
  
  class tcp_connection_automaton :
    public automaton,
    private observer
  {
  public:
    struct receive_val
    {
      int err;
      buffer buf;
      
      receive_val () :
	err (0)
      { }

      receive_val (const int e,
		   const buffer& b) :
	err (e),
	buf (b)
      { }
    };

  private:
    enum send_state_t {
      SEND_WAIT,
      SCHEDULE_WRITE_READY,
      WRITE_READY_WAIT,
      SEND_COMPLETE_READY,
    };

    enum receive_state_t {
      SCHEDULE_READ_READY,
      READ_READY_WAIT,
      RECEIVE_READY,
    };
      
    int m_fd;
    send_state_t m_send_state;
    int m_send_errno;
    buffer m_send_buffer;
    ssize_t m_bytes_written;
    receive_state_t m_receive_state;
    int m_receive_errno;
    receive_val m_receive_val;

  public:
    tcp_connection_automaton (const int fd) :
      m_fd (fd),
      m_send_state (SEND_WAIT),
      m_send_errno (0),
      m_receive_state (SCHEDULE_READ_READY),
      m_receive_errno (0)
    {
      assert (m_fd != -1);
      add_observable (&receive);
      schedule ();
    }

  private:
    void schedule () const {
      if (schedule_write_precondition ()) {
	ioa::schedule (&tcp_connection_automaton::schedule_write);
      }
      if (send_complete_precondition ()) {
	ioa::schedule (&tcp_connection_automaton::send_complete);
      }
      if (schedule_read_precondition ()) {
	ioa::schedule (&tcp_connection_automaton::schedule_read);
      }
      if (receive_precondition ()) {
	ioa::schedule (&tcp_connection_automaton::receive);
      }
    }

    void observe (observable* o) {
      if (o == &receive && receive.recent_op == BOUND && m_fd == -1 && m_receive_state == SCHEDULE_READ_READY) {
	// An automaton has bound to receive but we will never receive because m_fd is bad.
	// Generate an error.
	m_receive_val = receive_val (m_receive_errno, buffer ());
	m_receive_state = RECEIVE_READY;
	schedule ();
      }
    }

    void send_effect (const buffer& buf) {
      if (m_send_state == SEND_WAIT) {
	if (m_fd != -1) {
	  m_send_buffer = buf;
	  m_bytes_written = 0;
	  m_send_state = SCHEDULE_WRITE_READY;
	}
	else {
	  m_send_state = SEND_COMPLETE_READY;
	}
      }
    }

    bool schedule_write_precondition () const {
      return m_send_state == SCHEDULE_WRITE_READY;
    }

    void schedule_write_effect () {
      ioa::schedule_write_ready (&tcp_connection_automaton::write_ready, m_fd);
      m_send_state = WRITE_READY_WAIT;
    }

    UP_INTERNAL (tcp_connection_automaton, schedule_write);

    bool write_ready_precondition () const {
      return m_send_state == WRITE_READY_WAIT;
    }

    void write_ready_effect () {
      // Write to the socket.
      ssize_t bytes_written = write (m_fd, m_send_buffer.data () + m_bytes_written, m_send_buffer.size () - m_bytes_written);

      if (bytes_written == -1) {
	m_send_errno = errno;
	m_receive_errno = errno;
	close (m_fd);
	m_fd = -1;
	m_send_state = SEND_COMPLETE_READY;
      }
      else {
	// We have made progress.
	m_bytes_written += bytes_written;
	
	if (m_bytes_written == static_cast<ssize_t> (m_send_buffer.size ())) {
	  // We are done with this buffer.
	  m_send_state = SEND_COMPLETE_READY;
	}
	else {
	  // Not done.
	  m_send_state = SCHEDULE_WRITE_READY;
	}
      }
    }

    UP_INTERNAL (tcp_connection_automaton, write_ready);

  public:
    V_UP_INPUT (tcp_connection_automaton, send, buffer);

  private:
    bool send_complete_precondition () const {
      return m_send_state == SEND_COMPLETE_READY && bind_count (&tcp_connection_automaton::send_complete) != 0;
    }

    int send_complete_effect () {
      m_send_state = SEND_WAIT;
      return m_send_errno;
    }

  public:
    V_UP_OUTPUT (tcp_connection_automaton, send_complete, int);

  private:
    bool schedule_read_precondition () const {
      return m_receive_state == SCHEDULE_READ_READY && m_fd != -1;
    }

    void schedule_read_effect () {
      ioa::schedule_read_ready (&tcp_connection_automaton::read_ready, m_fd);
      m_receive_state = READ_READY_WAIT;
    }

    UP_INTERNAL (tcp_connection_automaton, schedule_read);

    bool read_ready_precondition () const {
      return m_receive_state == READ_READY_WAIT;
    }

    void read_ready_effect () {
      // Determine the number of bytes we can read without blocking.
      int num_bytes;
      if (ioctl (m_fd, FIONREAD, &num_bytes) == -1) {
	m_send_errno = errno;
	m_receive_errno = errno;
	m_receive_val = receive_val (m_receive_errno, buffer ());
	close (m_fd);
	m_fd = -1;
	m_receive_state = RECEIVE_READY;
	return;
      }

      // Create a buffer capable of holding num_bytes and fill it.
      buffer buf (num_bytes);
      ssize_t bytes_read = read (m_fd, buf.data (), num_bytes);
      if (bytes_read == -1) {
	m_send_errno = errno;
	m_receive_errno = errno;
	m_receive_val = receive_val (m_receive_errno, buffer ());
	close (m_fd);
	m_fd = -1;
	m_receive_state = RECEIVE_READY;      
	return;
      }

      m_receive_val = receive_val (0, buf);
      m_receive_state = RECEIVE_READY;      
    }

    UP_INTERNAL (tcp_connection_automaton, read_ready);

    bool receive_precondition () const {
      return m_receive_state == RECEIVE_READY && bind_count (&tcp_connection_automaton::receive) != 0;
    }

    receive_val receive_effect () {
      m_receive_state = SCHEDULE_READ_READY;
      return m_receive_val;
    }
    
  public:
    V_UP_OUTPUT (tcp_connection_automaton, receive, receive_val);
  };

}

#endif
