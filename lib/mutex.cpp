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

#include <ioa/mutex.hpp>
#include <cassert>

#include "profile.hpp"

namespace ioa {

  mutex::mutex () {
    BEGIN_SYS_CALL;
    int r = pthread_mutex_init (&m_mutex, 0);
    END_SYS_CALL;
    assert (r == 0);
  }
    
  mutex::~mutex () {
    BEGIN_SYS_CALL;
    int r = pthread_mutex_destroy (&m_mutex);
    END_SYS_CALL;
    assert (r == 0);
  }
    
  void mutex::lock () {
    BEGIN_SYS_CALL;
    int r = pthread_mutex_lock (&m_mutex);
    END_SYS_CALL;
    assert (r == 0);
  }
  
  void mutex::unlock () {
    BEGIN_SYS_CALL;
    int r = pthread_mutex_unlock (&m_mutex);
    END_SYS_CALL;
    assert (r == 0);
  }

}
