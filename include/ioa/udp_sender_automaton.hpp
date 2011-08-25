#ifndef __udp_sender_automaton_hpp__
#define __udp_sender_automaton_hpp__

#include <ioa/ioa.hpp>
#include <ioa/inet_address.hpp>
#include <list>
#include <string>

namespace ioa {

  class udp_sender_automaton :
    public automaton,
    private observer
  {
  public:
    struct send_arg {
      inet_address address;
      const_shared_ptr<std::string> buffer;

      send_arg (const inet_address& a,
		const const_shared_ptr<std::string>& b) :
	address (a),
	buffer (b)
      { }
    };

  private:
    enum state_t {
      SCHEDULE_WRITE_READY,
      WRITE_READY_WAIT,
    };

    struct first_aid_equal {
      const aid_t m_aid;
      
      first_aid_equal (const aid_t aid) :
	m_aid (aid)
      { }

      bool operator() (const std::pair<aid_t, send_arg>& o) const {
	return m_aid == o.first;
      }
    };

    std::list<std::pair<aid_t, send_arg> > m_send_queue;
    std::set<aid_t> m_send_set; // Set of aids in send_queue.
    std::set<aid_t> m_complete_set;

    state_t m_state;
    int m_fd;
    int m_errno;
    bool m_error_reported;

    void schedule () const;
    void observe (observable* o);
    void purge (const aid_t aid);
    void add_to_complete_set (const aid_t aid);

  public:
    udp_sender_automaton (const size_t send_buf_size = 0);
    ~udp_sender_automaton ();

  private:
    void send_effect (const send_arg& arg, aid_t aid);
    void send_schedule (aid_t) const;
  public:
    V_AP_INPUT (udp_sender_automaton, send, send_arg);

  private:
    bool send_complete_precondition (aid_t aid) const;
    void send_complete_effect (aid_t aid);
    void send_complete_schedule (aid_t) const;
  public:
    UV_AP_OUTPUT (udp_sender_automaton, send_complete);

  private:
    bool error_precondition () const;
    int error_effect ();
    void error_schedule () const;
  public:
    V_UP_OUTPUT (udp_sender_automaton, error, int);

  private:
    bool schedule_write_ready_precondition () const;
    void schedule_write_ready_effect ();
    void schedule_write_ready_schedule () const;
    UP_INTERNAL (udp_sender_automaton, schedule_write_ready);

    bool write_ready_precondition () const;
    void write_ready_effect ();
    void write_ready_schedule () const;
    UP_INTERNAL (udp_sender_automaton, write_ready);
  };

}

#endif
