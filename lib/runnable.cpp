#include <ioa/runnable.hpp>

namespace ioa {
  shared_mutex runnable_interface::m_mutex;
  size_t runnable_interface::m_count = 0;
}
