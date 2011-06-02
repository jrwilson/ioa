#ifndef __shared_lock_hpp__
#define __shared_lock_hpp__

#include <ioa/shared_mutex.hpp>

namespace ioa {
  
  class shared_lock
  {
  private:
    shared_mutex& m_mutex;
    
  public:
    shared_lock (shared_mutex& mutex);
    ~shared_lock ();
  };
  
}

#endif
