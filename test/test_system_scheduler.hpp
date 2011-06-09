#ifndef __test_system_scheduler_hpp__
#define __test_system_scheduler_hpp__

#include <ioa/action.hpp>

#include <set>

struct unbound_record
{
  ioa::aid_t m_aid;
  const void* m_member_ptr;
  size_t m_pid;

  unbound_record (const ioa::action_interface& ac) :
    m_aid (ac.get_aid ()),
    m_member_ptr (ac.get_member_ptr ()),
    m_pid (ac.get_pid ())
  { }

  bool operator< (const unbound_record& other) const {
    if (m_aid != other.m_aid) {
      return m_aid < other.m_aid;
    }
    if (m_member_ptr != other.m_member_ptr) {
      return m_member_ptr < other.m_member_ptr;
    }
    return m_pid < other.m_pid;
  }
};

struct test_system_scheduler
{
  ioa::aid_t m_create_key_exists_automaton;
  void* m_create_key_exists_key;

  ioa::aid_t m_instance_exists_automaton;
  void* m_instance_exists_key;

  ioa::aid_t m_automaton_created_automaton;
  void* m_automaton_created_key;
  ioa::aid_t m_automaton_created_child;

  ioa::aid_t m_bind_key_exists_automaton;
  void* m_bind_key_exists_key;

  ioa::aid_t m_output_automaton_dne_automaton;
  void* m_output_automaton_dne_key;

  ioa::aid_t m_input_automaton_dne_automaton;
  void* m_input_automaton_dne_key;

  ioa::aid_t m_binding_exists_automaton;
  void* m_binding_exists_key;

  ioa::aid_t m_input_action_unavailable_automaton;
  void* m_input_action_unavailable_key;

  ioa::aid_t m_output_action_unavailable_automaton;
  void* m_output_action_unavailable_key;

  ioa::aid_t m_bound_automaton;
  void* m_bound_key;

  ioa::aid_t m_output_bound_aid;
  const void* m_output_bound_member_ptr;
  size_t m_output_bound_pid;

  ioa::aid_t m_input_bound_aid;
  const void* m_input_bound_member_ptr;
  size_t m_input_bound_pid;

  ioa::aid_t m_bind_key_dne_automaton;
  void* m_bind_key_dne_key;

  std::set<std::pair<ioa::aid_t, void*> > m_unbound;

  std::set<unbound_record> m_output_unbound;

  std::set<unbound_record> m_input_unbound;

  ioa::aid_t m_create_key_dne_automaton;
  void* m_create_key_dne_key;

  std::set<std::pair<ioa::aid_t, void*> > m_automaton_destroyed;

  ioa::aid_t m_recipient_dne_automaton;
  void* m_recipient_dne_key;

  ioa::aid_t m_event_delivered_automaton;
  void* m_event_delivered_key;

  void reset () {
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
};

extern test_system_scheduler tss;

#endif
