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

#ifndef __system_scheduler_interface_hpp__
#define __system_scheduler_interface_hpp__

#include <ioa/automaton.hpp>
#include <memory>

namespace ioa {

  /*
    Some actions are executed by the system and some are executed by bindings.
    Both need to advise the scheduler of the automaton that is currently executing.
  */

  class system_scheduler_interface
  {
  public:
    virtual ~system_scheduler_interface () { }
    virtual void set_current_aid (const aid_t aid) = 0;
    virtual void clear_current_aid () = 0;
    
    virtual void create (const aid_t automaton,
			 std::auto_ptr<allocator_interface> allocator,
			 void* const key) = 0;
    
    virtual void bind (const aid_t automaton,
		       std::auto_ptr<bind_executor_interface> exec,
		       void* const key) = 0;
    
    virtual void unbind (const aid_t automaton,
			 void* const key) = 0;
    
    virtual void destroy (const aid_t automaton,
			  void* const key) = 0;

    virtual void created (const aid_t automaton,
			  const created_t,
			  void* const key,
			  const aid_t child) = 0;
    
    virtual void bound (const aid_t automaton,
			const bound_t,
			void* const key) = 0;
    
    virtual void output_bound (const output_executor_interface&) = 0;
    
    virtual void input_bound (const input_executor_interface&) = 0;
    
    virtual void unbound (const aid_t automaton,
			  const unbound_t,
			  void* const key) = 0;
    
    virtual void output_unbound (const output_executor_interface&) = 0;
    
    virtual void input_unbound (const input_executor_interface&) = 0;
    
    virtual void destroyed (const aid_t automaton,
			    const destroyed_t,
			    void* const key) = 0;
  };

}

#endif
