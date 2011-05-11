#ifndef __system_interface_hpp__
#define __system_interface_hpp__

#include "runnable.hpp"

namespace ioa {

  class system_interface
  {
  public:
    virtual ~system_interface () { }
    virtual void lock_automaton (const generic_automaton_handle& handle) = 0;
    virtual void unlock_automaton (const generic_automaton_handle& handle) = 0;
  };

  class scheduler_interface
  {
  public:
    virtual ~scheduler_interface () { }
    virtual void set_current_handle (const generic_automaton_handle& handle) = 0;
    virtual void clear_current_handle () = 0;
  };

  class internal_scheduler_interface :
    public scheduler_interface
  {
  public:
    virtual void schedule (runnable_interface*) = 0;
  };

}

#endif
