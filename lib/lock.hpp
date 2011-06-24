#ifndef __lock_hpp__
#define __lock_hpp__

#include <ioa/mutex.hpp>

namespace ioa {
  
  class lock
  {
  private:
    mutex& m_mutex;
    
  public:
    lock (mutex& m);
    ~lock ();
    friend class condition_variable;
  };

}

#endif
