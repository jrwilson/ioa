#include "runnable.hpp"

boost::shared_mutex ioa::runnable_interface::m_mutex;
size_t ioa::runnable_interface::m_count = 0;
