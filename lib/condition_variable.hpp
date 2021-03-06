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

#ifndef __condition_variable_hpp__
#define __condition_varaible_hpp__

#include <pthread.h>

namespace ioa {

  class lock;
  
  class condition_variable
  {
  private:
    pthread_cond_t m_cond;
    
  public:
    condition_variable ();
    ~condition_variable ();
    void wait (lock& lock);
    void notify_one ();
  };

}

#endif
