#include <ioa/scheduler.hpp>

namespace ioa {

  scheduler_interface* scheduler = 0;

  aid_t get_current_aid () {
    assert (scheduler != 0);
    return scheduler->get_current_aid ();
  }

  void schedule (automaton_interface::sys_create_type automaton_interface::*ptr) {
    assert (scheduler != 0);
    scheduler->schedule (ptr);
  }

  void schedule (automaton_interface::sys_bind_type automaton_interface::*ptr) {
    assert (scheduler != 0);
    scheduler->schedule (ptr);
  }

  void schedule (automaton_interface::sys_unbind_type automaton_interface::*ptr) {
    assert (scheduler != 0);
    scheduler->schedule (ptr);
  }

  void schedule (automaton_interface::sys_destroy_type automaton_interface::*ptr) {
    assert (scheduler != 0);
    scheduler->schedule (ptr);
  }
  
  void run (scheduler_interface& s,
	    shared_ptr<generator_interface> generator) {
    scheduler = &s;
    scheduler->run (generator);
  }

}
