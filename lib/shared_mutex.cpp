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

#include <ioa/shared_mutex.hpp>
#include <cassert>

#include "profile.hpp"

namespace ioa {
  
  shared_mutex::shared_mutex () {
    BEGIN_SYS_CALL;
    int r = pthread_rwlock_init (&m_lock, 0);
    END_SYS_CALL;
    assert (r == 0);
  }

  shared_mutex::~shared_mutex () {
    BEGIN_SYS_CALL;
    int r = pthread_rwlock_destroy (&m_lock);
    END_SYS_CALL;
    assert (r == 0);
  }
  
  void shared_mutex::unique_lock () {
    BEGIN_SYS_CALL;
    int r = pthread_rwlock_wrlock (&m_lock);
    END_SYS_CALL;
    assert (r == 0);
  }

  void shared_mutex::shared_lock () {
    BEGIN_SYS_CALL;
    int r = pthread_rwlock_rdlock (&m_lock);
    END_SYS_CALL;
    assert (r == 0);
  }
  
  void shared_mutex::unlock () {
    BEGIN_SYS_CALL;
    int r = pthread_rwlock_unlock (&m_lock);
    END_SYS_CALL;
    assert (r == 0);
  }
  
}
