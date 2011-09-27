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

#ifndef __handle_manager_hpp__
#define __handle_manager_hpp__

#include <ioa/automaton_manager_interface.hpp>
#include <ioa/scheduler.hpp>

namespace ioa {
  
  template <class T>
  class handle_manager :
    public automaton_handle_interface<T>
  {
  private:
    automaton_handle<T> m_handle;

  public:
    typedef T instance;

    handle_manager () { }

    handle_manager (const automaton_handle<T>& handle) :
      m_handle (handle)
    { }
    
    automaton_handle<T> get_handle () const {
      return m_handle;
    }
    
  };

}

#endif
