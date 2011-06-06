#ifndef __test_system_scheduler_hpp__
#define __test_system_scheduler_hpp__

#include <ioa/aid.hpp>
#include <ioa/bid.hpp>

struct test_system_scheduler
{
  ioa::aid_t m_init_automaton;

  ioa::aid_t m_create_key_exists_automaton;
  void* m_create_key_exists_key;

  ioa::aid_t m_instance_exists_automaton;
  void* m_instance_exists_aux;

  ioa::aid_t m_automaton_created_automaton;
  void* m_automaton_created_key;
  ioa::aid_t m_automaton_created_child;

  ioa::aid_t m_output_automaton_dne_automaton;
  void* m_output_automaton_dne_aux;

  ioa::aid_t m_input_automaton_dne_automaton;
  void* m_input_automaton_dne_aux;

  ioa::aid_t m_binding_exists_automaton;
  void* m_binding_exists_aux;

  ioa::aid_t m_input_action_unavailable_automaton;
  void* m_input_action_unavailable_aux;

  ioa::aid_t m_output_action_unavailable_automaton;
  void* m_output_action_unavailable_aux;

  ioa::aid_t m_bound_automaton;
  void* m_bound_aux;
  ioa::bid_t m_bound_bid;

  ioa::aid_t m_binding_dne_automaton;
  void* m_binding_dne_aux;

  ioa::aid_t m_unbound_automaton;
  ioa::bid_t m_unbound_bid;

  ioa::aid_t m_target_automaton_dne_automaton;
  void* m_target_automaton_dne_aux;

  ioa::aid_t m_destroyer_not_creator_automaton;
  void* m_destroyer_not_creator_aux;

  ioa::aid_t m_automaton_destroyed_automaton;
  ioa::aid_t m_automaton_destroyed_child;

  ioa::aid_t m_recipient_dne_automaton;
  void* m_recipient_dne_aux;

  void reset () {

  }
};

extern test_system_scheduler tss;

#endif
