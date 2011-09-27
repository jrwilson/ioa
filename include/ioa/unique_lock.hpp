#ifndef __unique_lock_hpp__
#define __unique_lock_hpp__

#include <ioa/shared_mutex.hpp>

namespace ioa {
  
  class unique_lock
  {
  private:
    shared_mutex& m_mutex;
    
  public:
    unique_lock (shared_mutex& mutex);
    ~unique_lock ();
  };
  
}

#endif
