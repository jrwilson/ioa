#include "runnable.hpp"

boost::shared_mutex ioa::runnable::m_mutex;
size_t ioa::runnable::m_count = 0;
