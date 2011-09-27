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

#ifndef __unique_lock_hpp__
#define __unique_lock_hpp__

#include <ioa/shared_mutex.hpp>

namespace ioa {
  
  class unique_lock
  {
  private:
    shared_mutex& m_mutex;
    
  public:
    unique_lock (shared_mutex& mutex);
    ~unique_lock ();
  };
  
}

#endif
