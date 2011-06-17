#ifndef __action_runnable_interface_hpp__
#define __action_runnable_interface_hpp__

#include <ioa/runnable_interface.hpp>
#include <ioa/executor_interface.hpp>

namespace ioa {

  class action_runnable_interface :
    public runnable_interface
  {
  public:
    virtual ~action_runnable_interface () { }
    
    virtual const action_executor_interface& get_action () const = 0;
    
    bool operator== (const action_runnable_interface& x) const {
      return get_action () == x.get_action ();
    }
  };

}

#endif
