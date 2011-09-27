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

#ifndef __runnable_interface_hpp__
#define __runnable_interface_hpp__

#include <ioa/shared_mutex.hpp>

namespace ioa {

  class model_interface;

  class runnable_interface
  {
  private:
    // We keep track of the number of runnables in existence.
    // When the count reaches 0, we can stop.
    static shared_mutex m_mutex;
    static size_t m_count;

  public:
    runnable_interface ();
    virtual ~runnable_interface ();
    static size_t count ();
    virtual void operator() (model_interface& model) = 0;
  };

}

#endif
