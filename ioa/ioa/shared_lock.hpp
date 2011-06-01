#ifndef __shared_lock_hpp__
#define __shared_lock_hpp__

class shared_lock
{
private:
  shared_mutex& m_mutex;

public:
  shared_lock (shared_mutex& mutex) :
    m_mutex (mutex)
  {
    m_mutex.shared_lock ();
  }

  ~shared_lock () {
    m_mutex.unlock ();
  }
};

#endif
