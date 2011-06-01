#ifndef __mutex_hpp__
#define __mutex_hpp__

class condition_variable;

class mutex
{
private:
  pthread_mutex_t m_mutex;
public:
  mutex () {
    int r = pthread_mutex_init (&m_mutex, 0);
    assert (r == 0);
  }

  ~mutex () {
    int r = pthread_mutex_destroy (&m_mutex);
    assert (r == 0);
  }

  void lock () {
    int r = pthread_mutex_lock (&m_mutex);
    assert (r == 0);
  }

  void unlock () {
    int r = pthread_mutex_unlock (&m_mutex);
    assert (r == 0);
  }

  friend class condition_variable;
};

#endif
