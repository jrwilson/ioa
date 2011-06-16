#include "test_system_scheduler.hpp"

#include <ioa/executor_interface.hpp>

test_system_scheduler::test_system_scheduler () {
  reset ();
}

void test_system_scheduler::reset () {
  m_create_key_exists_automaton = -1;
  m_create_key_exists_key = 0;
    
  m_instance_exists_automaton = -1;
  m_instance_exists_key = 0;
    
  m_automaton_created_automaton = -1;
  m_automaton_created_key = 0;
  m_automaton_created_child = -1;
    
  m_bind_key_exists_automaton = -1;
  m_bind_key_exists_key = 0;
    
  m_output_automaton_dne_automaton = -1;
  m_output_automaton_dne_key = 0;
    
  m_input_automaton_dne_automaton = -1;
  m_input_automaton_dne_key = 0;
    
  m_binding_exists_automaton = -1;
  m_binding_exists_key = 0;
    
  m_input_action_unavailable_automaton = -1;
  m_input_action_unavailable_key = 0;
    
  m_output_action_unavailable_automaton = -1;
  m_output_action_unavailable_key = 0;
    
  m_bound_automaton = -1;
  m_bound_key = 0;

  m_output_bound_aid = -1;
  m_output_bound_member_ptr = 0;
  m_output_bound_pid = 0;

  m_input_bound_aid = -1;
  m_input_bound_member_ptr = 0;
  m_input_bound_pid = 0;
    
  m_bind_key_dne_automaton = -1;
  m_bind_key_dne_key = 0;
    
  m_unbound.clear ();

  m_output_unbound.clear ();
    
  m_input_unbound.clear ();
    
  m_create_key_dne_automaton = -1;
  m_create_key_dne_key = 0;
    
  m_automaton_destroyed.clear ();

  m_recipient_dne_automaton = -1;
  m_recipient_dne_key = 0;

  m_event_delivered_automaton = -1;
  m_event_delivered_key = 0;
}

void test_system_scheduler::set_current_aid (ioa::aid_t) { }
void test_system_scheduler::clear_current_aid () { }

void test_system_scheduler::create (const ioa::aid_t automaton,
				    ioa::shared_ptr<ioa::generator_interface> generator,
				    void* const key) {
  assert (false);
}

void test_system_scheduler::bind (const ioa::aid_t automaton,
				  ioa::shared_ptr<ioa::bind_executor_interface> exec,
				  void* const key) {
  assert (false);
}

void test_system_scheduler::unbind (const ioa::aid_t automaton,
				   void* const key) {
  assert (false);
}

void test_system_scheduler::destroy (const ioa::aid_t automaton,
				     void* const key) {
  assert (false);
}
  
void test_system_scheduler::create_key_exists (const ioa::aid_t automaton,
					       void* const key) {
  m_create_key_exists_automaton = automaton;
  m_create_key_exists_key = key;
}

void test_system_scheduler::instance_exists (const ioa::aid_t automaton,
					     void* const key) {
  m_instance_exists_automaton = automaton;
  m_instance_exists_key = key;
}
  
void test_system_scheduler::automaton_created (const ioa::aid_t automaton,
					       void* const key,
					       const ioa::aid_t child) {
  m_automaton_created_automaton = automaton;
  m_automaton_created_key = key;
  m_automaton_created_child = child;
}

void test_system_scheduler::bind_key_exists (const ioa::aid_t automaton,
					     void* const key) {
  m_bind_key_exists_automaton = automaton;
  m_bind_key_exists_key = key;
}
  
void test_system_scheduler::output_automaton_dne (const ioa::aid_t automaton,
						  void* const key) {
  m_output_automaton_dne_automaton = automaton;
  m_output_automaton_dne_key = key;
}
  
void test_system_scheduler::input_automaton_dne (const ioa::aid_t automaton,
						 void* const key) {
  m_input_automaton_dne_automaton = automaton;
  m_input_automaton_dne_key = key;
}
  
void test_system_scheduler::binding_exists (const ioa::aid_t automaton,
					    void* const key) {
  m_binding_exists_automaton = automaton;
  m_binding_exists_key = key;
}
  
void test_system_scheduler::input_action_unavailable (const ioa::aid_t automaton,
						      void* const key) {
  m_input_action_unavailable_automaton = automaton;
  m_input_action_unavailable_key = key;
}
  
void test_system_scheduler::output_action_unavailable (const ioa::aid_t automaton,
						       void* const key) {
  m_output_action_unavailable_automaton = automaton;
  m_output_action_unavailable_key = key;
}
  
void test_system_scheduler::bound (const ioa::aid_t automaton,
				   void* const key) {
  m_bound_automaton = automaton;
  m_bound_key = key;
}

void test_system_scheduler::output_bound (const ioa::output_executor_interface& exec) {
  const ioa::action_interface& ac = exec.get_action ();
  m_output_bound_aid = ac.get_aid ();
  m_output_bound_member_ptr = ac.get_member_ptr ();
  m_output_bound_pid = ac.get_pid ();
}

void test_system_scheduler::input_bound (const ioa::input_executor_interface& exec) {
  const ioa::action_interface& ac = exec.get_action ();
  m_input_bound_aid = ac.get_aid ();
  m_input_bound_member_ptr = ac.get_member_ptr ();
  m_input_bound_pid = ac.get_pid ();
}

void test_system_scheduler::bind_key_dne (const ioa::aid_t automaton,
					  void* const key) {
  m_bind_key_dne_automaton = automaton;
  m_bind_key_dne_key = key;
}
  
void test_system_scheduler::unbound (const ioa::aid_t automaton,
				     void* const key) {
  m_unbound.insert (std::make_pair (automaton, key));
}

void test_system_scheduler::output_unbound (const ioa::output_executor_interface& exec) {
  const ioa::action_interface& ac = exec.get_action ();
  m_output_unbound.insert (unbound_record (ac));
}

void test_system_scheduler::input_unbound (const ioa::input_executor_interface& exec) {
  const ioa::action_interface& ac = exec.get_action ();
  m_input_unbound.insert (unbound_record (ac));
}
  
void test_system_scheduler::create_key_dne (const ioa::aid_t automaton,
					    void* const key) {
  m_create_key_dne_automaton = automaton;
  m_create_key_dne_key = key;
}

void test_system_scheduler::automaton_destroyed (const ioa::aid_t automaton,
						 void* const key) {
  m_automaton_destroyed.insert (std::make_pair (automaton, key));
}
  
void test_system_scheduler::recipient_dne (const ioa::aid_t automaton,
					   void* const key) {
  m_recipient_dne_automaton = automaton;
  m_recipient_dne_key = key;
}

void test_system_scheduler::event_delivered (const ioa::aid_t automaton,
					     void* const key) {
  m_event_delivered_automaton = automaton;
  m_event_delivered_key = key;
}
