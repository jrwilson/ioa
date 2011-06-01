#ifndef __scheduler_hpp__
#define __scheduler_hpp__

#include "scheduler_wrapper.hpp"
#include "simple_scheduler.hpp"

namespace ioa {
  simple_scheduler ss;
  scheduler_wrapper<simple_scheduler> scheduler (ss);
}

#endif
