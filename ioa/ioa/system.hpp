#ifndef __system_hpp__
#define __system_hpp__

#include "composition_manager.hpp"

namespace ioa {

  typedef enum {
    AUTOMATON_CREATED,
    AUTOMATON_EXISTS
  } create_result_type;

  template <class Instance>
  struct create_result {
    create_result_type type;
    automaton_handle<Instance> handle;
    create_result (const create_result_type type,
		   const automaton_handle<Instance>& handle) :
      type (type),
      handle (handle)
    { }
  };

  template <class Instance>
  create_result<Instance> make_create_result (const create_result_type type,
					      const automaton_handle<Instance>& handle) {
    return create_result<Instance> (type, handle);
  }

  typedef enum {
    PARAMETER_DECLARED,
    PARAMETER_EXISTS
  } declare_result_type;

  template <class Instance, class Parameter>
  struct declare_result {
    declare_result_type type;
    parameter_handle<Instance, Parameter> handle;
    declare_result (const declare_result_type type,
		    const parameter_handle<Instance, Parameter>& handle) :
      type (type),
      handle (handle)
    { }
  };
  
  template <class Instance, class Parameter>
  declare_result<Instance, Parameter> make_declare_result (const declare_result_type type,
							   const parameter_handle<Instance, Parameter>& handle) {
    return declare_result<Instance, Parameter> (type, handle);
  }
  
  typedef enum {
    OUTPUT_INVALID,
    INPUT_INVALID,
    COMPOSITION_EXISTS,
    INPUT_UNAVAILABLE,
    OUTPUT_UNAVAILABLE,
    COMPOSED,
    DECOMPOSED
  } compose_result_type;

  struct compose_result {
    compose_result_type type;
    compose_result (const compose_result_type type) :
      type (type)
    { }
  };
  
  compose_result make_compose_result (const compose_result_type type) {
    return compose_result (type);
  }

  struct root_automaton {
  };

  class system : public boost::shared_mutex {
  private:
    std::set<automaton*> m_automata;
    composition_manager m_composition_manager;

    struct automaton_instance_equal {
      void* m_instance;
      automaton_instance_equal(void* instance)
	: m_instance(instance) { }

      bool operator()(const automaton* aa) const {
	return m_instance == aa->get_instance();
      }
    };

  public:
    const automaton_handle<root_automaton> root_handle;

    system()
      : root_handle(new typed_automaton<root_automaton>()) { }

    ~system() {
      BOOST_FOREACH(automaton* aa, m_automata) {
	delete aa;
      }
    }

    // Non-modifying methods.
    template <class Action>
    void execute (const handle_interface& handle,
    		  Action& action) {
      // Lock the system so automata can't disappear.
      boost::shared_lock<boost::shared_mutex> lock (*this);
      if (handle.valid ()) {
    	BOOST_ASSERT (handle.get_automaton () == action.get_automaton ());
    	m_composition_manager.execute (action);
      }
    }

    // Modifying methods.
    template <class Child, class Callback>
    void create (const handle_interface& parent,
		 Child* child,
		 Callback& callback) {
      boost::unique_lock<boost::shared_mutex> lock (*this);
      if (parent.valid ()) {
	std::set<automaton*>::const_iterator pos =
	  std::find_if (m_automata.begin (),
			m_automata.end (),
			automaton_instance_equal (child));
	if(pos == m_automata.end ()) {
	  typed_automaton<Child>* c = new typed_automaton<Child> (child);
	  m_automata.insert (c);
	  const automaton_handle<Child> h (c);
	  callback (make_create_result (AUTOMATON_CREATED, h));
	}
	else {
	  const automaton_handle<Child> h;
	  callback (make_create_result (AUTOMATON_EXISTS, h));
	}
      }
      else {
	delete child;
      }
    }
    
    template <class Instance, class Parameter, class Callback>
    void declare (const automaton_handle<Instance>& handle,
    		  Parameter* parameter,
    		  Callback& callback) {
      boost::unique_lock<boost::shared_mutex> lock (*this);
      if (handle.valid ()) {
	typed_automaton<Instance>* automaton = handle.get_automaton ();
	if(!automaton->is_declared (parameter)) {
	  automaton->declare (parameter);
	  parameter_handle<Instance, Parameter> h (automaton, parameter);
	  callback (make_declare_result (PARAMETER_DECLARED, h));
	}
	else {
	  parameter_handle<Instance, Parameter> h (automaton, parameter);
	  callback (make_declare_result (PARAMETER_EXISTS, h));
	}
      }
    }

    template<class OutputAction, class InputAction, class Callback>
    void compose (const handle_interface& handle,
		  const handle_interface& output_handle,
		  const OutputAction& output_action,
		  const handle_interface& input_handle,
		  const InputAction& input_action,
		  Callback& callback) {
      // Lock the system.
      boost::unique_lock<boost::shared_mutex> lock (*this);
      if (handle.valid ()) {
      	if (!output_handle.valid ()) {
      	  callback (make_compose_result (OUTPUT_INVALID));
      	}
      	else if(!input_handle.valid ()) {
      	  callback (make_compose_result (INPUT_INVALID));
      	}
      	else {
	  BOOST_ASSERT (handle.get_automaton () == input_action.get_owner ());
	  BOOST_ASSERT (output_handle.get_automaton () == output_action.get_automaton ());
	  BOOST_ASSERT (input_handle.get_automaton () == input_action.get_automaton ());

      	  if (m_composition_manager.composed (output_action, input_action)) {
      	    callback (make_compose_result (COMPOSITION_EXISTS));
      	  }
      	  else if (!m_composition_manager.input_available (input_action)) {
      	    callback (make_compose_result (INPUT_UNAVAILABLE));
      	  }
      	  else if(!m_composition_manager.output_available (output_action, input_action.get_automaton ())) {
      	    callback (make_compose_result (OUTPUT_UNAVAILABLE));
      	  }
      	  else {
      	    m_composition_manager.compose (output_action, input_action);
      	    callback (make_compose_result (COMPOSED));
      	  }
      	}
      }
    }

    // template<class Callback, class Instance,
    // 	     class OutputInstance, class OutputMember,
    // 	     class InputInstance, class InputMember>
    // void decompose(Callback& callback,
    // 		   const automaton_handle<Instance>& handle,
    // 		   const automaton_handle<OutputInstance>& output_handle,
    // 		   OutputMember OutputInstance::*output_member,
    // 		   const automaton_handle<InputInstance>& input_handle,
    // 		   const InputMember InputInstance::*input_member) {
    //   // Lock the system.
    //   boost::unique_lock<boost::shared_mutex> lock(*this);
    //   if(handle.valid()) {
    // 	automaton<Instance>* owner = handle.get_instance();
    // 	automaton<OutputInstance>* output_automaton = output_handle.get_instance();
    // 	automaton<InputInstance>* input_automaton = input_handle.get_instance();
    // 	if(!m_composition_manager.composed(output_automaton, output_member, input_automaton, input_member)) {
    // 	  if(m_composition_manager.composed(output_automaton, output_member, input_automaton, input_member)) {
    // 	    // Decompose.
    // 	    BOOST_ASSERT(false);
    // 	  }
    // 	  else {
    // 	    callback.not_owner();
    // 	  }
    // 	}
    // 	else {
    // 	  callback.not_composed();
    // 	}
    //   }      
    // }

    // void rescind();
    // void destroy();

  };
}

#endif
