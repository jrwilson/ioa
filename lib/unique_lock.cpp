#include "unique_lock.hpp"

namespace ioa {

  unique_lock::unique_lock (shared_mutex& mutex) :
    m_mutex (mutex)
  {
    m_mutex.unique_lock ();
  }

  unique_lock::~unique_lock () {
    m_mutex.unlock ();
  }

}
