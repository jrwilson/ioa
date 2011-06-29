#include <ioa/tcp_connection_automaton.hpp>

namespace ioa {

  tcp_connection_automaton::tcp_connection_automaton (const int fd) :
    m_fd (fd),
    m_send_state (SEND_WAIT),
    m_send_errno (0),
    m_receive_state (SCHEDULE_READ_READY),
    m_receive_errno (0)
  {
    assert (m_fd != -1);
    add_observable (&send_complete);
    add_observable (&receive);
    schedule ();
  }

  tcp_connection_automaton::~tcp_connection_automaton () {
    if (m_fd != -1) {
      close (m_fd);
    }
  }

  void tcp_connection_automaton::schedule () const {
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

  void tcp_connection_automaton::observe (observable* o) {
    if ((o == &send_complete && send_complete.recent_op == UNBOUND) ||
	(o == &receive && receive.recent_op == UNBOUND)) {
      // User is unbinding.  Time to die.
      self_destruct ();
    }
    else if (o == &receive && receive.recent_op == BOUND && m_fd == -1 && m_receive_state == SCHEDULE_READ_READY) {
      // An automaton has bound to receive but we will never receive because m_fd is bad.
      // Generate an error.
      m_receive_buffer = buffer ();
      m_receive_state = RECEIVE_READY;
      schedule ();
    }
  }

  void tcp_connection_automaton::send_effect (const buffer& buf) {
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

  bool tcp_connection_automaton::schedule_write_precondition () const {
    return m_send_state == SCHEDULE_WRITE_READY;
  }

  void tcp_connection_automaton::schedule_write_effect () {
    if (m_fd != -1) {
      ioa::schedule_write_ready (&tcp_connection_automaton::write_ready, m_fd);
      m_send_state = WRITE_READY_WAIT;
    }
    else {
      m_send_state = SEND_COMPLETE_READY;
    }
  }

  bool tcp_connection_automaton::write_ready_precondition () const {
    return m_send_state == WRITE_READY_WAIT;
  }

  void tcp_connection_automaton::write_ready_effect () {
    if (m_fd == -1) {
      m_send_state = SEND_COMPLETE_READY;
      return;
    }

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

  bool tcp_connection_automaton::send_complete_precondition () const {
    return m_send_state == SEND_COMPLETE_READY && bind_count (&tcp_connection_automaton::send_complete) != 0;
  }

  int tcp_connection_automaton::send_complete_effect () {
    m_send_state = SEND_WAIT;
    return m_send_errno;
  }

  bool tcp_connection_automaton::schedule_read_precondition () const {
    return m_receive_state == SCHEDULE_READ_READY && m_fd != -1;
  }

  void tcp_connection_automaton::schedule_read_effect () {
    if (m_fd != -1) {
      ioa::schedule_read_ready (&tcp_connection_automaton::read_ready, m_fd);
      m_receive_state = READ_READY_WAIT;
    }
    else {
      m_receive_state = RECEIVE_READY;
    }
  }

  bool tcp_connection_automaton::read_ready_precondition () const {
    return m_receive_state == READ_READY_WAIT;
  }

  void tcp_connection_automaton::read_ready_effect () {
    if (m_fd == -1) {
      m_receive_state = RECEIVE_READY;
      return;
    }

    // Determine the number of bytes we can read without blocking.
    int num_bytes;
    if (ioctl (m_fd, FIONREAD, &num_bytes) == -1) {
      m_send_errno = errno;
      m_receive_errno = errno;
      m_receive_buffer = buffer ();
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
      m_receive_buffer = buffer ();
      close (m_fd);
      m_fd = -1;
      m_receive_state = RECEIVE_READY;      
      return;
    }
    else if (bytes_read == 0) {
      m_send_errno = ECONNRESET;
      m_receive_errno = ECONNRESET;
      m_receive_buffer = buffer ();
      close (m_fd);
      m_fd = -1;
      m_receive_state = RECEIVE_READY;
      return;
    }

    m_receive_buffer = buf;
    m_receive_state = RECEIVE_READY;      
  }

  bool tcp_connection_automaton::receive_precondition () const {
    return m_receive_state == RECEIVE_READY && bind_count (&tcp_connection_automaton::receive) != 0;
  }

  tcp_connection_automaton::receive_val tcp_connection_automaton::receive_effect () {
    m_receive_state = SCHEDULE_READ_READY;
    return receive_val (m_receive_errno, m_receive_buffer);
  }

}
