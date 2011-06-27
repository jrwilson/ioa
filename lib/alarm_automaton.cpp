#include <ioa/alarm_automaton.hpp>

namespace ioa {

  alarm_automaton::alarm_automaton () :
    m_state (SET_WAIT)
  {
    schedule ();
  }

  void alarm_automaton::set_effect (const time& interval) {
    if (m_state == SET_WAIT) {
      m_state = INTERRUPT_WAIT;
      schedule_after (&alarm_automaton::interrupt, interval);
    }
  }

  // Treat like an input.
  bool alarm_automaton::interrupt_precondition () const {
    return true;
  }
  
  void alarm_automaton::interrupt_effect () { 
    m_state = ALARM_READY;
  }
  
  bool alarm_automaton::alarm_precondition () const {
    return m_state == ALARM_READY && bind_count (&alarm_automaton::alarm) != 0;
  }
  
  void alarm_automaton::alarm_effect () {
    m_state = SET_WAIT;
  }

  void alarm_automaton::schedule () const {
    if (alarm_precondition ()) {
      ioa::schedule (&alarm_automaton::alarm);
    }
  }

}
