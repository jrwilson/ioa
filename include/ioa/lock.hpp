#ifndef __lock_hpp__
#define __lock_hpp__

#include "mutex.hpp"

class condition_variable;

class lock
{
private:
  mutex& m_mutex;

public:
  lock (mutex& m) :
    m_mutex (m)
  {
    m_mutex.lock ();
  }

  ~lock () {
    m_mutex.unlock ();
  }

  friend class condition_variable;
};

#endif
