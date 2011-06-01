#ifndef __system_interface_hpp__
#define __system_interface_hpp__

#include "automaton_handle.hpp"

namespace ioa {

  class system_interface
  {
  public:
    virtual ~system_interface () { }
    virtual void lock_automaton (const aid_t) = 0;
    virtual void unlock_automaton (const aid_t) = 0;
  };

}

#endif
