#include <ioa/scheduler.hpp>

namespace ioa {

  scheduler_interface* scheduler = 0;

  aid_t get_aid () {
    assert (scheduler != 0);
    return scheduler->get_current_aid ();
  }

  void schedule (automaton::sys_create_type automaton::*ptr) {
    assert (scheduler != 0);
    scheduler->schedule (ptr);
  }

  void schedule (automaton::sys_bind_type automaton::*ptr) {
    assert (scheduler != 0);
    scheduler->schedule (ptr);
  }

  void schedule (automaton::sys_unbind_type automaton::*ptr) {
    assert (scheduler != 0);
    scheduler->schedule (ptr);
  }

  void schedule (automaton::sys_destroy_type automaton::*ptr) {
    assert (scheduler != 0);
    scheduler->schedule (ptr);
  }
  
  void run (scheduler_interface& s,
	    const_shared_ptr<generator_interface> generator) {
    scheduler = &s;
    scheduler->run (generator);
  }

}
