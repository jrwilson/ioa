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
    
    // Bind/Unbind

    template <class D>
    void output_automaton_dne (D& d) {
      d.output_automaton_dne ();
    }

    template <class D>
    void input_automaton_dne (D& d) {
      d.input_automaton_dne ();
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
    void bound (const bid_t bid,
		D& d) {
      d.bound (bid);
    }

    template <class D>
    void unbound (D& d) {
      d.unbound ();
    }

    template <class D>
    void binding_dne (D& d) {
      d.binding_dne ();
    }

  };

}

#endif
