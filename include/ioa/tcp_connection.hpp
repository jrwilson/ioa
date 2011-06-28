#ifndef __tcp_connection_hpp__
#define __tcp_connection_hpp__

#include <ioa/buffer.hpp>
#include <sys/ioctl.h>

#include <iostream>

namespace ioa {
  
  class tcp_connection :
    public automaton
  {
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
    buffer m_send_buffer;
    ssize_t m_bytes_written;
    receive_state_t m_receive_state;
    buffer m_receive_buffer;

  public:
    tcp_connection (const int fd) :
      m_fd (fd),
      m_send_state (SEND_WAIT),
      m_receive_state (SCHEDULE_READ_READY)
    {
      std::cout << __func__ << std::endl;
      std::cout << "m_fd = " << m_fd << std::endl;
      schedule ();
    }

  private:
    void schedule () const {
      if (schedule_write_precondition ()) {
	ioa::schedule (&tcp_connection::schedule_write);
      }
      if (send_complete_precondition ()) {
	ioa::schedule (&tcp_connection::send_complete);
      }
      if (schedule_read_precondition ()) {
	ioa::schedule (&tcp_connection::schedule_read);
      }
      if (receive_precondition ()) {
	ioa::schedule (&tcp_connection::receive);
      }
    }

    void send_effect (const buffer& buf) {
      std::cout << __func__ << std::endl;
      if (m_send_state == SEND_WAIT) {
	m_send_buffer = buf;
	m_bytes_written = 0;
	m_send_state = SCHEDULE_WRITE_READY;
      }
    }

    bool schedule_write_precondition () const {
      return m_send_state == SCHEDULE_WRITE_READY;
    }

    void schedule_write_effect () {
      std::cout << __func__ << std::endl;
      ioa::schedule_write_ready (&tcp_connection::write_ready, m_fd);
      m_send_state = WRITE_READY_WAIT;
    }

    UP_INTERNAL (tcp_connection, schedule_write);

    // Treat like an input.
    bool write_ready_precondition () const {
      return true;
    }

    void write_ready_effect () {
      assert (m_send_state == WRITE_READY_WAIT);

      ssize_t bytes_written = write (m_fd, m_send_buffer.data () + m_bytes_written, m_send_buffer.size () - m_bytes_written);
      std::cout << "bytes_written " << bytes_written << std::endl;

      if (bytes_written == -1) {
	// TODO
	assert (false);
      }

      m_bytes_written += bytes_written;

      if (m_bytes_written == static_cast<ssize_t> (m_send_buffer.size ())) {
	// Done.
	m_send_state = SEND_COMPLETE_READY;
      }
      else {
	// Not done.
	m_send_state = WRITE_READY_WAIT;
	ioa::schedule_write_ready (&tcp_connection::write_ready, m_fd);
      }
    }

    UP_INTERNAL (tcp_connection, write_ready);

  public:
    V_UP_INPUT (tcp_connection, send, buffer);

  private:
    bool send_complete_precondition () const {
      return m_send_state == SEND_COMPLETE_READY && bind_count (&tcp_connection::send_complete) != 0;
    }

    void send_complete_effect () {
      std::cout << __func__ << std::endl;
      m_send_state = SEND_WAIT;
    }

  public:
    UV_UP_OUTPUT (tcp_connection, send_complete);

  private:
    bool schedule_read_precondition () const {
      return m_receive_state == SCHEDULE_READ_READY;
    }

    void schedule_read_effect () {
      std::cout << __func__ << std::endl;
      ioa::schedule_read_ready (&tcp_connection::read_ready, m_fd);
      m_receive_state = READ_READY_WAIT;
    }

    UP_INTERNAL (tcp_connection, schedule_read);

    // Treat like an input.
    bool read_ready_precondition () const {
      return true;
    }

    void read_ready_effect () {
      assert (m_receive_state == READ_READY_WAIT);

      int num_bytes;
      if (ioctl (m_fd, FIONREAD, &num_bytes) == -1) {
	// TODO
	assert (false);
      }

      std::cout << "num_bytes = " << num_bytes << std::endl;
      buffer buf (num_bytes);
      ssize_t bytes_read = read (m_fd, buf.data (), num_bytes);
      std::cout << "bytes_read = " << bytes_read << std::endl;
      if (bytes_read == -1) {
	// TODO
	assert (false);
      }

      m_receive_buffer = buf;
      m_receive_state = RECEIVE_READY;      
    }

    UP_INTERNAL (tcp_connection, read_ready);

    bool receive_precondition () const {
      return m_receive_state == RECEIVE_READY && bind_count (&tcp_connection::receive) != 0;
    }

    buffer receive_effect () {
      std::cout << __func__ << std::endl;
      m_receive_state = SCHEDULE_READ_READY;
      return m_receive_buffer;
    }
    
  public:
    V_UP_OUTPUT (tcp_connection, receive, buffer);
  };

}

#endif
