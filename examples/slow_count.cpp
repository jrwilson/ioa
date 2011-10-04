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

class count_to_ten :
  public ioa::automaton
{
private:
  int m_count;

  void schedule () const {
    if (increment_precondition ()) {
      ioa::schedule_after (&count_to_ten::increment, ioa::time (1, 0));
    }
  }

  bool increment_precondition () const {
    return m_count <= 10;
  }

  void increment_effect () {
    std::cout << m_count << std::endl;
    if (increment_precondition ()) {
      ++m_count;
    }
  }

  void increment_schedule () const {
    schedule ();
  }

  UP_INTERNAL (count_to_ten, increment);

public:
  count_to_ten () :
    m_count (1)
  {
    schedule ();
  }
};

int
main () {
  ioa::simple_scheduler ss;
  ioa::run (ss, ioa::make_allocator<count_to_ten> ());
  return 0; 
}

