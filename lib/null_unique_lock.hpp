#ifndef __null_unique_lock_hpp__
#define __nulL_unique_lock_hpp__

#include <ioa/unique_lock_interface.hpp>
#include <ioa/shared_mutex_interface.hpp>

namespace ioa {

  class null_unique_lock :
    public unique_lock_interface
  {
  public:
    null_unique_lock (shared_mutex_interface&) { }
  };

}

#endif
