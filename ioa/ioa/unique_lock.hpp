#ifndef __unique_lock_hpp__
#define __unique_lock_hpp__

class unique_lock
{
private:
  shared_mutex& m_mutex;

public:
  unique_lock (shared_mutex& mutex) :
    m_mutex (mutex)
  {
    m_mutex.unique_lock ();
  }

  ~unique_lock () {
    m_mutex.unlock ();
  }

};

#endif
