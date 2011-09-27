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

#ifndef __unbind_runnable_hpp__
#define __unbind_runnable_hpp__

#include <ioa/runnable_interface.hpp>

namespace ioa {
  
  class unbind_runnable :
    public runnable_interface
  {
  private:
    const aid_t m_automaton;
    void* const m_key;
    
  public:
    unbind_runnable (const aid_t automaton,
		     void* const key) :
      m_automaton (automaton),
      m_key (key)
    { }
    
    void operator() (model_interface& model) {
      model.unbind (m_automaton, m_key);
    }
  };
  
}

#endif
