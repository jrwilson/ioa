#include <ioa/mutex.hpp>
#include <cassert>

#include "profile.hpp"

namespace ioa {

  mutex::mutex () {
    BEGIN_SYS_CALL;
    int r = pthread_mutex_init (&m_mutex, 0);
    END_SYS_CALL;
    assert (r == 0);
  }
    
  mutex::~mutex () {
    BEGIN_SYS_CALL;
    int r = pthread_mutex_destroy (&m_mutex);
    END_SYS_CALL;
    assert (r == 0);
  }
    
  void mutex::lock () {
    BEGIN_SYS_CALL;
    int r = pthread_mutex_lock (&m_mutex);
    END_SYS_CALL;
    assert (r == 0);
  }
  
  void mutex::unlock () {
    BEGIN_SYS_CALL;
    int r = pthread_mutex_unlock (&m_mutex);
    END_SYS_CALL;
    assert (r == 0);
  }

}
