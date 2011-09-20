#include <ioa/shared_mutex.hpp>
#include <cassert>

#include "profile.hpp"

namespace ioa {
  
  shared_mutex::shared_mutex () {
    BEGIN_SYS_CALL;
    int r = pthread_rwlock_init (&m_lock, 0);
    END_SYS_CALL;
    assert (r == 0);
  }

  shared_mutex::~shared_mutex () {
    BEGIN_SYS_CALL;
    int r = pthread_rwlock_destroy (&m_lock);
    END_SYS_CALL;
    assert (r == 0);
  }
  
  void shared_mutex::unique_lock () {
    BEGIN_SYS_CALL;
    int r = pthread_rwlock_wrlock (&m_lock);
    END_SYS_CALL;
    assert (r == 0);
  }

  void shared_mutex::shared_lock () {
    BEGIN_SYS_CALL;
    int r = pthread_rwlock_rdlock (&m_lock);
    END_SYS_CALL;
    assert (r == 0);
  }
  
  void shared_mutex::unlock () {
    BEGIN_SYS_CALL;
    int r = pthread_rwlock_unlock (&m_lock);
    END_SYS_CALL;
    assert (r == 0);
  }
  
}
