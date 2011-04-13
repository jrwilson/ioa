#ifndef __composition_manager_hpp__
#define __composition_manager_hpp__

#include "action.hpp"

namespace ioa {

  class composition_manager {

  private:
    typedef std::list<macro_action*> list_type;
    list_type m_macro_actions;

    struct output_equal {
      const output_action& output;

      output_equal (const output_action& output) :
	output (output)
      { }

      bool operator() (const macro_action* ma) const {
	return ma->involves_output (output);
      }
    };

    struct input_equal {
      const input_action& input;

      input_equal (const input_action& input) :
	input (input)
      { }
      
      bool operator() (const macro_action* ma) const {
	return ma->involves_input (input);
      }
    };

    template <class OutputAction,
	      class InputAction>
    void compose (untyped /* */,
		  const OutputAction& output_action,
		  const InputAction& input_action) {

      untyped_macro_action* ma;
      list_type::const_iterator pos = std::find_if (m_macro_actions.begin (),
						    m_macro_actions.end (),
						    output_equal (output_action));
      if (pos == m_macro_actions.end ()) {
	ma = new untyped_macro_action (output_action);
	m_macro_actions.push_back (ma);
      }
      else {
	// Necessary but safe.
	ma = static_cast<untyped_macro_action*> (*pos);
      }

      ma->add_input (input_action);
    }

    template <class OutputAction,
	      class InputAction>
    void compose (typed /* */,
		  const OutputAction& output_action,
		  const InputAction& input_action) {

      typed_macro_action<typename OutputAction::value_type>* ma;
      list_type::const_iterator pos = std::find_if (m_macro_actions.begin (),
						    m_macro_actions.end (),
						    output_equal (output_action));
      if (pos == m_macro_actions.end ()) {
	ma = new typed_macro_action<typename OutputAction::value_type> (output_action);
	m_macro_actions.push_back (ma);
      }
      else {
	// Necessary but safe.
	ma = static_cast<typed_macro_action<typename OutputAction::value_type>*> (*pos);
      }

      ma->add_input (input_action);
    }

    void execute_local (local_action& local_action) {
      scheduler_interface& scheduler = get_scheduler ();
      
      // Lock.
      automaton::lock_type lock (*(local_action.get_automaton ()));
      // Set the current automaton.
      scheduler.set_current_automaton (local_action.get_automaton ());
      // Execute.
      local_action.execute ();
      scheduler.set_current_automaton (0);
    }

  public:
    bool composed (const output_action& output_action,
		   const input_action& input_action) const {
      
      list_type::const_iterator pos = std::find_if (m_macro_actions.begin(),
						    m_macro_actions.end(),
						    output_equal (output_action));
      if(pos == m_macro_actions.end ()) {
	return false;
      }
      else {
	return (*pos)->involves_input_check_owner (input_action);
      }
    }

    bool input_available (const input_action& input_action) const {
      list_type::const_iterator pos = std::find_if (m_macro_actions.begin (),
						    m_macro_actions.end (),
						    input_equal (input_action));
      return pos == m_macro_actions.end ();
    }

    bool output_available (const output_action& output_action,
			   const automaton* input_automaton) const {
      if (output_action.get_automaton () == input_automaton) {
	// No self compositions!!
	return false;
      }

      list_type::const_iterator pos = std::find_if (m_macro_actions.begin (),
						    m_macro_actions.end (),
						    output_equal (output_action));
      
      if (pos == m_macro_actions.end ()) {
	return true;
      }
      else {
	return !(*pos)->involves_automaton (input_automaton);
      }
    }

    template <class OutputAction,
	      class InputAction>
    void compose (const OutputAction& output_action,
		  const InputAction& input_action) {
      compose (typename OutputAction::value_status (), output_action, input_action);
    }

    template<class OutputAction,
	     class InputAction>
    void decompose (const OutputAction& output_action,
		    const InputAction& input_action) {
      list_type::iterator pos = std::find_if (m_macro_actions.begin (),
					      m_macro_actions.end (),
					      output_equal (output_action));
      if (pos != m_macro_actions.end ()) {
	(*pos)->decompose (input_action);
	if ((*pos)->empty ()) {
	  delete (*pos);
	}
	m_macro_actions.erase (pos);
      }
    }
 
    void decompose (const automaton* automaton,
		    const void* parameter) {
      list_type::iterator pos = m_macro_actions.begin ();
      while (pos != m_macro_actions.end ()) {
	(*pos)->decompose (automaton, parameter);
	if ((*pos)->empty ()) {
	  delete (*pos);
	  pos = m_macro_actions.erase (pos);
	}
	else {
	  ++pos;
	}
      }
    }

    void decompose (const automaton* automaton) {
      list_type::iterator pos = m_macro_actions.begin ();
      while (pos != m_macro_actions.end ()) {
	(*pos)->decompose (automaton);
	if ((*pos)->empty ()) {
	  delete (*pos);
	  pos = m_macro_actions.erase (pos);
	}
	else {
	  ++pos;
	}
      }
    }

    void execute (output_action& output_action) {
      list_type::const_iterator pos = std::find_if (m_macro_actions.begin(),
						    m_macro_actions.end(),
						    output_equal (output_action));
      if (pos == m_macro_actions.end ()) {
	// Not composed.
	execute_local (output_action);
      }
      else {
	// Composed.
	(*pos)->execute ();
      }
    }

    void execute (internal_action& internal_action) {
      execute_local (internal_action);
    }

  };
  
}

#endif
