#include <ioa/automaton_interface.hpp>
#include <ioa/scheduler.hpp>

namespace ioa {

  void automaton_interface::schedule () {
    if (sys_create_precondition ()) {
      scheduler::schedule (&automaton_interface::sys_create);
    }

    if (sys_destroy_precondition ()) {
      scheduler::schedule (&automaton_interface::sys_destroy);
    }
  }

}
