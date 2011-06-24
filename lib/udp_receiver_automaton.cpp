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
      close (m_fd);
      m_fd = -1;
      m_errno = errno;
      return;
    }

    // Set non-blocking.
    flags |= O_NONBLOCK;
    if (fcntl (m_fd, F_SETFL, flags) == -1) {
      close (m_fd);
      m_fd = -1;
      m_errno = errno;
      return;
    }

    const int val = 1;
#ifdef SO_REUSEADDR
    // Set reuse.
    if (setsockopt (m_fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof (val)) == -1) {
      close (m_fd);
      m_fd = -1;
      m_errno = errno;
      return;
    }
#endif

#ifdef SO_REUSEPORT
    // Set reuse.
    if (setsockopt (m_fd, SOL_SOCKET, SO_REUSEPORT, &val, sizeof (val)) == -1) {
      close (m_fd);
      m_fd = -1;
      m_errno = errno;
      return;
    }
#endif

    // Bind.
    if (::bind (m_fd, address.get_sockaddr (), address.get_socklen ()) == -1) {
      close (m_fd);
      m_fd = -1;
      m_errno = errno;
      return;
    }
  }

  udp_receiver_automaton::udp_receiver_automaton (const inet_address& address) :
    m_fd (-1),
    m_errno (0),
    m_buffer (0),
    m_buffer_size (0)
  {
    add_observable (&receive);
    prepare_socket (address);
    schedule ();
  }

  udp_receiver_automaton::udp_receiver_automaton (const inet_address& group_addr,
						  const inet_address& local_addr) :
    m_fd (-1),
    m_buffer (0),
    m_buffer_size (0)
  {
    add_observable (&receive);
    prepare_socket (local_addr);

    inet_mreq req (group_addr, local_addr);
    // Join the multicast group.
    if (setsockopt (m_fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, req.get_mreq (), req.get_length ()) == -1) {
      close (m_fd);
      m_fd = -1;
      m_errno = errno;
    }

    schedule ();
  }

  udp_receiver_automaton::~udp_receiver_automaton () {
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

  bool udp_receiver_automaton::do_recvfrom_precondition () const {
    return m_fd != -1;
  }

  void udp_receiver_automaton::do_recvfrom_effect () {
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
    
  bool udp_receiver_automaton::receive_precondition () const {
    return !m_receive.empty ();  // We don't care about being bound because udp is lossy.
  }

  udp_receiver_automaton::receive_val udp_receiver_automaton::receive_effect () {
    std::auto_ptr<receive_val> retval (m_receive.front ());
    m_receive.pop ();
    schedule ();
    return *retval;
  }

  void udp_receiver_automaton::schedule () const {
    if (do_recvfrom_precondition ()) {
      schedule_read_ready (&udp_receiver_automaton::do_recvfrom, m_fd);
    }
    if (receive_precondition ()) {
      ioa::schedule (&udp_receiver_automaton::receive);
    }
  }
  
  void udp_receiver_automaton::observe (observable* o) {
    if (m_fd == -1 && o == &receive && receive.recent_op == BOUND) {
      // An automaton has bound to receive but we will never receive because m_fd is bad.
      // Generate an error.
      m_receive.push (new receive_val (errno, inet_address (), buffer ()));
      schedule ();
    }
  }

};
