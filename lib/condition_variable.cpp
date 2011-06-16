#include "condition_variable.hpp"
#include "lock.hpp"
#include <cassert>

namespace ioa {

  condition_variable::condition_variable () {
    int r = pthread_cond_init (&m_cond, 0);
    assert (r == 0);
  }

  condition_variable::~condition_variable () {
    int r = pthread_cond_destroy (&m_cond);
    assert (r == 0);
  }

  void condition_variable::wait (lock& lock) {
    int r = pthread_cond_wait (&m_cond, &lock.m_mutex.m_mutex);
    assert (r == 0);
  }

  void condition_variable::notify_one () {
    int r = pthread_cond_signal (&m_cond);
    assert (r == 0);
  }

}
