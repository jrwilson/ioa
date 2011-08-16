#ifndef __tcp_acceptor_automaton_hpp__
#define __tcp_acceptor_automaton_hpp__

#include <ioa/tcp_connection_automaton.hpp>
#include <ioa/inet_address.hpp>
#include <fcntl.h>
#include <queue>

#define BACKLOG 5

namespace ioa {
  
  class tcp_acceptor_automaton :
    public automaton,
    private observer
  {
  public:
    struct accept_val
    {
      int err;
      inet_address address;
      automaton_handle<tcp_connection_automaton> handle;

      accept_val (const int e,
		  const inet_address& a,
		  const automaton_handle<tcp_connection_automaton>& h) :
	err (e),
	address (a),
	handle (h)
      { }
    };

  private:
    enum state_t {
      SCHEDULE_READ_READY,
      READ_READY_WAIT,
      CREATE_READY,
      CREATED_WAIT,
      ACCEPT_READY,
    };
    state_t m_state;
    int m_fd;
    int m_errno;
    inet_address m_address;
    int m_connection_fd;
    automaton_manager<tcp_connection_automaton>* m_connection;
    automaton_handle<tcp_connection_automaton> m_handle;

  public:
    tcp_acceptor_automaton (const ioa::inet_address& address);
    ~tcp_acceptor_automaton ();

  private:
    void schedule () const;
    void observe (observable* o);

  private:
    bool schedule_read_ready_precondition () const;
    void schedule_read_ready_effect ();
    void schedule_read_ready_schedule () const { schedule (); }
    UP_INTERNAL (tcp_acceptor_automaton, schedule_read_ready);

    bool read_ready_precondition () const;
    void read_ready_effect ();
    void read_ready_schedule () const { schedule (); }
    UP_INTERNAL (tcp_acceptor_automaton, read_ready);

    bool create_precondition () const;
    void create_effect ();
    void create_schedule () const { schedule (); }
    UP_INTERNAL (tcp_acceptor_automaton, create);

  private:
    bool accept_precondition () const;
    accept_val accept_effect ();
    void accept_schedule () const { schedule (); }
  public:
    V_UP_OUTPUT (tcp_acceptor_automaton, accept, accept_val);
  };

}

#endif
