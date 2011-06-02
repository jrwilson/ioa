#include <ioa/shared_mutex.hpp>
#include <cassert>

namespace ioa {
  
  shared_mutex::shared_mutex () {
    int r = pthread_rwlock_init (&m_lock, 0);
    assert (r == 0);
  }

  shared_mutex::~shared_mutex () {
    int r = pthread_rwlock_destroy (&m_lock);
    assert (r == 0);
  }
  
  void shared_mutex::unique_lock () {
    int r = pthread_rwlock_wrlock (&m_lock);
    assert (r == 0);
  }

  void shared_mutex::shared_lock () {
    int r = pthread_rwlock_rdlock (&m_lock);
    assert (r == 0);
  }
  
  void shared_mutex::unlock () {
    int r = pthread_rwlock_unlock (&m_lock);
    assert (r == 0);
  }
  
}
