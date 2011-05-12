#ifndef __automaton2_hpp__
#define __automaton2_hpp__

class automaton2 :
  public ioa::automaton_interface
{
private:
  void init () { }
  void instance_exists (const void*) { }
  void automaton_created (const ioa::generic_automaton_handle&) { }
  void parameter_exists (const ioa::generic_parameter_handle&) { }
  void parameter_declared (const ioa::generic_parameter_handle&) { }
  void bind_output_automaton_dne () { }
  void bind_input_automaton_dne () { }
  void bind_output_parameter_dne () { }
  void bind_input_parameter_dne () { }
  void binding_exists () { }
  void input_action_unavailable () { }
  void output_action_unavailable () { }
  void bound () { }
  void unbind_output_automaton_dne () { }
  void unbind_input_automaton_dne () { }
  void unbind_output_parameter_dne () { }
  void unbind_input_parameter_dne () { }
  void binding_dne () { }
  void unbound () { }
  void parameter_dne (const ioa::generic_parameter_handle&) { }
  void parameter_rescinded (const ioa::generic_parameter_handle&) { }
  void target_automaton_dne (const ioa::generic_automaton_handle&) { }
  void destroyer_not_creator (const ioa::generic_automaton_handle&) { }
  void automaton_destroyed (const ioa::generic_automaton_handle&) { }

  bool output_ () {
    return false;
  }

  void input_ () {
  }

public:
  ioa::void_output_wrapper<automaton2, &automaton2::output_> output;
  ioa::void_input_wrapper<automaton2, &automaton2::input_> input;
  ioa::void_input_wrapper<automaton2, &automaton2::input_> input2;

  automaton2 () :
    output (*this),
    input (*this),
    input2 (*this)
  { }
};

#endif
