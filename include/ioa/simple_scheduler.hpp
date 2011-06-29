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
    simple_scheduler ();
    ~simple_scheduler ();
    
    aid_t get_current_aid ();
    
    size_t binding_count (const action_executor_interface&);
    
    void schedule (automaton::sys_create_type automaton::*ptr);
    
    void schedule (automaton::sys_bind_type automaton::*ptr);
    
    void schedule (automaton::sys_unbind_type automaton::*ptr);
    
    void schedule (automaton::sys_destroy_type automaton::*ptr);

    void schedule (automaton::sys_self_destruct_type automaton::*ptr);
    
    void schedule (action_runnable_interface*);
    
    void schedule_after (action_runnable_interface*,
			 const time&);
    
    void schedule_read_ready (action_runnable_interface*,
			      int fd);
    
    void schedule_write_ready (action_runnable_interface*,
			       int fd);

    void close (int fd);

    void run (const_shared_ptr<generator_interface> generator);
  };

}

#endif
