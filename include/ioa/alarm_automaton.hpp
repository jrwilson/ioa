#ifndef __alarm_automaton_hpp__
#define __alarm_automaton_hpp__

#include <ioa/ioa.hpp>

namespace ioa {

  class alarm_automaton :
    public automaton
  {
  private:
    enum state_t {
      SET_WAIT,
      INTERRUPT_WAIT,
      ALARM_READY,
    };
    state_t m_state;

  public:
    alarm_automaton ();

  private:
    void set_effect (const time& interval);

  public:
    V_UP_INPUT (alarm_automaton, set, time);

  private:    
    // Treat like an input.
    bool interrupt_precondition () const;
    void interrupt_effect ();
    UP_INTERNAL (alarm_automaton, interrupt);
    bool alarm_precondition () const;
    void alarm_effect ();

  public:
    UV_UP_OUTPUT (alarm_automaton, alarm);

  private:
    void schedule () const;

  };

}

#endif
