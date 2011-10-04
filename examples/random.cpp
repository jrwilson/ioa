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

#include <ioa/ioa.hpp>
#include <ioa/simple_scheduler.hpp>

#include <iostream>
#include <cmath>

class random_automaton :
  public ioa::automaton
{
private:
  int const ACTIONS;
  int const CALCS;
  float const INTERNAL_FRACTION;

  int m_actions;
  int m_seed;

  void waste_time () {
    for (int i = 0; i < CALCS; ++i) {
      m_seed = m_seed * 1103515245 + 12345;
    }
  }

  void schedule () const {
    if (drand48 () < INTERNAL_FRACTION) {
      if (internal_precondition ()) {
	ioa::schedule (&random_automaton::internal);
      }
    }
    else {
      if (output_precondition ()) {
	ioa::schedule (&random_automaton::output);
      }
    }
  }

public:
  random_automaton (int const actions,
		    int const calcs,
		    float const internal_fraction) :
    ACTIONS (actions),
    CALCS (calcs),
    INTERNAL_FRACTION (sqrt (internal_fraction)),
    m_actions (0)
  {
    schedule ();
  }

private:
  bool internal_precondition () const {
    return m_actions != ACTIONS;
  }

  void internal_effect () {
    waste_time ();
    ++m_actions;
  }

  void internal_schedule () const {
    schedule ();
  }

  UP_INTERNAL (random_automaton, internal);

private:
  bool output_precondition () const {
    return m_actions != ACTIONS;
  }

  void output_effect () {
    waste_time ();
    ++m_actions;
  }

  void output_schedule () const {
    schedule ();
  }

public:
  UV_UP_OUTPUT (random_automaton, output);

private:
  void input_effect () {
  }

  void input_schedule () const {
    schedule ();
  }
public:
  UV_UP_INPUT (random_automaton, input);
};

class random_pair_automaton :
  public ioa::automaton
{
public:
  random_pair_automaton (int const actions,
			 int const calcs,
			 float const internal_fraction) {
    srand48 (time (0));
    ioa::automaton_manager<random_automaton>* r1 = ioa::make_automaton_manager (this, ioa::make_allocator<random_automaton> (actions, calcs, internal_fraction));
    ioa::automaton_manager<random_automaton>* r2 = ioa::make_automaton_manager (this, ioa::make_allocator<random_automaton> (actions, calcs, internal_fraction));
    ioa::make_binding_manager (this,
			       r1, &random_automaton::output,
			       r2, &random_automaton::input);
    ioa::make_binding_manager (this,
			       r2, &random_automaton::output,
			       r1, &random_automaton::input);
  }
};

int main (int argc,
	  char* argv[]) {
  if (argc != 5) {
    std::cerr << "Usage: " << argv[0] << " THREADS ACTIONS CALCS INTERNAL_FRACTION" << std::endl;
    exit (EXIT_FAILURE);
  }

  const int threads = atoi (argv[1]);
  const int actions = atoi (argv[2]);
  const int calcs = atoi (argv[3]);
  const float internal_fraction = atof (argv[4]);

  ioa::simple_scheduler sched (threads);
  ioa::run (sched, ioa::make_allocator<random_pair_automaton> (actions, calcs, internal_fraction));
  return 0;
}
