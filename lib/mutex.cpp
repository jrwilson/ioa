#include "mutex.hpp"
#include <cassert>

namespace ioa {

  mutex::mutex () {
    int r = pthread_mutex_init (&m_mutex, 0);
    assert (r == 0);
  }
    
  mutex::~mutex () {
    int r = pthread_mutex_destroy (&m_mutex);
    assert (r == 0);
  }
    
  void mutex::lock () {
    int r = pthread_mutex_lock (&m_mutex);
    assert (r == 0);
  }
  
  void mutex::unlock () {
    int r = pthread_mutex_unlock (&m_mutex);
    assert (r == 0);
  }

}
