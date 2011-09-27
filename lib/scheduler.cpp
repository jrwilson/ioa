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

#include <ioa/scheduler.hpp>

namespace ioa {

  scheduler_interface* scheduler = 0;

  aid_t get_aid () {
    assert (scheduler != 0);
    return scheduler->get_current_aid ();
  }

  void schedule (automaton::sys_create_type automaton::*ptr) {
    assert (scheduler != 0);
    scheduler->schedule (ptr);
  }

  void schedule (automaton::sys_bind_type automaton::*ptr) {
    assert (scheduler != 0);
    scheduler->schedule (ptr);
  }

  void schedule (automaton::sys_unbind_type automaton::*ptr) {
    assert (scheduler != 0);
    scheduler->schedule (ptr);
  }

  void schedule (automaton::sys_destroy_type automaton::*ptr) {
    assert (scheduler != 0);
    scheduler->schedule (ptr);
  }

  void close (int fd) {
    assert (scheduler != 0);
    scheduler->close (fd);
  }

}
