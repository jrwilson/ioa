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

#include <iostream>

#include <ioa/ioa.hpp>
#include <ioa/global_fifo_scheduler.hpp>

class trigger :
  public ioa::automaton
{
private:

  bool request_precondition () const {
    return true;
  }
  
  void request_effect () { }

  void request_schedule () const {
    if (request_precondition ()) {
      ioa::schedule (&trigger::request);
    }
  }

public:
  trigger () {
    request_schedule ();
  }

  UV_UP_OUTPUT (trigger, request);
};

/*
  Clock Automaton
  Distributed Algorithms, p. 213.
*/

class clock_automaton :
  public ioa::automaton
{
private:
  int m_counter;
  int m_flag;

public:

  clock_automaton () :
    m_counter (0),
    m_flag (false)
  {
    schedule ();
  }

private:
  void schedule () const {
    if (tick_precondition ()) {
      ioa::schedule (&clock_automaton::tick);
    }
    if (clock_precondition ()) {
      ioa::schedule (&clock_automaton::clock);
    }
  }

  void request_effect () {
    m_flag = true;
  }

  void request_schedule () const {
    schedule ();
  }

public:
  UV_UP_INPUT (clock_automaton, request);

private:
  bool tick_precondition () const {
    return true;
  }
  
  void tick_effect () {
    m_counter = m_counter + 1;
  }

  void tick_schedule () const {
    schedule ();
  }

public:
  UP_INTERNAL (clock_automaton, tick);

private:
  bool clock_precondition () const {
    return m_flag;
  }

  int clock_effect () {
    m_flag = false;
    return m_counter;
  }

  void clock_schedule () const {
    schedule ();
  }

public:
  V_UP_OUTPUT (clock_automaton, clock, int);
};


class display :
  public ioa::automaton
{
private:
  void clock_effect (int const & t) {
    std::cout << "t = " << t << std::endl;
  }

  void clock_schedule () const { }

public:
  V_UP_INPUT (display, clock, int);
  
};


class composer :
  public ioa::automaton
{
public:
  composer ()
  {
    ioa::automaton_manager<trigger>* t = new ioa::automaton_manager<trigger> (this, ioa::make_allocator<trigger> ());
    ioa::automaton_manager<clock_automaton>* c = new ioa::automaton_manager<clock_automaton> (this, ioa::make_allocator<clock_automaton> ());
    ioa::automaton_manager<display>* d = new ioa::automaton_manager<display> (this, ioa::make_allocator<display> ());
    ioa::make_binding_manager (this, t, &trigger::request, c, &clock_automaton::request);
    ioa::make_binding_manager (this, c, &clock_automaton::clock, d, &display::clock);
  }

};

int
main () {
  ioa::global_fifo_scheduler sched;
  ioa::run (sched, ioa::make_allocator<composer> ());
  return 0; 
}
