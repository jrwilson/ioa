#ifndef __condition_variable_hpp__
#define __condition_varaible_hpp__

#include <pthread.h>

namespace ioa {

  class lock;
  
  class condition_variable
  {
  private:
    pthread_cond_t m_cond;
    
  public:
    condition_variable ();
    ~condition_variable ();
    void wait (lock& lock);
    void notify_one ();
  };

}

#endif
