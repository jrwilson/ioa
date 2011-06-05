#ifndef __automaton_interface_hpp__
#define __automaton_interface_hpp__

// All of the automata should be classes.

namespace ioa {

  class automaton_interface {
  public:
    virtual ~automaton_interface () { }

    int init;
    int instance_exists;
    int automaton_created;
    int automaton_destroyed;
    int output_automaton_dne;
    int input_automaton_dne;
    int binding_exists;
    int output_action_unavailable;
    int input_action_unavailable;
    int bound;
    int unbound;
    int binding_dne;
    int target_automaton_dne;
    int destroyer_not_creator;
    int recipient_dne;
  };

}

#endif
