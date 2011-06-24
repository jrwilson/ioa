#ifndef __udp_receiver_automaton_hpp__
#define __udp_receiver_automaton_hpp__

#include <ioa/ioa.hpp>
#include <ioa/buffer.hpp>
#include <ioa/inet_address.hpp>

#include <fcntl.h>
#include <sys/ioctl.h>

#include <queue>
#include <algorithm>

namespace ioa {

  class udp_receiver_automaton :
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

  private:
    void prepare_socket (const inet_address& address);

  public:
    udp_receiver_automaton (const inet_address& address);
    udp_receiver_automaton (const inet_address& group_addr,
			    const inet_address& local_addr);
    ~udp_receiver_automaton ();

  private:
    bool do_recvfrom_precondition () const;
    void do_recvfrom_effect ();
    UP_INTERNAL (udp_receiver_automaton, do_recvfrom);

  private:
    bool receive_precondition () const;
    receive_val receive_effect ();

  public:
    V_UP_OUTPUT (udp_receiver_automaton, receive, receive_val);

  private:
    void schedule () const;
    void observe (observable* o);
  };

}

#endif
