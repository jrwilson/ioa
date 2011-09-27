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

#ifndef __automaton_manager_interface_hpp__
#define __automaton_manager_interface_hpp__

#include <ioa/automaton_handle.hpp>

namespace ioa {

  class automaton_manager_interface
  {
  public:
    enum state_t {
      START,
      INSTANCE_EXISTS,
      CREATED,
      DESTROYED
    };
    virtual ~automaton_manager_interface () { }
    virtual void destroy () = 0;
    virtual state_t get_state () const = 0;
  };
  
  template <class I>
  class automaton_handle_interface :
    public observable
  {
  public:
    virtual ~automaton_handle_interface () { }
    virtual automaton_handle<I> get_handle () const = 0;
  };


}

#endif
