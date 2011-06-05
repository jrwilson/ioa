#ifndef __automaton_interface_hpp__
#define __automaton_interface_hpp__

#include <ioa/action_wrapper.hpp>

// All of the automata should be classes.

namespace ioa {

  class automaton_interface {
  public:
    automaton_interface () :
      ACTION (automaton_interface, init),
      ACTION (automaton_interface, instance_exists),
      ACTION (automaton_interface, automaton_created),
      ACTION (automaton_interface, automaton_destroyed),
      ACTION (automaton_interface, output_automaton_dne),
      ACTION (automaton_interface, input_automaton_dne),
      ACTION (automaton_interface, binding_exists),
      ACTION (automaton_interface, output_action_unavailable),
      ACTION (automaton_interface, input_action_unavailable),
      ACTION (automaton_interface, bound),
      ACTION (automaton_interface, unbound),
      ACTION (automaton_interface, binding_dne),
      ACTION (automaton_interface, target_automaton_dne),
      ACTION (automaton_interface, destroyer_not_creator),
      ACTION (automaton_interface, recipient_dne)
    { }

    virtual ~automaton_interface () { }

    UP_INTERNAL (automaton_interface, init) { }
    SYSTEM_INPUT (automaton_interface, instance_exists) { }
    SYSTEM_INPUT (automaton_interface, automaton_created) { }
    SYSTEM_INPUT (automaton_interface, automaton_destroyed) { }
    SYSTEM_INPUT (automaton_interface, output_automaton_dne) { }
    SYSTEM_INPUT (automaton_interface, input_automaton_dne) { }
    SYSTEM_INPUT (automaton_interface, binding_exists) { }
    SYSTEM_INPUT (automaton_interface, output_action_unavailable) { }
    SYSTEM_INPUT (automaton_interface, input_action_unavailable) { }
    SYSTEM_INPUT (automaton_interface, bound) { }
    SYSTEM_INPUT (automaton_interface, unbound) { }
    SYSTEM_INPUT (automaton_interface, binding_dne) { }
    SYSTEM_INPUT (automaton_interface, target_automaton_dne) { }
    SYSTEM_INPUT (automaton_interface, destroyer_not_creator) { }
    SYSTEM_INPUT (automaton_interface, recipient_dne) { }
  };

}

#endif
