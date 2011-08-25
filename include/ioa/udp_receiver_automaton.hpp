#ifndef __udp_receiver_automaton_hpp__
#define __udp_receiver_automaton_hpp__

#include <ioa/ioa.hpp>
#include <ioa/inet_address.hpp>
#include <queue>
#include <string>

namespace ioa {

  class udp_receiver_automaton :
    public automaton
  {
  public:
    struct receive_val {
      inet_address address;
      const_shared_ptr<std::string> buffer;

      receive_val (const inet_address& a,
		   const const_shared_ptr<std::string>& b) :
	address (a),
	buffer (b)
      { }
    };

  private:
    enum state_t {
      SCHEDULE_READ_READY,
      READ_READY_WAIT,
    };

    state_t m_state;
    int m_fd;
    int m_errno;
    char* m_buf;
    ssize_t m_buf_size;
    std::queue<receive_val> m_recv_queue;
    bool m_error_reported;

  private:
    void prepare_socket (const inet_address& address);
    void schedule () const;

  public:
    udp_receiver_automaton (const inet_address& address);
    udp_receiver_automaton (const inet_address& group_addr,
			    const inet_address& local_addr);
    ~udp_receiver_automaton ();

  private:
    bool receive_precondition () const;
    receive_val receive_effect ();
    void receive_schedule () const;
  public:
    V_UP_OUTPUT (udp_receiver_automaton, receive, receive_val);

  private:
    bool error_precondition () const;
    int error_effect ();
    void error_schedule () const;
  public:
    V_UP_OUTPUT (udp_receiver_automaton, error, int);

  private:
    bool schedule_read_ready_precondition () const;
    void schedule_read_ready_effect ();
    void schedule_read_ready_schedule () const;
    UP_INTERNAL (udp_receiver_automaton, schedule_read_ready);

    bool read_ready_precondition () const;
    void read_ready_effect ();
    void read_ready_schedule () const;
    UP_INTERNAL (udp_receiver_automaton, read_ready);
  };

}

#endif
