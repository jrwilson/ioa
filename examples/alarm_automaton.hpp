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
    time m_interval;

  public:
    alarm_automaton ();

  private:
    void schedule () const;

    void set_effect (const time& interval);
    void set_schedule () const { schedule (); }
  public:
    V_UP_INPUT (alarm_automaton, set, time);

  private:    
    bool schedule_after_precondition () const;
    void schedule_after_effect ();
    void schedule_after_schedule () const { schedule (); }
    UP_INTERNAL (alarm_automaton, schedule_after);

    bool interrupt_precondition () const;
    void interrupt_effect ();
    void interrupt_schedule () const { schedule (); }
    UP_INTERNAL (alarm_automaton, interrupt);

  private:
    bool alarm_precondition () const;
    void alarm_effect ();
    void alarm_schedule () const { schedule (); }
  public:
    UV_UP_OUTPUT (alarm_automaton, alarm);
  };

}

#endif
