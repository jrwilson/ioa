#ifndef __ioa_hpp__
#define __ioa_hpp__

#include <memory>
#include <set>
#include <boost/foreach.hpp>
#include <boost/thread.hpp>

#include "automaton.hpp"
#include "composition_manager.hpp"

namespace ioa {

  struct root_automaton { };

  class system : public boost::shared_mutex {
  private:
    std::set<abstract_automaton*> m_automata;
    composition_manager m_composition_manager;

    struct automaton_instance_equal {
      void* m_instance;
      automaton_instance_equal(void* instance)
	: m_instance(instance) { }

      bool operator()(const abstract_automaton* aa) const {
	return m_instance == aa->get_instance();
      }
    };

  public:
    const automaton_handle<root_automaton> root_handle;

    system()
      : root_handle(new automaton<root_automaton>()) { }

    ~system() {
      BOOST_FOREACH(abstract_automaton* aa, m_automata) {
	delete aa;
      }
    }

    // Modifying methods.
    template<class Callback, class Parent, class Child>
    void create(Callback& callback,
		const automaton_handle<Parent>& parent,
		Child* child) {
      boost::unique_lock<boost::shared_mutex> lock(*this);
      if (parent.valid()) {
	std::set<abstract_automaton*>::const_iterator pos =
	  std::find_if (m_automata.begin(),
			m_automata.end(),
			automaton_instance_equal(child));
	if(pos == m_automata.end()) {
	  automaton<Child>* c = new automaton<Child>(child);
	  m_automata.insert(c);
	  const automaton_handle<Child> h(c);
	  callback.child_created(parent, h);
	}
	else {
	  callback.automaton_exists(parent, child);
	}
      }
      else {
	delete child;
      }
    }

    template<class Callback, class Instance>
    void declare(Callback& callback,
    		 const automaton_handle<Instance>& handle,
    		 const void* parameter) {
      boost::unique_lock<boost::shared_mutex> lock(*this);
      if (handle.valid()) {
	automaton<Instance>* automaton = handle.get_automaton();
	if(!automaton->is_declared(parameter)) {
	  automaton->declare(parameter);
	  callback.parameter_declared(handle, parameter);
	}
	else {
	  callback.parameter_exists(handle, parameter);
	}
      }
    }

    template<class Callback, class Instance,
	     class OutputInstance, class OutputMember,
	     class InputInstance, class InputMember,
	     class T>
    void compose(Callback& callback,
		 const automaton_handle<Instance>& handle,
		 const automaton_handle<OutputInstance>& output_handle,
		 OutputMember OutputInstance::*output_member,
		 const automaton_handle<InputInstance>& input_handle,
		 InputMember InputInstance::*input_member) {
      // Lock the system.
      boost::unique_lock<boost::shared_mutex> lock(*this);
      if(handle.valid()) {
      	if(!output_handle.valid()) {
      	  callback.output_invalid();
      	}
      	else if(!input_handle.valid()) {
      	  callback.input_invalid();
      	}
      	else {
	  automaton<Instance>* owner = handle.get_automaton();
      	  automaton<OutputInstance>* output_automaton = output_handle.get_automaton();
      	  automaton<InputInstance>* input_automaton = input_handle.get_automaton();
      	  if(m_composition_manager.composed<OutputInstance, OutputMember, InputInstance, InputMember, T>(owner, output_automaton, output_member, input_automaton, input_member)) {
      	    callback.composition_exists();
      	  }
      	  else if(!m_composition_manager.input_available(input_automaton, input_member)) {
      	    callback.input_unavailable();
      	  }
      	  else if(!m_composition_manager.output_available(output_automaton, output_member, input_automaton)) {
      	    callback.output_unavailable();
      	  }
      	  else {
      	    m_composition_manager.compose<Callback, OutputInstance, OutputMember, InputInstance, InputMember, T>(callback, owner, output_automaton, output_member, input_automaton, input_member);
      	    callback.composed();
      	  }
      	}
      }
    }

    template<class Callback, class Instance,
	     class OutputInstance, class OutputMember,
	     class InputInstance, class InputMember>
    void decompose(Callback& callback,
		   const automaton_handle<Instance>& handle,
		   const automaton_handle<OutputInstance>& output_handle,
		   OutputMember OutputInstance::*output_member,
		   const automaton_handle<InputInstance>& input_handle,
		   const InputMember InputInstance::*input_member) {
      // Lock the system.
      boost::unique_lock<boost::shared_mutex> lock(*this);
      if(handle.valid()) {
	automaton<Instance>* owner = handle.get_instance();
	automaton<OutputInstance>* output_automaton = output_handle.get_instance();
	automaton<InputInstance>* input_automaton = input_handle.get_instance();
	if(!m_composition_manager.composed(output_automaton, output_member, input_automaton, input_member)) {
	  if(m_composition_manager.composed(output_automaton, output_member, input_automaton, input_member)) {
	    // Decompose.
	    BOOST_ASSERT(false);
	  }
	  else {
	    callback.not_owner();
	  }
	}
	else {
	  callback.not_composed();
	}
      }      
    }

    void rescind();
    void destroy();

    // Non-modifying methods.
    template<class OutputInstance, class OutputMember>
    void execute_output(const automaton_handle<OutputInstance>& output_handle,
			OutputMember OutputInstance::*output_member) {
      // Lock the system so automata can't disappear.
      boost::shared_lock<boost::shared_mutex> lock(*this);
      if (output_handle.valid()) {
	m_composition_manager.execute(output_handle.get_automaton(), output_member);
      }
    }

    template<class Instance, class InternalAction>
    void execute_internal(const automaton_handle<Instance>& handle,
			  InternalAction Instance::*ptr) {
      // Lock the system so automata can't disappear.
      boost::shared_lock<boost::shared_mutex> lock(*this);
      if (handle.valid()) {
	automaton<Instance>* automaton = handle.get_automaton();
	// Lock the automaton.
	abstract_automaton::lock_type lock(*automaton);
	// Execute the action.
	Instance* instance = automaton->get_typed_instance();
	InternalAction& ref = (*instance).*ptr;
	ref();
      }
    }

    void execute_free_input();

    void execute_callback();
  };

}

#endif
