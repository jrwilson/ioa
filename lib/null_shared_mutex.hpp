#ifndef __null_shared_mutex_hpp__
#define __null_shared_mutex_hpp__

#include <ioa/shared_mutex_interface.hpp>

namespace ioa {
  
  class null_shared_mutex :
    public shared_mutex_interface
  {

  };

}

#endif
