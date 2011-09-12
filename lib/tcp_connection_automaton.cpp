#include <ioa/tcp_connection_automaton.hpp>

#include <sys/ioctl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>

namespace ioa {

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
    if (connected_precondition ()) {
      ioa::schedule (&tcp_connection_automaton::connected);
    }
    if (error_precondition ()) {
      ioa::schedule (&tcp_connection_automaton::error);
    }
  }

  tcp_connection_automaton::tcp_connection_automaton () :
    m_fd (-1),
    m_errno (0),
    m_connected_reported (false),
    m_error_reported (false),
    m_send_state (SEND_WAIT),
    m_receive_state (SCHEDULE_READ_READY),
    m_buffer (0),
    m_buffer_size (0)
  { }

  tcp_connection_automaton::~tcp_connection_automaton () {
    if (m_fd != -1) {
      ioa::close (m_fd);
    }
    delete[] m_buffer;
  }

  void tcp_connection_automaton::send_effect (const std::string& buf) {
    if (m_errno == 0 && m_send_state == SEND_WAIT) {
      m_send_buffer = buf;
      m_bytes_written = 0;
      m_send_state = SCHEDULE_WRITE_READY;
    }
  }

  void tcp_connection_automaton::send_schedule () const {
    schedule ();
  }

  bool tcp_connection_automaton::schedule_write_precondition () const {
    return m_fd != -1 && m_errno == 0 && m_send_state == SCHEDULE_WRITE_READY;
  }

  void tcp_connection_automaton::schedule_write_effect () {
    m_send_state = WRITE_READY_WAIT;
    ioa::schedule_write_ready (&tcp_connection_automaton::write_ready, m_fd);
  }

  void tcp_connection_automaton::schedule_write_schedule () const {
    schedule ();
  }
  
  bool tcp_connection_automaton::write_ready_precondition () const {
    return m_fd != -1 && m_errno == 0 && m_send_state == WRITE_READY_WAIT;
  }

  void tcp_connection_automaton::write_ready_effect () {
    // Write to the socket.
#ifdef MSG_NOSIGNAL
    ssize_t bytes_written = ::send (m_fd, static_cast<const char*> (m_send_buffer->data ()) + m_bytes_written, m_send_buffer->size () - m_bytes_written, MSG_NOSIGNAL);
#else
    ssize_t bytes_written = write (m_fd, static_cast<const char*> (m_send_buffer.data ()) + m_bytes_written, m_send_buffer.size () - m_bytes_written);
#endif

    if (bytes_written == -1) {
      m_errno = errno;
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

  void tcp_connection_automaton::write_ready_schedule () const {
    schedule ();
  }
  
  bool tcp_connection_automaton::send_complete_precondition () const {
    return m_send_state == SEND_COMPLETE_READY && binding_count (&tcp_connection_automaton::send_complete) != 0;
  }

  void tcp_connection_automaton::send_complete_effect () {
    m_send_state = SEND_WAIT;
  }

  void tcp_connection_automaton::send_complete_schedule () const {
    schedule ();
  }
  
  bool tcp_connection_automaton::schedule_read_precondition () const {
    return m_fd != -1 && m_errno == 0 && m_receive_state == SCHEDULE_READ_READY;
  }

  void tcp_connection_automaton::schedule_read_effect () {
    m_receive_state = READ_READY_WAIT;
    ioa::schedule_read_ready (&tcp_connection_automaton::read_ready, m_fd);
  }

  void tcp_connection_automaton::schedule_read_schedule () const {
    schedule ();
  }
  
  bool tcp_connection_automaton::read_ready_precondition () const {
    return m_fd != -1 && m_errno == 0 && m_receive_state == READ_READY_WAIT;
  }

  void tcp_connection_automaton::read_ready_effect () {
    // Determine the number of bytes we can read without blocking.
    int num_bytes;
    if (ioctl (m_fd, FIONREAD, &num_bytes) == -1) {
      m_errno = errno;
      return;
    }

    // Resize the buffer to hold num_bytes.
    if (m_buffer_size < num_bytes) {
      delete[] m_buffer;
      m_buffer = new char[num_bytes];
      m_buffer_size = num_bytes;
    }
    ssize_t bytes_read = read (m_fd, m_buffer, num_bytes);
    if (bytes_read == -1) {
      m_errno = errno;
      return;
    }
    else if (bytes_read == 0) {
      m_errno = ECONNRESET;
      return;
    }

    m_receive_buffer = std::string (m_buffer, bytes_read);
    m_receive_state = RECEIVE_READY;      
  }

  void tcp_connection_automaton::read_ready_schedule () const {
    schedule ();
  }
  
  bool tcp_connection_automaton::receive_precondition () const {
    return m_receive_state == RECEIVE_READY && binding_count (&tcp_connection_automaton::receive) != 0;
  }

  std::string tcp_connection_automaton::receive_effect () {
    m_receive_state = SCHEDULE_READ_READY;
    return m_receive_buffer;
  }

  void tcp_connection_automaton::receive_schedule () const {
    schedule ();
  }

  void tcp_connection_automaton::init_effect (const int& fd) {
    if (m_fd == -1) {
      m_fd = fd;

      // No SIGPIPE.
#ifdef SO_NOSIGPIPE
      const int set = 1;
      if (setsockopt (m_fd, SOL_SOCKET, SO_NOSIGPIPE, &set, sizeof (int)) == -1) {
	m_errno = errno;
      }
#endif
    }
  }

  void tcp_connection_automaton::init_schedule () const {
    schedule ();
  }

  bool tcp_connection_automaton::connected_precondition () const {
    return m_fd != -1 && !m_connected_reported && binding_count (&tcp_connection_automaton::connected) != 0;
  }

  void tcp_connection_automaton::connected_effect () {
    m_connected_reported = true;
  }
  
  void tcp_connection_automaton::connected_schedule () const {
    schedule ();
  }

  bool tcp_connection_automaton::error_precondition () const {
    return m_errno != 0 && !m_error_reported && binding_count (&tcp_connection_automaton::error) != 0;
  }

  int tcp_connection_automaton::error_effect () {
    m_error_reported = true;
    return m_errno;
  }
  
  void tcp_connection_automaton::error_schedule () const {
    schedule ();
  }

  connection_init_automaton::connection_init_automaton (const automaton_handle<tcp_connection_automaton>& conn,
							int fd) :
    m_state (START),
    m_self (get_aid ()),
    m_conn (conn),
    m_fd (fd) {
    add_observable (make_binding_manager (this,
					  &m_self, &connection_init_automaton::init,
					  &m_conn, &tcp_connection_automaton::init));
  }

  void connection_init_automaton::observe (observable* o) {
    binding_manager_interface* bmi = static_cast<binding_manager_interface*> (o);
    switch (bmi->get_state ()) {
    case binding_manager_interface::START:
      // Do nothing.
      break;
    case binding_manager_interface::OUTPUT_AUTOMATON_DNE:
    case binding_manager_interface::INPUT_AUTOMATON_DNE:
    case binding_manager_interface::INPUT_ACTION_UNAVAILABLE:
    case binding_manager_interface::OUTPUT_ACTION_UNAVAILABLE:
      // Fail.
      m_state = FAIL;
      break;
    case binding_manager_interface::BOUND:
      break;
    case binding_manager_interface::UNBOUND:
      break;
    }
    
    schedule ();
  }

  void connection_init_automaton::schedule () const {
    if (init_precondition ()) {
      ioa::schedule (&connection_init_automaton::init);
    }
    if (done_precondition ()) {
      ioa::schedule (&connection_init_automaton::done);
    }
  }

  bool connection_init_automaton::init_precondition () const {
    return m_state == START && binding_count (&connection_init_automaton::init) != 0;
  }

  int connection_init_automaton::init_effect () {
    m_state = SUCCESS;
    return m_fd;
  }

  void connection_init_automaton::init_schedule () const {
    schedule ();
  }

  bool connection_init_automaton::done_precondition () const {
    return (m_state == SUCCESS || m_state == FAIL) && binding_count (&connection_init_automaton::done) != 0;
  }

  int connection_init_automaton::done_effect () {
    int retval;
    switch (m_state) {
    case SUCCESS:
      retval = -1;
      break;
    case FAIL:
      retval = m_fd;
      break;
    default:
      assert (false);
      break;
    }
    m_state = STOP;

    return retval;
  }

  void connection_init_automaton::done_schedule () const {
    schedule ();
  }

}
