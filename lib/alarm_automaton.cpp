#include <ioa/alarm_automaton.hpp>

namespace ioa {

  alarm_automaton::alarm_automaton (const size_t fan_out) :
    m_state (SET_WAIT),
    m_fan_out (fan_out)
  { }

  void alarm_automaton::schedule () const {
    if (schedule_after_precondition ()) {
      ioa::schedule (&alarm_automaton::schedule_after);
    }
    if (alarm_precondition ()) {
      ioa::schedule (&alarm_automaton::alarm);
    }
  }

  void alarm_automaton::set_effect (const time& interval) {
    if (m_state == SET_WAIT) {
      m_state = SCHEDULE_AFTER_READY;
      m_interval = interval;
    }
  }

  bool alarm_automaton::schedule_after_precondition () const {
    return m_state == SCHEDULE_AFTER_READY;
  }

  void alarm_automaton::schedule_after_effect () {
    ioa::schedule_after (&alarm_automaton::interrupt, m_interval);
    m_state = INTERRUPT_WAIT;
  }

  bool alarm_automaton::interrupt_precondition () const {
    return m_state == INTERRUPT_WAIT;
  }
  
  void alarm_automaton::interrupt_effect () { 
    m_state = ALARM_READY;
  }
  
  bool alarm_automaton::alarm_precondition () const {
    return m_state == ALARM_READY && binding_count (&alarm_automaton::alarm) == m_fan_out;
  }
  
  void alarm_automaton::alarm_effect () {
    m_state = SET_WAIT;
  }

}
