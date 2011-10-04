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

#ifndef __simple_scheduler_hpp__
#define __simple_scheduler_hpp__

#include <ioa/scheduler_interface.hpp>

namespace ioa {

  class simple_scheduler_impl;

  class simple_scheduler :
    public scheduler_interface
  {
  private:
    simple_scheduler_impl* m_impl;
    
    simple_scheduler (const simple_scheduler&) { }
    void operator= (const simple_scheduler&) { }

  public:
    simple_scheduler (int const threads = 1);
    ~simple_scheduler ();
    
    aid_t get_current_aid ();
    
    size_t binding_count (const action_executor_interface&);
    
    void schedule (automaton::sys_create_type automaton::*ptr);
    
    void schedule (automaton::sys_bind_type automaton::*ptr);
    
    void schedule (automaton::sys_unbind_type automaton::*ptr);
    
    void schedule (automaton::sys_destroy_type automaton::*ptr);

    void schedule (action_runnable_interface*);
    
    void schedule_after (action_runnable_interface*,
			 const time&);
    
    void schedule_read_ready (action_runnable_interface*,
			      int fd);
    
    void schedule_write_ready (action_runnable_interface*,
			       int fd);

    void close (int fd);

    void run (std::auto_ptr<allocator_interface> allocator);

    void begin_sys_call ();

    void end_sys_call ();
  };

}

#endif
