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

#ifndef __action_runnable_interface_hpp__
#define __action_runnable_interface_hpp__

#include <ioa/runnable_interface.hpp>
#include <ioa/executor_interface.hpp>

namespace ioa {

  class action_runnable_interface :
    public runnable_interface
  {
  public:
    virtual ~action_runnable_interface () { }
    
    virtual const action_executor_interface& get_action () const = 0;
    
    bool operator== (const action_runnable_interface& x) const {
      return get_action () == x.get_action ();
    }

    bool operator< (const action_runnable_interface& x) const {
      return get_action () < x.get_action ();
    }
  };

}

#endif
