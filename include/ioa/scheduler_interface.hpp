#ifndef __scheduler_interface_hpp__
#define __scheduler_interface_hpp__

#include <ioa/action_runnable_interface.hpp>
#include <ioa/automaton.hpp>
#include <ioa/time.hpp>

namespace ioa {
  
  class scheduler_interface
  {
  public:
    virtual ~scheduler_interface () { }

    virtual aid_t get_current_aid () = 0;

    virtual size_t bind_count (const action_executor_interface&) = 0;

    virtual void schedule (automaton::sys_create_type automaton::*ptr) = 0;

    virtual void schedule (automaton::sys_bind_type automaton::*ptr) = 0;

    virtual void schedule (automaton::sys_unbind_type automaton::*ptr) = 0;

    virtual void schedule (automaton::sys_destroy_type automaton::*ptr) = 0;

    virtual void schedule (automaton::sys_self_destruct_type automaton::*ptr) = 0;

    virtual void schedule (action_runnable_interface*) = 0;

    virtual void schedule_after (action_runnable_interface*,
				 const time&) = 0;
    
    virtual void schedule_read_ready (action_runnable_interface*,
				      int fd) = 0;
    
    virtual void schedule_write_ready (action_runnable_interface*,
				       int fd) = 0;

    virtual void close (int fd) = 0;

    virtual void run (const_shared_ptr<generator_interface> generator) = 0;
  };
  
}

#endif
