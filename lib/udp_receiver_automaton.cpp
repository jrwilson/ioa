#include <ioa/udp_receiver_automaton.hpp>

namespace ioa {

  void udp_receiver_automaton::prepare_socket (const inet_address& address) {
    // Open a socket.
    m_fd = socket (AF_INET, SOCK_DGRAM, 0);
    if (m_fd == -1) {
      m_errno = errno;
      return;
    }

    // Get the flags.
    int flags = fcntl (m_fd, F_GETFL, 0);
    if (flags < 0) {
      // Set errno before close because close sets errno.
      m_errno = errno;
      close (m_fd);
      m_fd = -1;
      return;
    }

    // Set non-blocking.
    flags |= O_NONBLOCK;
    if (fcntl (m_fd, F_SETFL, flags) == -1) {
      m_errno = errno;
      close (m_fd);
      m_fd = -1;
      return;
    }

    const int val = 1;
#ifdef SO_REUSEADDR
    // Set reuse.
    if (setsockopt (m_fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof (val)) == -1) {
      m_errno = errno;
      close (m_fd);
      m_fd = -1;
      return;
    }
#endif

#ifdef SO_REUSEPORT
    // Set reuse.
    if (setsockopt (m_fd, SOL_SOCKET, SO_REUSEPORT, &val, sizeof (val)) == -1) {
      m_errno = errno;
      close (m_fd);
      m_fd = -1;
      return;
    }
#endif

    // Bind.
    if (::bind (m_fd, address.get_sockaddr (), address.get_socklen ()) == -1) {
      m_errno = errno;
      close (m_fd);
      m_fd = -1;
      return;
    }
  }

  udp_receiver_automaton::udp_receiver_automaton (const inet_address& address,
						  const size_t fan_out) :
    m_state (SCHEDULE_READ_READY),
    m_fan_out (fan_out),
    m_errno (0),
    m_buffer (0),
    m_buffer_size (0),
    m_receive (0)
  {
    add_observable (&receive);
    prepare_socket (address);
    schedule ();
  }

  udp_receiver_automaton::udp_receiver_automaton (const inet_address& group_addr,
						  const inet_address& local_addr,
						  const size_t fan_out) :
    m_state (SCHEDULE_READ_READY),
    m_fan_out (fan_out),
    m_errno (0),
    m_buffer (0),
    m_buffer_size (0),
    m_receive (0)
  {
    add_observable (&receive);
    prepare_socket (local_addr);

    inet_mreq req (group_addr, local_addr);
    // Join the multicast group.
    if (setsockopt (m_fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, req.get_mreq (), req.get_length ()) == -1) {
      m_errno = errno;
      close (m_fd);
      m_fd = -1;
    }

    schedule ();
  }

  udp_receiver_automaton::~udp_receiver_automaton () {
    if (m_fd != -1) {
      close (m_fd);
    }

    delete[] m_buffer;

    if (m_receive != 0) {
      delete m_receive;
    }
  }

  void udp_receiver_automaton::schedule () const {
    if (schedule_read_ready_precondition ()) {
      ioa::schedule (&udp_receiver_automaton::schedule_read_ready);
    }
    if (receive_precondition ()) {
      ioa::schedule (&udp_receiver_automaton::receive);
    }
  }
  
  void udp_receiver_automaton::observe (observable* o) {
    if (o == &receive && receive.recent_op == BOUND && m_fd == -1 && m_state == SCHEDULE_READ_READY) {
      // An automaton has bound to receive but we will never receive because m_fd is bad.
      // Generate an error.
      m_receive = new receive_val (m_errno, inet_address (), buffer ());
      m_state = RECEIVE_READY;
      schedule ();
    }
  }

  bool udp_receiver_automaton::schedule_read_ready_precondition () const {
    return m_state == SCHEDULE_READ_READY && m_fd != -1;
  }

  void udp_receiver_automaton::schedule_read_ready_effect () {
    ioa::schedule_read_ready (&udp_receiver_automaton::read_ready, m_fd);
    m_state = READ_READY_WAIT;
  }

  // Treat like an input.
  bool udp_receiver_automaton::read_ready_precondition () const {
    return m_state == READ_READY_WAIT;
  }

  void udp_receiver_automaton::read_ready_effect () {
    assert (m_receive == 0);

    int expect_bytes;
    int res = ioctl (m_fd, FIONREAD, &expect_bytes);
    if (res == -1) {
      m_errno = errno;
      close (m_fd);
      m_fd = -1;
      m_receive = new receive_val (m_errno, inet_address (), buffer ());
      m_state = RECEIVE_READY;
      return;
    }

    // Resize the buffer.
    if (static_cast<int> (m_buffer_size) < expect_bytes) {
      delete[] m_buffer;
      m_buffer = new unsigned char[expect_bytes];
      m_buffer_size = expect_bytes;
    }
	
    inet_address recv_address;
    
    ssize_t actual_bytes = recvfrom (m_fd, m_buffer, m_buffer_size, 0, recv_address.get_sockaddr_ptr (), recv_address.get_socklen_ptr ());
    if (actual_bytes != -1) {
      // Success.
      m_receive = new receive_val (0, recv_address, buffer (m_buffer, actual_bytes));
      m_state = RECEIVE_READY;
      return;
    }
    else {
      m_errno = errno;
      close (m_fd);
      m_fd = -1;
      m_receive = new receive_val (m_errno, inet_address (), buffer ());
      m_state = RECEIVE_READY;
      return;
    }
  }
    
  bool udp_receiver_automaton::receive_precondition () const {
    return m_state == RECEIVE_READY && bind_count (&udp_receiver_automaton::receive) == m_fan_out;
  }

  udp_receiver_automaton::receive_val udp_receiver_automaton::receive_effect () {
    assert (m_receive != 0);
    std::auto_ptr<receive_val> retval (m_receive);
    m_receive = 0;
    m_state = SCHEDULE_READ_READY;
    return *retval;
  }

};
