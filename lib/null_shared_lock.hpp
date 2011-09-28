#ifndef __null_shared_lock_hpp__
#define __null_shared_lock_hpp__

#include <ioa/shared_lock_interface.hpp>
#include <ioa/shared_mutex_interface.hpp>

namespace ioa {

  class null_shared_lock :
    public shared_lock_interface
  {
  public:
    null_shared_lock (shared_mutex_interface&) { }
  };

}

#endif
