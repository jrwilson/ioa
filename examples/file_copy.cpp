#include <ioa.hpp>
#include <iostream>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

class buffer {
private:
  size_t m_size;
  unsigned char* m_data;
public:
  buffer () :
    m_size (0),
    m_data (0)
  { }

  buffer (const size_t size,
	  const void* data) :
    m_size (size) {
    m_data = new unsigned char[m_size];
    memcpy (m_data, data, m_size);
  }

  buffer (const buffer& other) :
    m_size (other.m_size) {
    m_data = new unsigned char[m_size];
    memcpy (m_data, other.m_data, m_size);
  }

  buffer& operator= (const buffer& other) {
    if (this != &other) {
      delete[] m_data;
      m_size = other.m_size;
      m_data = new unsigned char[m_size];
      memcpy (m_data, other.m_data, m_size);
    }
    return *this;
  }

  ~buffer () {
    delete[] m_data;
  }

  size_t size () const {
    return m_size;
  }
  
  unsigned char* data () const {
    return m_data;
  }

};

class udp_broadcast_automaton :
  public ioa::automaton_interface
{
private:
  enum state_t {
    WAITING_FOR_OPEN,
    SEND_OPEN,
    OPENED
  };

  enum send_state_t {
    WAITING_FOR_SEND,
    WAITING_FOR_DO_SEND,
    WAITING_FOR_SEND_RESULT,
  };

  state_t m_state;
  send_state_t m_send_state;
  int m_fd;
  int m_open_errno;
  struct sockaddr_in m_address;
  buffer m_send_buffer;
  int m_send_errno;

public:
  struct open_arg {
    std::string address;
    short int port;

    open_arg (const std::string a,
	      const short int p) :
      address (a),
      port (p)
    { }
  };

private:

  void open_action (const open_arg& arg) {
    if (m_state == WAITING_FOR_OPEN) {
      // Initialize the address.
      memset (&m_address, 0, sizeof (struct sockaddr_in));
      m_address.sin_family = AF_INET;
      m_address.sin_port = htons (arg.port);
      int res = inet_pton (AF_INET, arg.address.c_str (), &m_address.sin_addr.s_addr);
      if (res < 0 ) {
	m_open_errno = errno;
      }
      else if (res == 0) {
	m_open_errno = EINVAL;
      }

      if (res > 0) {
	// Open a socket.
	m_fd = socket (AF_INET, SOCK_DGRAM, 0);
	m_open_errno = (m_fd != -1) ? 0 : errno;
	
	if (m_fd != -1) {
	  // Set broadcasting.
	  const int val = 1;
	  res = setsockopt (m_fd, SOL_SOCKET, SO_BROADCAST, &val, sizeof (val));
	  if (res == -1) {
	    m_open_errno = errno;
	    // Close the socket.
	    close (m_fd);
	    m_fd = -1;
	  }
	  else {
	    m_open_errno = 0;
	  }
	}
      }
	
      m_state = SEND_OPEN;
    }
    schedule ();
  }
  
  bool open_result_precondition () const {
    return m_state == SEND_OPEN && ioa::scheduler::bind_count (&udp_broadcast_automaton::open_result) != 0;
  }

  int open_result_action () {
    if (m_open_errno == 0) {
      m_state = OPENED;
    }
    else {
      m_state = WAITING_FOR_OPEN;
    }
    schedule ();
    return m_open_errno;
  }

  void send_action (const buffer& buf) {
    if (m_state == OPENED && m_send_state == WAITING_FOR_SEND && buf.size () != 0) {
      m_send_buffer = buf;
      ioa::scheduler::schedule_write_ready (&udp_broadcast_automaton::do_send, m_fd);
      m_send_state = WAITING_FOR_DO_SEND;
    }
  }

  bool do_send_precondition () const { return true; }
  void do_send_action () {
    assert (m_send_state == WAITING_FOR_DO_SEND);

    ssize_t nbytes = sendto (m_fd, m_send_buffer.data (), m_send_buffer.size (), 0, reinterpret_cast<const sockaddr*> (&m_address), sizeof (m_address));
    m_send_errno = (nbytes == -1) ? errno : 0;
    m_send_state = WAITING_FOR_SEND_RESULT;
    schedule ();
  }
  UP_INTERNAL (udp_broadcast_automaton, do_send);

  bool send_result_precondition () const {
    return m_send_state == WAITING_FOR_SEND_RESULT && ioa::scheduler::bind_count (&udp_broadcast_automaton::send_result);
  }

  int send_result_action () {
    m_send_state = WAITING_FOR_SEND;
    schedule ();
    return m_send_errno;
  }

  void schedule () const {
    if (open_result_precondition ()) {
      ioa::scheduler::schedule (&udp_broadcast_automaton::open_result);
    }
    if (send_result_precondition ()) {
      ioa::scheduler::schedule (&udp_broadcast_automaton::send_result);
    }
  }

public:

  udp_broadcast_automaton () :
    m_state (WAITING_FOR_OPEN),
    m_send_state (WAITING_FOR_SEND),
    m_fd (-1)
  { }

  V_UP_INPUT (udp_broadcast_automaton, open, open_arg);
  V_UP_OUTPUT (udp_broadcast_automaton, open_result, int);
  V_UP_INPUT (udp_broadcast_automaton, send, buffer);
  V_UP_OUTPUT (udp_broadcast_automaton, send_result, int);
};

class file_copy :
  public ioa::automaton_interface
{
private:
  enum state_t {
    START,
    OPEN_SENT,
    OPEN_RECEIVED,
    SEND_SENT,
    SEND_RECEIVED
  };
  
  state_t m_state;
  std::auto_ptr<ioa::self_helper<file_copy> > m_self;

  bool open_precondition () const {
    return m_state == START && ioa::scheduler::bind_count (&file_copy::open) != 0;
  }

  udp_broadcast_automaton::open_arg open_action () {
    m_state = OPEN_SENT;
    schedule ();
    return udp_broadcast_automaton::open_arg ("255.255.255.255", 64470);
  }

  V_UP_OUTPUT (file_copy, open, udp_broadcast_automaton::open_arg);

  void open_result_action (const int& result) {
    if (result != 0) {
      char buf[256];
      strerror_r (result, buf, 256);
      std::cerr << "Couldn't open udp_broadcast_automaton: " << buf << std::endl;
    }
    else {
      m_state = OPEN_RECEIVED;
    }
    schedule ();
  }

  V_UP_INPUT (file_copy, open_result, int);

  bool send_precondition () const {
    return m_state == OPEN_RECEIVED && ioa::scheduler::bind_count (&file_copy::send) != 0;
  }

  buffer send_action () {
    m_state = SEND_SENT;
    schedule ();
    std::string s ("Hello, World!");
    return buffer (s.size (), s.c_str ());
  }
  V_UP_OUTPUT (file_copy, send, buffer);

  void send_result_action (const int& result) {
    if (result != 0) {
      char buf[256];
      strerror_r (result, buf, 256);
      std::cerr << "Couldn't send udp_broadcast_automaton: " << buf << std::endl;
    }
    else {
      m_state = SEND_RECEIVED;
    }
    schedule ();
  }

  V_UP_INPUT (file_copy, send_result, int);

  void schedule () const {
    if (open_precondition ()) {
      ioa::scheduler::schedule (&file_copy::open);
    }
    if (send_precondition ()) {
      ioa::scheduler::schedule (&file_copy::send);
    }
  }

public:
  file_copy () :
    m_state (START),
    m_self (new ioa::self_helper<file_copy> ())
  {
    ioa::automaton_helper<udp_broadcast_automaton>* sender = new ioa::automaton_helper<udp_broadcast_automaton> (this, ioa::make_generator<udp_broadcast_automaton> ());

    ioa::make_bind_helper (this, m_self.get (), &file_copy::open, sender, &udp_broadcast_automaton::open);
    ioa::make_bind_helper (this, sender, &udp_broadcast_automaton::open_result, m_self.get (), &file_copy::open_result);
    ioa::make_bind_helper (this, m_self.get (), &file_copy::send, sender, &udp_broadcast_automaton::send);
    ioa::make_bind_helper (this, sender, &udp_broadcast_automaton::send_result, m_self.get (), &file_copy::send_result);

    schedule ();
  }

};

int
main () {
  ioa::scheduler::run (ioa::make_generator<file_copy> ());
  return 0; 
}
