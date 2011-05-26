#ifndef __ioa_hpp__
#define __ioa_hpp__

#include "scheduler.hpp"
#include "simple_scheduler.hpp"
#include "dispatching_automaton.hpp"
#include "instance_generator.hpp"
#include "action_wrapper.hpp"

namespace ioa {

  simple_scheduler ss;
  scheduler_wrapper<simple_scheduler> scheduler (ss);
}

#include "automaton_helper.hpp"
#include "bind_helper.hpp"

#endif
