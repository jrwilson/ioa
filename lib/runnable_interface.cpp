#include <ioa/runnable_interface.hpp>

namespace ioa {

  shared_mutex runnable_interface::m_mutex;
  size_t runnable_interface::m_count = 0;

  runnable_interface::runnable_interface ()
  {
    unique_lock lock (m_mutex);
    ++m_count;
  }

  runnable_interface::~runnable_interface () {
    unique_lock lock (m_mutex);
    --m_count;
  }

  size_t runnable_interface::count () {
    shared_lock lock (m_mutex);
    return m_count;
  }

}
