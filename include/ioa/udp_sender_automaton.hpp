#ifndef __udp_sender_automaton_hpp__
#define __udp_sender_automaton_hpp__

#include <ioa/ioa.hpp>
#include <ioa/buffer.hpp>
#include <ioa/inet_address.hpp>

#include <fcntl.h>

#include <list>
#include <algorithm>

namespace ioa {

  class udp_sender_automaton :
    public automaton,
    private observer
  {
  public:
    struct send_arg {
      inet_address address;
      ioa::buffer buffer;

      send_arg (const inet_address& a,
		const ioa::buffer& b) :
	address (a),
	buffer (b)
      { }

      send_arg (const send_arg& other) :
	address (other.address),
	buffer (other.buffer)
      { }

      send_arg& operator= (const send_arg& other) {
	if (this != &other) {
	  address = other.address;
	  buffer = other.buffer;
	}
	return *this;
      }
    };

  private:
    enum state_t {
      SCHEDULE_WRITE_READY,
      WRITE_WAIT,
    };
    state_t m_state;
    int m_fd;
    int m_errno;

    std::list<std::pair<aid_t, send_arg*> > m_send_queue;
    std::set<aid_t> m_send_set; // Set of aids in send_queue.
    
    std::map<aid_t, int> m_complete_map;

  public:
    udp_sender_automaton ();
    ~udp_sender_automaton ();

  private:
    void add_to_send_queue (const aid_t aid,
			    const send_arg& arg);
    void add_to_complete_map (const aid_t aid,
			      const int err_no);
    void send_effect (const send_arg& arg, aid_t aid);

  public:
    V_AP_INPUT (udp_sender_automaton, send, send_arg);

  private:
    // Treat like an output.
    bool schedule_write_ready_precondition () const;
    void schedule_write_ready_effect ();
    UP_INTERNAL (udp_sender_automaton, schedule_write_ready);

    // Treat like an input.
    bool write_precondition () const;
    void write_effect ();
    UP_INTERNAL (udp_sender_automaton, write);

    bool send_complete_precondition (aid_t aid) const;
    int send_complete_effect (aid_t aid);

  public:
    V_AP_OUTPUT (udp_sender_automaton, send_complete, int);

  private:
    void schedule () const;

    struct first_aid_equal {
      const aid_t m_aid;
      
      first_aid_equal (const aid_t aid) :
	m_aid (aid)
      { }

      bool operator() (const std::pair<aid_t, send_arg*>& o) const {
	return m_aid == o.first;
      }

      bool operator() (const std::pair<aid_t, int>& o) const {
	return m_aid == o.first;
      }
    };

    void purge (const aid_t aid);
    void observe (observable* o);
  };

}

#endif
