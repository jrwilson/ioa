#include "shared_lock.hpp"

namespace ioa {

  shared_lock::shared_lock (shared_mutex& mutex) :
    m_mutex (mutex)
  {
    m_mutex.shared_lock ();
  }

  shared_lock::~shared_lock () {
    m_mutex.unlock ();
  }

}
