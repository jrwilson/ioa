#ifndef __test_system_scheduler_hpp__
#define __test_system_scheduler_hpp__

#include <ioa/system_scheduler.hpp>
#include <utility>

namespace ioa {

  template <>
  void system_scheduler::schedule (const aid_t,
				   int automaton_interface::*) { }

  template <>
  void system_scheduler::schedule (const aid_t,
  				   int automaton_interface::*,
  				   const std::pair<void*, int>&) { }
  
  template <>
  void system_scheduler::schedule (const aid_t,
  				   int automaton_interface::*,
  				   void* const&) { }

  template <>
  void system_scheduler::schedule (const aid_t,
				   int automaton_interface::*,
				   const aid_t&) { }

}

#endif
