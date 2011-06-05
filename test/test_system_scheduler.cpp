#include <ioa/system_scheduler.hpp>
#include <ioa/automaton_locker.hpp>

namespace ioa {
  
  void automaton_locker::lock_automaton (aid_t) { }
  void automaton_locker::unlock_automaton (aid_t) { }

  void system_scheduler::set_current_aid (aid_t, const automaton_interface&) { }
  void system_scheduler::clear_current_aid () { }

  void system_scheduler::init (const aid_t automaton) { }
  
  void system_scheduler::instance_exists (const aid_t automaton,
					  void* aux) { }
  
  void system_scheduler::automaton_created (const aid_t automaton,
					    void* aux,
					    const aid_t child) { }
  
  void system_scheduler::output_automaton_dne (const aid_t binder,
					       void* aux) { }
  
  void system_scheduler::input_automaton_dne (const aid_t binder,
					      void* aux) { }
  
  void system_scheduler::binding_exists (const aid_t binder,
					 void* aux) { }
  
  void system_scheduler::input_action_unavailable (const aid_t binder,
						   void* aux) { }
  
  void system_scheduler::output_action_unavailable (const aid_t binder,
						    void* aux) { }
  
  void system_scheduler::bound (const aid_t binder,
				void* aux,
				const bid_t bid) { }
  
  void system_scheduler::binding_dne (const aid_t binder,
				      void* aux) { }
  
  void system_scheduler::unbound (const aid_t binder,
				  const bid_t bid) { }
  
  void system_scheduler::target_automaton_dne (const aid_t automaton,
					       void* aux) { }
  
  void system_scheduler::destroyer_not_creator (const aid_t automaton,
						void* aux) { }
  
  void system_scheduler::automaton_destroyed (const aid_t parent,
					      const aid_t child) { }
  
  void system_scheduler::recipient_dne (const aid_t from,
					void* aux) { }
  
}
