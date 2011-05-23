#ifndef __scheduler_interface_hpp__
#define __scheduler_interface_hpp__

#include "automaton_handle.hpp"

namespace ioa {
  
  class scheduler_interface
  {
  public:
    virtual ~scheduler_interface () { }
    virtual void set_current_aid (const aid_t) = 0;
    virtual void set_current_aid (const aid_t,
				  const automaton_interface*) = 0;
    virtual void clear_current_aid () = 0;
  };
  
}

#endif
