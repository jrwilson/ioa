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

#include "thread.hpp"

#include <cassert>

namespace ioa {

  void* thread_func (void* a) {
    assert (a != 0);
    thread_arg_interface* arg = static_cast<thread_arg_interface*> (a);
    (*arg) ();
    pthread_exit (0);
  }

  thread::~thread () {
    int r = pthread_attr_destroy (&m_attr);
    assert (r == 0);
  }
  
  void thread::join () {
    int r = pthread_join (m_thread, 0);
    assert (r == 0);
  }

  pthread_t thread::get_id () const {
    return m_thread;
  }
}
