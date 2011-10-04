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
#include <ioa/global_fifo_scheduler.hpp>

// For std::cout.
#include <iostream>

class count_to_ten_automaton :
  public ioa::automaton
{
private:
  int m_count;

public:
  count_to_ten_automaton () :
    m_count (1) {
    increment_schedule ();
  }
  
private:
  bool increment_precondition () const {
    return m_count <= 10;
  }

  void increment_effect () {
    std::cout << m_count << std::endl;
    ++m_count;
  }

  void increment_schedule () const {
    if (increment_precondition ()) {
      ioa::schedule (&count_to_ten_automaton::increment);
    }
  }

  UP_INTERNAL (count_to_ten_automaton, increment);
};

int main () {
  ioa::global_fifo_scheduler sched;
  ioa::run (sched, ioa::make_allocator<count_to_ten_automaton> ());
  return 0; 
}
