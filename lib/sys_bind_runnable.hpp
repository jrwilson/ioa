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

#ifndef __sys_bind_runnable_hpp__
#define __sys_bind_runnable_hpp__

#include <ioa/action_runnable.hpp>
#include <ioa/automaton.hpp>

namespace ioa {
  
  class sys_bind_runnable :
    public action_runnable_interface
  {
  private:
    const aid_t m_automaton;
    const action_executor<automaton, automaton::sys_bind_type> m_action;

  public:
    sys_bind_runnable (const aid_t automaton) :
      m_automaton (automaton),
      m_action (automaton, &automaton::sys_bind)
    { }
    
    void operator() (model_interface& model) {
      model.execute_sys_bind (m_automaton);
    }

    const action_executor_interface& get_action () const {
      return m_action;
    }
  };
  
}

#endif
