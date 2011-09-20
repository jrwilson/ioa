#include "condition_variable.hpp"
#include "lock.hpp"
#include <cassert>

#include "profile.hpp"

namespace ioa {

  condition_variable::condition_variable () {
    BEGIN_SYS_CALL;
    int r = pthread_cond_init (&m_cond, 0);
    END_SYS_CALL;
    assert (r == 0);
  }

  condition_variable::~condition_variable () {
    BEGIN_SYS_CALL;
    int r = pthread_cond_destroy (&m_cond);
    END_SYS_CALL;
    assert (r == 0);
  }

  void condition_variable::wait (lock& lock) {
    BEGIN_SYS_CALL;
    int r = pthread_cond_wait (&m_cond, &lock.m_mutex.m_mutex);
    END_SYS_CALL;
    assert (r == 0);
  }

  void condition_variable::notify_one () {
    BEGIN_SYS_CALL;
    int r = pthread_cond_signal (&m_cond);
    END_SYS_CALL;
    assert (r == 0);
  }

}
