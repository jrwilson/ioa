#ifndef __system_interface_hpp__
#define __system_interface_hpp__

#include "runnable.hpp"

namespace ioa {

  class system_interface
  {
  public:
    virtual ~system_interface () { }
    virtual void lock_automaton (const aid_t) = 0;
    virtual void unlock_automaton (const aid_t) = 0;
  };

  class scheduler_interface
  {
  public:
    virtual ~scheduler_interface () { }
    virtual void set_current_aid (const aid_t) = 0;
    virtual void clear_current_aid () = 0;
  };

}

#endif
