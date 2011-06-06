#include "test_system_scheduler.hpp"

#include <ioa/system_scheduler.hpp>
#include <ioa/automaton_locker.hpp>

test_system_scheduler tss;

namespace ioa {
  
  void automaton_locker::lock_automaton (aid_t) { }
  void automaton_locker::unlock_automaton (aid_t) { }

  void system_scheduler::set_current_aid (aid_t, const automaton_interface&) { }
  void system_scheduler::clear_current_aid () { }

  void system_scheduler::init (const aid_t automaton) {
    tss.m_init_automaton = automaton;
  }
  
  void system_scheduler::create_key_exists (const aid_t automaton,
					    void* const key) {
    tss.m_create_key_exists_automaton = automaton;
    tss.m_create_key_exists_key = key;
  }

  void system_scheduler::instance_exists (const aid_t automaton,
					  void* const key) {
    tss.m_instance_exists_automaton = automaton;
    tss.m_instance_exists_key = key;
  }
  
  void system_scheduler::automaton_created (const aid_t automaton,
					    void* const key,
					    const aid_t child) {
    tss.m_automaton_created_automaton = automaton;
    tss.m_automaton_created_key = key;
    tss.m_automaton_created_child = child;
  }

  void system_scheduler::bind_key_exists (const aid_t automaton,
					  void* const key) {
    tss.m_bind_key_exists_automaton = automaton;
    tss.m_bind_key_exists_key = key;
  }
  
  void system_scheduler::output_automaton_dne (const aid_t automaton,
					       void* const key) {
    tss.m_output_automaton_dne_automaton = automaton;
    tss.m_output_automaton_dne_key = key;
  }
  
  void system_scheduler::input_automaton_dne (const aid_t automaton,
					      void* const key) {
    tss.m_input_automaton_dne_automaton = automaton;
    tss.m_input_automaton_dne_key = key;
 }
  
  void system_scheduler::binding_exists (const aid_t automaton,
					 void* const key) {
    tss.m_binding_exists_automaton = automaton;
    tss.m_binding_exists_key = key;
  }
  
  void system_scheduler::input_action_unavailable (const aid_t automaton,
						   void* const key) {
    tss.m_input_action_unavailable_automaton = automaton;
    tss.m_input_action_unavailable_key = key;
  }
  
  void system_scheduler::output_action_unavailable (const aid_t automaton,
						    void* const key) {
    tss.m_output_action_unavailable_automaton = automaton;
    tss.m_output_action_unavailable_key = key;
  }
  
  void system_scheduler::bound (const aid_t automaton,
				void* const key) {
    tss.m_bound_automaton = automaton;
    tss.m_bound_key = key;
  }

  void system_scheduler::bind_key_dne (const aid_t automaton,
				       void* const key) {
    tss.m_bind_key_dne_automaton = automaton;
    tss.m_bind_key_dne_key = key;
  }
  
  void system_scheduler::unbound (const aid_t automaton,
				  void* const key) {
    tss.m_unbound.insert (std::make_pair (automaton, key));
  }
  
  void system_scheduler::create_key_dne (const aid_t automaton,
					 void* const key) {
    tss.m_create_key_dne_automaton = automaton;
    tss.m_create_key_dne_key = key;
  }

  void system_scheduler::automaton_destroyed (const aid_t automaton,
					      void* const key) {
    tss.m_automaton_destroyed.insert (std::make_pair (automaton, key));
  }
  
  void system_scheduler::recipient_dne (const aid_t automaton,
					void* const key) {
    tss.m_recipient_dne_automaton = automaton;
    tss.m_recipient_dne_key = key;
  }

  void system_scheduler::event_delivered (const aid_t automaton,
					void* const key) {
    tss.m_event_delivered_automaton = automaton;
    tss.m_event_delivered_key = key;
  }

}
