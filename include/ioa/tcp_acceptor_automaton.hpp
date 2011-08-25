#ifndef __tcp_acceptor_automaton_hpp__
#define __tcp_acceptor_automaton_hpp__

#include <ioa/tcp_connection_automaton.hpp>
#include <ioa/inet_address.hpp>
#include <queue>

namespace ioa {
  
  class tcp_acceptor_automaton :
    public automaton,
    private observer
  {
  public:
    struct accept_val
    {
      inet_address address;
      automaton_handle<tcp_connection_automaton> handle;

      accept_val (const inet_address& a,
		  const automaton_handle<tcp_connection_automaton>& h) :
	address (a),
	handle (h)
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
    bool m_error_reported;
    handle_manager<tcp_acceptor_automaton> m_self;
    std::map<automaton_manager<tcp_connection_automaton>*, inet_address> m_address_map;
    std::queue<std::pair<inet_address, automaton_manager<tcp_connection_automaton>*> > m_accept_queue;

    void schedule () const;
    void observe (observable* o);

  public:
    tcp_acceptor_automaton (const ioa::inet_address& address,
			    const int backlog = 5);
    ~tcp_acceptor_automaton ();

  private:
    bool accept_precondition () const;
    accept_val accept_effect ();
    void accept_schedule () const;
  public:
    V_UP_OUTPUT (tcp_acceptor_automaton, accept, accept_val);

  private:
    bool error_precondition () const;
    int error_effect ();
    void error_schedule () const;
  public:
    V_UP_OUTPUT (tcp_acceptor_automaton, error, int);

  private:
    bool schedule_read_ready_precondition () const;
    void schedule_read_ready_effect ();
    void schedule_read_ready_schedule () const;
    UP_INTERNAL (tcp_acceptor_automaton, schedule_read_ready);

    bool read_ready_precondition () const;
    void read_ready_effect ();
    void read_ready_schedule () const;
    UP_INTERNAL (tcp_acceptor_automaton, read_ready);

    void closed_effect (automaton_manager<tcp_connection_automaton>*);
    void closed_schedule (automaton_manager<tcp_connection_automaton>*) const;
    UV_P_INPUT (tcp_acceptor_automaton, closed, automaton_manager<tcp_connection_automaton>*);
  };

}

#endif
