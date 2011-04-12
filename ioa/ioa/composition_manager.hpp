#ifndef __composition_manager_hpp__
#define __composition_manager_hpp__

#include "action.hpp"

namespace ioa {

  class composition_manager {

  private:
    typedef std::list<macro_action_interface*> list_type;
    list_type m_macro_actions;

    struct output_equal {
      const local_action_interface& output_action;

      output_equal (const local_action_interface& output_action) :
	output_action (output_action)
      { }

      bool operator() (const macro_action_interface* ma) const {
	return ma->involves_output (output_action);
      }
    };

    struct input_equal {
      const input_action_interface& input_action;

      input_equal (const input_action_interface& input_action) :
	input_action (input_action)
      { }
      
      bool operator() (const macro_action_interface* ma) const {
	return ma->involves_input (input_action);
      }
    };

    template<class Instance, class Member>
    static const void* to_ptr (const automaton<Instance>* automaton,
			       const Member Instance::*member) {
      Instance* instance = automaton->get_typed_instance();
      const Member& ref = (*instance).*member;
      return &ref;
    }

    template <class OutputAction,
	      class InputAction,
	      class Scheduler>
    void compose (untyped /* */,
		  const OutputAction& output_action,
		  const InputAction& input_action,
		  Scheduler& scheduler) {

      untyped_macro_action<Scheduler>* ma;
      list_type::const_iterator pos = std::find_if (m_macro_actions.begin (),
						    m_macro_actions.end (),
						    output_equal (output_action));
      if (pos == m_macro_actions.end ()) {
	ma = new untyped_macro_action<Scheduler> (output_action, scheduler);
	m_macro_actions.push_back (ma);
      }
      else {
	// Necessary but safe.
	ma = static_cast<untyped_macro_action<Scheduler> * > (*pos);
      }

      ma->add_input (input_action);
    }

    template <class OutputAction,
	      class InputAction,
	      class Scheduler>
    void compose (typed /* */,
		  const OutputAction& output_action,
		  const InputAction& input_action,
		  Scheduler& scheduler) {

      typed_macro_action<typename OutputAction::type, Scheduler>* ma;
      list_type::const_iterator pos = std::find_if (m_macro_actions.begin (),
						    m_macro_actions.end (),
						    output_equal (output_action));
      if (pos == m_macro_actions.end ()) {
	ma = new typed_macro_action<typename OutputAction::type, Scheduler> (output_action, scheduler);
	m_macro_actions.push_back (ma);
      }
      else {
	// Necessary but safe.
	ma = static_cast<typed_macro_action<typename OutputAction::type, Scheduler>*> (*pos);
      }

      ma->add_input (input_action);
    }

  public:
    bool composed (const local_action_interface& output_action,
		   const input_action_interface& input_action) const {
      
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

    bool input_available (const input_action_interface& input_action) const {
      list_type::const_iterator pos = std::find_if (m_macro_actions.begin (),
						    m_macro_actions.end (),
						    input_equal (input_action));
      return pos == m_macro_actions.end ();
    }

    bool output_available (const local_action_interface& output_action,
			   const abstract_automaton* input_automaton) const {
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
	      class InputAction,
	      class Scheduler>
    void compose (const OutputAction& output_action,
		  const InputAction& input_action,
		  Scheduler& scheduler) {
      compose (typename OutputAction::type_status (), output_action, input_action, scheduler);
    }

    template<class OutputInstance, class OutputMember,
	     class InputInstance, class InputMember,
	     class T>
    void decompose (const abstract_automaton* owner,
		    automaton<OutputInstance>* output_automaton,
		    OutputMember OutputInstance::*output_member,
		    automaton<InputInstance>* input_automaton,
		    InputMember InputInstance::*input_member) {
      const void* output = to_ptr(output_automaton, output_member);
      const void* input = to_ptr(input_automaton, input_member);
      list_type::iterator pos = std::find_if(m_macro_actions.begin(),
						   m_macro_actions.end(),
						   output_equal(output_automaton, output));
      if(pos != m_macro_actions.end()) {
	(*pos)->decompose(input_automaton, input, owner);
	if((*pos)->empty()) {
	  delete (*pos);
	}
	m_macro_actions.erase(pos);
      }
    }

    template <class Scheduler>
    void execute (local_action_interface& output_action,
		  Scheduler& scheduler) {
      list_type::const_iterator pos = std::find_if (m_macro_actions.begin(),
						    m_macro_actions.end(),
						    output_equal (output_action));
      
      if (pos == m_macro_actions.end ()) {
	// Not composed.
	
	// Lock.
	abstract_automaton::lock_type lock (*(output_action.get_automaton ()));
	// Set the current automaton.
	scheduler.set_current_automaton (output_action.get_automaton ());
	// Execute.
	output_action.execute ();
	scheduler.set_current_automaton (0);
      }
      else {
	// Composed.
	(*pos)->execute ();
      }
    }

  };

}

#endif