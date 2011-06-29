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
    };

  private:
    enum state_t {
      SEND_WAIT,
      SCHEDULE_WRITE_READY,
      WRITE_READY_WAIT,
    };

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
    void schedule () const;
    void observe (observable* o);
    void purge (const aid_t aid);
    void add_to_send_queue (const aid_t aid,
			    const send_arg& arg);
    void add_to_complete_map (const aid_t aid,
			      const int err_no);

  private:
    void send_effect (const send_arg& arg, aid_t aid);
  public:
    V_AP_INPUT (udp_sender_automaton, send, send_arg);

  private:
    bool schedule_write_ready_precondition () const;
    void schedule_write_ready_effect ();
    UP_INTERNAL (udp_sender_automaton, schedule_write_ready);

    bool write_ready_precondition () const;
    void write_ready_effect ();
    UP_INTERNAL (udp_sender_automaton, write_ready);

  private:
    bool send_complete_precondition (aid_t aid) const;
    int send_complete_effect (aid_t aid);
  public:
    V_AP_OUTPUT (udp_sender_automaton, send_complete, int);
  };

}

#endif
