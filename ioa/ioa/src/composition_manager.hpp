#ifndef __composition_manager_hpp__
#define __composition_manager_hpp__

#include "action.hpp"

namespace ioa {

  class composition_manager {
  private:
    typedef std::list<abstract_macro_action*> list_type;
    list_type m_macro_actions;

    struct output_equal {
      const abstract_automaton* output_automaton;
      const void* output;

      output_equal(const abstract_automaton* output_automaton,
		   const void* output)
	: output_automaton(output_automaton),
	  output(output) { }

      bool operator() (const abstract_macro_action* ma) const {
	return ma->is_output(output_automaton, output);
      }
    };

    struct input_equal {
      const abstract_automaton* input_automaton;
      const void* input;

      input_equal(const abstract_automaton* input_automaton,
		   const void* input)
	: input_automaton(input_automaton),
	  input(input) { }

      bool operator() (const abstract_macro_action* ma) const {
	return ma->is_input(input_automaton, input);
      }
    };

    template<class Instance, class Member>
    static const void* to_ptr(const automaton<Instance>* automaton,
			      const Member Instance::*member) {
      Instance* instance = automaton->get_typed_instance();
      const Member& ref = (*instance).*member;
      return &ref;
    }

  public:
    template<class OutputInstance, class OutputMember,
	     class InputInstance, class InputMember,
	     class T>
    bool composed(const abstract_automaton* owner,
		  const automaton<OutputInstance>* output_automaton,
		  const OutputMember OutputInstance::*output_member,
		  const automaton<InputInstance>* input_automaton,
		  const InputMember InputInstance::*input_member) const {
      const void* output = to_ptr(output_automaton, output_member);
      const void* input = to_ptr(input_automaton, input_member);

      list_type::const_iterator pos = std::find_if(m_macro_actions.begin(),
						   m_macro_actions.end(),
						   output_equal(output_automaton, output));
      if(pos == m_macro_actions.end()) {
	return false;
      }
      else {
	return (*pos)->is_input(input_automaton, input, owner);
      }
    }

    template<class InputInstance, class InputMember>
    bool input_available(const automaton<InputInstance>* input_automaton,
			 const InputMember InputInstance::*input_member) const {
      const void* input = to_ptr(input_automaton, input_member);
      list_type::const_iterator pos = std::find_if(m_macro_actions.begin(),
						   m_macro_actions.end(),
						   input_equal(input_automaton, input));
      return pos == m_macro_actions.end();
    }

    template<class OutputInstance, class OutputMember>
    bool output_available(const automaton<OutputInstance>* output_automaton,
			  const OutputMember OutputInstance::*output_member,
			  const abstract_automaton* input_automaton) const {
      const void* output = to_ptr(output_automaton, output_member);

      if(output_automaton == input_automaton) {
      	// Can't compose with self.
      	return false;
      }

      list_type::const_iterator pos = std::find_if(m_macro_actions.begin(),
						   m_macro_actions.end(),
						   output_equal(output_automaton, output));

      if(pos == m_macro_actions.end()) {
	return true;
      }
      else {
	return !(*pos)->is_input(input_automaton);
      }
    }

    template<class Callback,
	     class OutputInstance, class OutputMember,
	     class InputInstance, class InputMember,
	     class T>
    void compose(Callback& callback,
		 const abstract_automaton* owner,
		 automaton<OutputInstance>* output_automaton,
		 OutputMember OutputInstance::*output_member,
		 automaton<InputInstance>* input_automaton,
		 InputMember InputInstance::*input_member) {

      macro_action<T>* ma;
      const void* output = to_ptr(output_automaton, output_member);
      list_type::const_iterator pos = std::find_if(m_macro_actions.begin(),
						   m_macro_actions.end(),
						   output_equal(output_automaton, output));
      if(pos == m_macro_actions.end()) {
	output_action<OutputInstance, OutputMember, T>* oa = new output_action<OutputInstance, OutputMember, T>(output_automaton, output_member);
	ma = new macro_action<T>(oa);
	m_macro_actions.push_back(ma);
      }
      else {
	// Necessary but safe.
	ma = static_cast<macro_action<T>*>(*pos);
      }

      input_action<InputInstance, InputMember, T, Callback>* ia = new input_action<InputInstance, InputMember, T, Callback>(input_automaton, input_member, owner, callback);
      ma->add_input(ia);
    }

    template<class OutputInstance, class OutputMember,
	     class InputInstance, class InputMember,
	     class T>
    void decompose(const abstract_automaton* owner,
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

    template<class OutputInstance, class OutputMember>
    void execute(automaton<OutputInstance>* output_automaton,
		 OutputMember OutputInstance::*output_member) const {

      const void* output = to_ptr(output_automaton, output_member);

      list_type::const_iterator pos = std::find_if(m_macro_actions.begin(),
						   m_macro_actions.end(),
						   output_equal(output_automaton, output));
      
      if(pos == m_macro_actions.end()) {
	// Not composed.

	// Lock and execute.
	abstract_automaton::lock_type lock(*output_automaton);
	OutputInstance* instance = output_automaton->get_typed_instance();
	OutputMember& ref = (*instance).*output_member;
	ref();
      }
      else {
	// Composed.
	(*(*pos))();
      }
    }

  };

}

#endif
