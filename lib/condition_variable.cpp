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

#include "condition_variable.hpp"
#include "lock.hpp"
#include <cassert>

#include "profile.hpp"

namespace ioa {

  condition_variable::condition_variable () {
    BEGIN_SYS_CALL;
    int r = pthread_cond_init (&m_cond, 0);
    END_SYS_CALL;
    assert (r == 0);
  }

  condition_variable::~condition_variable () {
    BEGIN_SYS_CALL;
    int r = pthread_cond_destroy (&m_cond);
    END_SYS_CALL;
    assert (r == 0);
  }

  void condition_variable::wait (lock& lock) {
    BEGIN_SYS_CALL;
    int r = pthread_cond_wait (&m_cond, &lock.m_mutex.m_mutex);
    END_SYS_CALL;
    assert (r == 0);
  }

  void condition_variable::notify_one () {
    BEGIN_SYS_CALL;
    int r = pthread_cond_signal (&m_cond);
    END_SYS_CALL;
    assert (r == 0);
  }

}
