#ifndef __null_mutex_hpp__
#define __null_mutex_hpp__

#include <ioa/mutex_interface.hpp>

namespace ioa {

  class null_mutex :
    public mutex_interface
  {
  public:
    void lock () { }
    void unlock () { }
  };
}

#endif
