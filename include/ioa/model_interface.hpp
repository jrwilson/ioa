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

#ifndef __model_interface_hpp__
#define __model_interface_hpp__

#include <ioa/aid.hpp>
#include <memory>

namespace ioa {

  class automaton;
  class output_executor_interface;
  class internal_executor_interface;
  class generator_interface;
  class bind_executor_interface;
  class system_input_executor_interface;
  class input_executor_interface;

  class model_interface
  {
  public:
    virtual ~model_interface () { }

    virtual void add_bind_key (const aid_t binder,
			       void* const key) = 0;
    virtual void remove_bind_key (const aid_t binder,
				  void* const key) = 0;
    
    // Executing user actions.
    virtual automaton* get_instance (const aid_t aid) = 0;
    virtual void lock_automaton (const aid_t aid) = 0;
    virtual void unlock_automaton (const aid_t aid) = 0;
    virtual int execute (output_executor_interface& exec) = 0;
    virtual int execute (internal_executor_interface& exec) = 0;

    // Executing system outputs.
    virtual int execute_sys_create (const aid_t automaton) = 0;
    virtual int execute_sys_bind (const aid_t automaton) = 0;
    virtual int execute_sys_unbind (const aid_t automaton) = 0;
    virtual int execute_sys_destroy (const aid_t automaton) = 0;

    // Executing configuation actions.
    virtual aid_t create (const aid_t automaton,
			  std::auto_ptr<generator_interface> generator,
			  void* const key) = 0;
    virtual int bind (const aid_t automaton,
		      std::auto_ptr<bind_executor_interface> exec,
		      void* const key) = 0;
    virtual int unbind (const aid_t automaton,
			void* const key) = 0;
    virtual int destroy (const aid_t automaton,
			 void* const key) = 0;
    virtual int destroy (const aid_t automaton) = 0;

    // Executing system inputs.
    virtual int execute (system_input_executor_interface& exec) = 0;
    virtual int execute_output_bound (output_executor_interface& exec) = 0;
    virtual int execute_input_bound (input_executor_interface& exec) = 0;
    virtual int execute_output_unbound (output_executor_interface& exec) = 0;
    virtual int execute_input_unbound (input_executor_interface& exec) = 0;
  };

}

#endif
