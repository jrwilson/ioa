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
      SCHEDULE_AFTER_READY,
      INTERRUPT_WAIT,
      ALARM_READY,
    };
    state_t m_state;
    const size_t m_fan_out;
    time m_interval;

  public:
    alarm_automaton (const size_t fan_out = 1);

  private:
    void schedule () const;

    void set_effect (const time& interval);
  public:
    V_UP_INPUT (alarm_automaton, set, time);

  private:    
    bool schedule_after_precondition () const;
    void schedule_after_effect ();
    UP_INTERNAL (alarm_automaton, schedule_after);

    bool interrupt_precondition () const;
    void interrupt_effect ();
    UP_INTERNAL (alarm_automaton, interrupt);

  private:
    bool alarm_precondition () const;
    void alarm_effect ();
  public:
    UV_UP_OUTPUT (alarm_automaton, alarm);
  };

}

#endif
