#ifndef __shared_mutex_hpp__
#define __shared_mutex_hpp__

#include <pthread.h>

namespace ioa {
  
  class shared_mutex
  {
  private:
    pthread_rwlock_t m_lock;
    
  public:
    shared_mutex ();
    ~shared_mutex ();
    void unique_lock ();
    void shared_lock ();
    void unlock ();
  };

}

#endif
