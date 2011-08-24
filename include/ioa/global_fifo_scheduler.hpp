#ifndef __global_fifo_scheduler_hpp__
#define __global_fifo_scheduler_hpp__

#include <ioa/scheduler_interface.hpp>
#include <memory>

namespace ioa {

  class global_fifo_scheduler_impl;

  class global_fifo_scheduler :
    public scheduler_interface
  {
  private:
    global_fifo_scheduler_impl* m_impl;
    
    global_fifo_scheduler (const global_fifo_scheduler&) { }
    void operator= (const global_fifo_scheduler&) { }

  public:
    global_fifo_scheduler ();
    ~global_fifo_scheduler ();
    
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
    
    void run (std::auto_ptr<generator_interface> generator);
  };

}

#endif
