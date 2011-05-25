#ifndef __dispatching_automaton_hpp__
#define __dispatching_automaton_hpp__

namespace ioa {

  class dispatching_automaton :
    public automaton_interface
  {
  public:
    
    // Create/Destroy
    
    template <class I, class D>
    void instance_exists (const I* i, D& d) {
      d.instance_exists (i);
    }
    
    template <class I, class D>
    void automaton_created (const automaton_handle<I>& automaton, D& d) {
      d.automaton_created (automaton);
    }
    
    template <class D>
    void automaton_destroyed (D& d) {
      d.automaton_destroyed ();
    }
    
    template <class D>
    void target_automaton_dne (D& d) {
      d.target_automaton_dne ();
    }
    
    template <class D>
    void destroyer_not_creator (D& d) {
      d.destroyer_not_creator ();
    }
    
    // Declare/Rescind
    
    template <class D>
    void parameter_exists (D& d) {
      d.parameter_exists ();
    }
    
    template <class P, class D>
    void parameter_declared (const parameter_handle<P>& parameter, D& d) {
      d.parameter_declared (parameter);
    }
    
    template <class D>
    void parameter_rescinded (D& d) {
      d.parameter_rescinded ();
    }
    
    template <class D>
    void parameter_dne (D& d) {
      d.parameter_dne ();
    }

    // Bind/Unbind

    template <class D>
    void bind_output_automaton_dne (D& d) {
      d.bind_output_automaton_dne ();
    }

    template <class D>
    void bind_input_automaton_dne (D& d) {
      d.bind_input_automaton_dne ();
    }

    template <class D>
    void bind_output_parameter_dne (D& d) {
      d.bind_output_parameter_dne ();
    }

    template <class D>
    void bind_input_parameter_dne (D& d) {
      d.bind_input_parameter_dne ();
    }

    template <class D>
    void binding_exists (D& d) {
      d.binding_exists ();
    }

    template <class D>
    void input_action_unavailable (D& d) {
      d.input_action_unavailable ();
    }

    template <class D>
    void output_action_unavailable (D& d) {
      d.output_action_unavailable ();
    }

    template <class D>
    void bound (D& d) {
      d.bound ();
    }

    template <class D>
    void unbound (D& d) {
      d.unbound ();
    }

    template <class D>
    void unbind_output_automaton_dne (D& d) {
      d.unbind_output_automaton_dne ();
    }

    template <class D>
    void unbind_input_automaton_dne (D& d) {
      d.unbind_input_automaton_dne ();
    }

    template <class D>
    void unbind_output_parameter_dne (D& d) {
      d.unbind_output_parameter_dne ();
    }

    template <class D>
    void unbind_input_parameter_dne (D& d) {
      d.unbind_input_parameter_dne ();
    }

    template <class D>
    void binding_dne (D& d) {
      d.binding_dne ();
    }

  };

}

#endif