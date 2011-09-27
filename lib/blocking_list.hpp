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

#ifndef __blocking_list_hpp__
#define __blocking_list_hpp__

#include <ioa/mutex.hpp>
#include "condition_variable.hpp"
#include <list>

namespace ioa {
  
  template <class T>
  class blocking_list
  {
  private:
    condition_variable m_condition;
    
  public:
    std::list<T> list;
    mutex list_mutex;
    
    T pop () {
      lock lock (list_mutex);
      while (list.empty ()) {
	m_condition.wait (lock);
      }
      T retval = list.front ();
      list.pop_front ();
      return retval;
    }
    
    size_t push (const T& t) {
      size_t retval;
      {
	lock lock (list_mutex);
	list.push_back (t);
	retval = list.size ();
      }
      // Only one thread should be calling pop.
      m_condition.notify_one ();
      return retval;
    }
    
  };

}

#endif
