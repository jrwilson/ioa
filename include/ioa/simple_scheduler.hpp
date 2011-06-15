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
    
    size_t bind_count (const action_interface&);
    
    void schedule (automaton_interface::sys_create_type automaton_interface::*ptr);
    
    void schedule (automaton_interface::sys_bind_type automaton_interface::*ptr);
    
    void schedule (automaton_interface::sys_unbind_type automaton_interface::*ptr);
    
    void schedule (automaton_interface::sys_destroy_type automaton_interface::*ptr);
    
    void schedule (action_runnable_interface*);
    
    void schedule_after (action_runnable_interface*,
			 const time&);
    
    void schedule_read_ready (action_runnable_interface*,
			      int fd);
    
    void schedule_write_ready (action_runnable_interface*,
			       int fd);
    
    void run (shared_ptr<generator_interface> generator);
  };

}

#endif
