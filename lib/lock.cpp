#include <ioa/lock.hpp>

namespace ioa {

  lock::lock (mutex& m) :
    m_mutex (m)
  {
    m_mutex.lock ();
  }

  lock::~lock () {
    m_mutex.unlock ();
  }

}
