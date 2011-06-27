#include <ioa/ioa.hpp>
#include <ioa/global_fifo_scheduler.hpp>
#include <ioa/inet_address.hpp>
#include <ioa/tcp_connector.hpp>

#include <iostream>

template <class T>
class auto_scheduler
{
private:
  const T& m_automaton;
  void (T::*m_schedule_ptr) () const;

public:
  auto_scheduler (const T& automaton,
		  void (T::*schedule_ptr) () const) :
    m_automaton (automaton),
    m_schedule_ptr (schedule_ptr)
  { }

  ~auto_scheduler () {
    (m_automaton.*m_schedule_ptr) ();
  }
};

class echo_client_automaton :
  public ioa::automaton
{
private:
  enum state_t {
    CONNECT_READY,
    CONNECT_COMPLETE_WAIT,
  };

  state_t m_state;

public:
  echo_client_automaton () :
    m_state (CONNECT_READY)
  {
    auto_scheduler<echo_client_automaton> as (*this, &echo_client_automaton::schedule);

    new ioa::automaton_manager<ioa::tcp_connector> (this, ioa::make_generator<ioa::tcp_connector> ());
  }

private:
  void schedule () const {
    if (connect_precondition ()) {
      ioa::schedule (&echo_client_automaton::connect);
    }
  }

  bool connect_precondition () const {
    return m_state == CONNECT_READY;
  }

  ioa::inet_address connect_effect () {
    auto_scheduler<echo_client_automaton> as (*this, &echo_client_automaton::schedule);
    m_state = CONNECT_COMPLETE_WAIT;
    return ioa::inet_address ("127.0.0.1", 54321);
  }

  V_UP_OUTPUT (echo_client_automaton, connect, ioa::inet_address);  
};

int main () {
  ioa::global_fifo_scheduler sched;
  ioa::run (sched, ioa::make_generator<echo_client_automaton> ());
  return 0;
}
