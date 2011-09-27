/*
   Copyright 2011 Justin R. Wilson

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#include <ioa/alarm_automaton.hpp>

namespace ioa {

  alarm_automaton::alarm_automaton () :
    m_state (SET_WAIT)
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
    return m_state == ALARM_READY && binding_count (&alarm_automaton::alarm) != 0;
  }
  
  void alarm_automaton::alarm_effect () {
    m_state = SET_WAIT;
  }

}
