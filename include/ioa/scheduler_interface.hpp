#ifndef __scheduler_interface_hpp__
#define __scheduler_interface_hpp__

#include <ioa/aid.hpp>

namespace ioa {
  
  class scheduler_interface
  {
  public:
    virtual ~scheduler_interface () { }
    virtual void set_current_aid (const aid_t aid) = 0;
    //virtual aid_t get_current_aid () = 0;
    virtual void clear_current_aid () = 0;
    // virtual size_t binding_count (const action_executor_interface&) = 0;
    // virtual void schedule (action_runnable_interface*) = 0;
    // virtual void schedule_after (action_runnable_interface*,
    // 				 const time&) = 0;
    // virtual void schedule_read_ready (action_runnable_interface*,
    // 				      int fd) = 0;
    // virtual void schedule_write_ready (action_runnable_interface*,
    // 				       int fd) = 0;
    // virtual void close (int fd) = 0;
    // virtual void run (std::auto_ptr<generator_interface> generator) = 0;
  };
  
}

#endif
