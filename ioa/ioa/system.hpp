#ifndef __system_hpp__
#define __system_hpp__

#include <boost/foreach.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <set>
#include <list>
#include "automaton.hpp"
#include "action.hpp"
#include "composition.hpp"

namespace ioa {
  
  class system
  {
  private:    
    struct automaton_interface_instance_equal
    {
      void* instance;
      
      automaton_interface_instance_equal (void* instance) :
    	instance (instance)
      { }
      
      bool operator() (const std::pair<automaton_interface* const, serial_type>& automaton) const {
    	return instance == automaton.first->get_instance ();
      }
    };
    
    class composition_equal
    {
    private:
      const output_action_interface& m_output;
      const input_action_interface& m_input;
      
    public:
      composition_equal (const output_action_interface& output,
  			 const input_action_interface& input) :
  	m_output (output),
  	m_input (input)
      { }
      
      bool operator() (const composition_interface* c) const {
  	return c->involves_output (m_output) && c->involves_input (m_input, true);
      }
    };
    
    class composition_output_equal
    {
    private:
      const output_action_interface& m_output;
      
    public:
      composition_output_equal (const output_action_interface& output) :
  	m_output (output)
      { }
      
      bool operator() (const composition_interface* c) const {
  	return c->involves_output (m_output);
      }
    };
    
    class composition_input_equal
    {
    private:
      const input_action_interface& m_input;
      
    public:
      composition_input_equal (const input_action_interface& input) :
  	m_input (input)
      { }
      
      bool operator() (const composition_interface* c) const {
  	return c->involves_input (m_input, false);
      }
    };
    
    boost::shared_mutex m_mutex;
    locker<automaton_interface*> m_automata;
    //   std::map<timestamp<automaton>, timestamp<automaton> > m_parent_child;
    std::list<composition_interface*> m_compositions;
    
  public:
    
    ~system ()
    {
      for (locker<automaton_interface*>::const_iterator pos = m_automata.begin ();
	   pos != m_automata.end ();
	   ++pos) {
	delete pos->first;
      }
    }
    
    enum create_result_type {
      CREATE_CREATOR_DNE,
      CREATE_EXISTS,
      CREATE_SUCCESS,
    };
    
    template <class T>
    struct create_result
    {
      create_result_type type;
      automaton_handle<T> automaton;
      
      create_result (const automaton_handle<T>& automaton) :
  	type (CREATE_SUCCESS),
  	automaton (automaton)
      { }
      
      create_result (const create_result_type type) :
  	type (type)
      {
  	BOOST_ASSERT (type != CREATE_SUCCESS);
      }
    };
    
    template <class T>
    create_result<T> create (T* instance)
    {
      BOOST_ASSERT (instance != 0);
      
      boost::unique_lock<boost::shared_mutex> lock (m_mutex);
      
      locker<automaton_interface*>::const_iterator pos = std::find_if (m_automata.begin (),
								       m_automata.end (),
								       automaton_interface_instance_equal (instance));
      
      if (pos != m_automata.end ()) {
      	return create_result<T> (CREATE_EXISTS);
      }
      
      automaton<T>* created = new automaton<T> (instance);
      automaton_handle<T> handle = m_automata.insert (created);
      
      return create_result<T> (handle);
    }
    
    //   template <class T>
    //   create_result<T>
    //   create (const timestamp<automaton>& creator,
    // 	    T* instance)
    //   {
    //     BOOST_ASSERT (instance != 0);
    
    //     boost::unique_lock<boost::shared_mutex> lock (m_mutex);
    
    //     if (timestamp_to_automaton (creator) == 0) {
    // 	return create_result<T> (CREATE_CREATOR_DNE);
    //     }
    
    //     std::list<automaton*>::const_iterator pos = std::find_if (m_automata.begin (),
    // 									  m_automata.end (),
    // 									  automata_instance_equal (instance));
    //     if (pos != m_automata.end ()) {
    // 	return create_result<T> (CREATE_EXISTS);
    //     }
    
    //     typed_automaton<T>* created = new typed_automaton<T> (instance);
    //     m_automata.push_back (created);
    
    //     m_parent_child.insert (std::make_pair (creator, created));
    
    //     return create_result<T> (created->get_typed_timestamp ());
    //   }
    
    enum declare_result_type {
      DECLARE_AUTOMATON_DNE,
      DECLARE_EXISTS,
      DECLARE_SUCCESS,
    };
    
    template <class T>
    struct declare_result
    {
      declare_result_type type;
      parameter_handle<T> parameter;
      
      declare_result (const parameter_handle<T>& param) :
	type (DECLARE_SUCCESS),
	parameter (param)
      { }
      
      declare_result (declare_result_type type) :
  	type (type)
      {
	BOOST_ASSERT (type != DECLARE_SUCCESS);
      }
    };
    
    template <class T>
    declare_result<T>
    declare (const generic_automaton_handle& a,
  	     T* parameter)
    {
      boost::unique_lock<boost::shared_mutex> lock (m_mutex);
      
      if (!m_automata.contains (a)) {
  	return declare_result<T> (DECLARE_AUTOMATON_DNE);	
      }
      
      automaton_interface* pa = a.value ();
      
      if (pa->parameter_exists (parameter)) {
  	return declare_result<T> (DECLARE_EXISTS);
      }
      
      return declare_result<T> (pa->declare_parameter (parameter));
    }
    
    enum compose_result_type {
      COMPOSE_COMPOSER_AUTOMATON_DNE,
      COMPOSE_OUTPUT_AUTOMATON_DNE,
      COMPOSE_INPUT_AUTOMATON_DNE,
      COMPOSE_OUTPUT_PARAMETER_DNE,
      COMPOSE_INPUT_PARAMETER_DNE,
      COMPOSE_EXISTS,
      COMPOSE_INPUT_ACTION_UNAVAILABLE,
      COMPOSE_OUTPUT_ACTION_UNAVAILABLE,
      COMPOSE_SUCCESS,
    };
    
    struct compose_result
    {
      compose_result_type type;
      
      compose_result (compose_result_type type) :
  	type (type)
      { }
    };
    
  private:
    template <class OM, class IM>
    compose_result
    compose (const action<OM>& output,
  	     const action<IM>& input)
    {
      if (!m_automata.contains (input.get_composer_handle ())) {
  	// Owner DNE.
  	return compose_result (COMPOSE_COMPOSER_AUTOMATON_DNE);
      }
      
      if (!output.parameter_exists ()) {
  	return compose_result (COMPOSE_OUTPUT_PARAMETER_DNE);
      }
      
      if (!input.parameter_exists ()) {
  	return compose_result (COMPOSE_INPUT_PARAMETER_DNE);
      }
      
      std::list<composition_interface*>::const_iterator pos = std::find_if (m_compositions.begin (),
									    m_compositions.end (),
									    composition_equal (output, input));
      
      if (pos != m_compositions.end ()) {
  	// Composed.
  	return compose_result (COMPOSE_EXISTS);
      }
      
      std::list<composition_interface*>::const_iterator in_pos = std::find_if (m_compositions.begin (),
									       m_compositions.end (),
									       composition_input_equal (input));
      
      if (in_pos != m_compositions.end ()) {
  	// Input unavailable.
  	return compose_result (COMPOSE_INPUT_ACTION_UNAVAILABLE);
      }
      
      std::list<composition_interface*>::const_iterator out_pos = std::find_if (m_compositions.begin (),
										m_compositions.end (),
										composition_output_equal (output));
      
      if (output.get_automaton_handle () == input.get_automaton_handle () ||
  	  (out_pos != m_compositions.end () && (*out_pos)->involves_input_automaton (input.get_automaton_handle ()))) {
  	// Output unavailable.
  	return compose_result (COMPOSE_OUTPUT_ACTION_UNAVAILABLE);
      }
      
      composition<OM>* c;
      
      if (out_pos != m_compositions.end ()) {
	c = static_cast<composition<OM>*> (*out_pos);
    }
    else {
      c = new composition<OM> (output);
      m_compositions.push_front (c);
    }
    
    // Compose.
    c->compose (input);
    return compose_result (COMPOSE_SUCCESS);
  }
    
    template <class OI, class OM>
    OM*
    get_output_member (const automaton_handle<OI>& output_automaton,
  		       OM OI::*output_member_ptr)
    {
      if (!m_automata.contains (output_automaton)) {
  	return 0;
      }
      
      automaton<OI>* toa = output_automaton.value ();
      return &((*(toa->get_typed_instance ())).*output_member_ptr);
    }
    
  template <class II, class IM>
  IM*
  get_input_member (const automaton_handle<II>& input_automaton,
		    IM II::*input_member_ptr)
  {
    if (!m_automata.contains (input_automaton)) {
      return 0;
    }
      
    automaton<II>* tia = input_automaton.value ();
    return &((*(tia->get_typed_instance ())).*input_member_ptr);
  }
  
public:
  template <class OI, class OM, class II, class IM>
  compose_result
  compose (const automaton_handle<OI>& output_automaton,
	   OM OI::*output_member_ptr,
	   const automaton_handle<II>& input_automaton,
	   IM II::*input_member_ptr,
	   const generic_automaton_handle& composer_automaton)
  {
    boost::unique_lock<boost::shared_mutex> lock (m_mutex);
      
    OM* output_member = get_output_member (output_automaton, output_member_ptr);
    if (output_member == 0) {
      return compose_result (COMPOSE_OUTPUT_AUTOMATON_DNE);
    }
      
    IM* input_member = get_input_member (input_automaton, input_member_ptr);
    if (input_member == 0) {
      return compose_result (COMPOSE_INPUT_AUTOMATON_DNE);
    }
      
    action<OM> o (output_automaton, *output_member);
    action<IM> i (input_automaton, *input_member, composer_automaton);
      
    return compose (o, i);
  }
  
  template <class OI, class OM, class II, class IM, class T>
  compose_result
  compose (const automaton_handle<OI>& output_automaton,
	   OM OI::*output_member_ptr,
	   const parameter_handle<T>& output_parameter,	     
	   const automaton_handle<II>& input_automaton,
	   IM II::*input_member_ptr)
  {
    boost::unique_lock<boost::shared_mutex> lock (m_mutex);

    OM* output_member = get_output_member (output_automaton, output_member_ptr);
    if (output_member == 0) {
      return compose_result (COMPOSE_OUTPUT_AUTOMATON_DNE);
    }

    IM* input_member = get_input_member (input_automaton, input_member_ptr);
    if (input_member == 0) {
      return compose_result (COMPOSE_INPUT_AUTOMATON_DNE);
    }

    action<OM> o (output_automaton, *output_member, output_parameter);
    action<IM> i (input_automaton, *input_member, output_automaton);

    return compose (o, i);
  }

  template <class OI, class OM, class II, class IM, class T>
  compose_result
  compose (const automaton_handle<OI>& output_automaton,
	   OM OI::*output_member_ptr,
	   const automaton_handle<II>& input_automaton,
	   IM II::*input_member_ptr,
	   const parameter_handle<T>& input_parameter)
  {
    boost::unique_lock<boost::shared_mutex> lock (m_mutex);
    
    OM* output_member = get_output_member (output_automaton, output_member_ptr);
    if (output_member == 0) {
      return compose_result (COMPOSE_OUTPUT_AUTOMATON_DNE);
    }
    
    IM* input_member = get_input_member (input_automaton, input_member_ptr);
    if (input_member == 0) {
      return compose_result (COMPOSE_INPUT_AUTOMATON_DNE);
    }
    
    action<OM> o (output_automaton, *output_member);
    action<IM> i (input_automaton, *input_member, input_parameter, input_automaton);
    
    return compose (o, i);
  }

  //   // decompose_result decompose (output* o, input* i) { }
  //   // rescind_result rescind (automaton* automaton, void* parameter) { }
  //   // destroy_result destroy (automaton* destroyer, automaton* destroyee) { }

  enum execute_result_type {
    EXECUTE_AUTOMATON_DNE,
    EXECUTE_PARAMETER_DNE,
    EXECUTE_SUCCESS,
  };

  struct execute_result
  {
    execute_result_type type;

    execute_result (execute_result_type type) :
      type (type)
    { }
  };

private:
  execute_result
  execute_output (output_action_interface& ac)
  {
    std::list<composition_interface*>::const_iterator out_pos = std::find_if (m_compositions.begin (),
									      m_compositions.end (),
									      composition_output_equal (ac));

    if (out_pos == m_compositions.end ()) {
      // Not composed.
      ac.lock_automaton ();
      ac.execute ();
      ac.unlock_automaton ();
    }
    else {
      (*out_pos)->execute ();
    }

    return execute_result (EXECUTE_SUCCESS);
  }

public:
  template <class OI, class OM>
  execute_result
  execute_output (const automaton_handle<OI>& output_automaton,
		  OM OI::*output_member_ptr)
  {
    boost::shared_lock<boost::shared_mutex> lock (m_mutex);
      
    OM* output_member = get_output_member (output_automaton, output_member_ptr);
    if (output_member == 0) {
      return execute_result (EXECUTE_AUTOMATON_DNE);
    }
      
    action<OM> ac (output_automaton, *output_member);
    return execute_output (ac);
  }

  template <class OI, class OM, class T>
  execute_result
  execute_output (const automaton_handle<OI>& output_automaton,
		  OM OI::*output_member_ptr,
		  const parameter_handle<T>& output_parameter)
  {
    boost::shared_lock<boost::shared_mutex> lock (m_mutex);
      
    OM* output_member = get_output_member (output_automaton, output_member_ptr);
    if (output_member == 0) {
      return execute_result (EXECUTE_AUTOMATON_DNE);
    }

    action<OM> ac (output_automaton, *output_member, output_parameter);

    if (!ac.parameter_exists ()) {
      return execute_result (EXECUTE_PARAMETER_DNE);
    }

    return execute_output (ac);
  }

  // void execute_internal () { }
  // void deliver_event () { }
};

}

#endif
