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

#ifndef __create_runnable_hpp__
#define __create_runnable_hpp__

#include <ioa/runnable_interface.hpp>

namespace ioa {
  
  class create_runnable :
    public runnable_interface
  {
  private:
    const aid_t m_automaton;
    std::auto_ptr<generator_interface> m_generator;
    void* const m_key;
  public:
    create_runnable (const aid_t automaton,
		     std::auto_ptr<generator_interface> generator,
		     void* const key) :
      m_automaton (automaton),
      m_generator (generator),
      m_key (key)
    { }
    
    void operator() (model_interface& model) {
      model.create (m_automaton, m_generator, m_key);
    }
  };
  
}

#endif
