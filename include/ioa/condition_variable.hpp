#ifndef __condition_variable_hpp__
#define __condition_varaible_hpp__

#include <pthread.h>

#include "lock.hpp"

class condition_variable
{
private:
  pthread_cond_t m_cond;

public:
  condition_variable () {
    int r = pthread_cond_init (&m_cond, 0);
    assert (r == 0);
  }

  ~condition_variable () {
    int r = pthread_cond_destroy (&m_cond);
    assert (r == 0);
  }

  void wait (lock& lock) {
    int r = pthread_cond_wait (&m_cond, &lock.m_mutex.m_mutex);
    assert (r == 0);
  }

  void notify_one () {
    int r = pthread_cond_signal (&m_cond);
    assert (r == 0);
  }
};

#endif
