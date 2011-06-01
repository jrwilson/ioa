#ifndef __shared_mutex_hpp__
#define __shared_mutex_hpp__

class shared_mutex
{
private:
  pthread_rwlock_t m_lock;

public:
  shared_mutex () {
    int r = pthread_rwlock_init (&m_lock, 0);
    assert (r == 0);
  }

  ~shared_mutex () {
    int r = pthread_rwlock_destroy (&m_lock);
    assert (r == 0);
  }

  void unique_lock () {
    int r = pthread_rwlock_wrlock (&m_lock);
    assert (r == 0);
  }

  void shared_lock () {
    int r = pthread_rwlock_rdlock (&m_lock);
    assert (r == 0);
  }

  void unlock () {
    int r = pthread_rwlock_unlock (&m_lock);
    assert (r == 0);
  }

};

#endif
