#ifndef __mutex_hpp__
#define __mutex_hpp__

#include <pthread.h>

namespace ioa {
  
  class mutex
  {
  private:
    pthread_mutex_t m_mutex;
    
  public:
    mutex ();
    ~mutex ();
    void lock ();
    void unlock ();
    
    friend class condition_variable;
  };

}

#endif
