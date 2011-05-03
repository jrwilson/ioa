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
    std::list<std::pair<generic_automaton_handle, generic_automaton_handle> > m_parent_child;
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

  private:
    template <class T>
    create_result<T> inner_create (T* instance)
    {
      BOOST_ASSERT (instance != 0);
      
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

  public:
    
    template <class T>
    create_result<T> create (T* instance)
    {
      BOOST_ASSERT (instance != 0);
      
      boost::unique_lock<boost::shared_mutex> lock (m_mutex);

      return inner_create (instance);
    }
    
    template <class T>
    create_result<T>
    create (const generic_automaton_handle& creator,
    	    T* instance)
    {
      BOOST_ASSERT (instance != 0);
    
      boost::unique_lock<boost::shared_mutex> lock (m_mutex);
    
      if (!m_automata.contains (creator)) {
	delete instance;
     	return create_result<T> (CREATE_CREATOR_DNE);
      }

      create_result<T> result = inner_create (instance);
      if (result.type == CREATE_SUCCESS) {
	m_parent_child.push_back (std::make_pair (creator, result.automaton));
      }

      return result;
    }
    
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

    template <class I, class M>
    M&
    ptr_to_member (const automaton_handle<I>& handle,
		   M I::*member_ptr)
    {
      automaton<I>* toa = handle.value ();
      return ((*(toa->get_typed_instance ())).*member_ptr);
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

      if (!m_automata.contains (output_automaton)) {
	return compose_result (COMPOSE_OUTPUT_AUTOMATON_DNE);
      }

      if (!m_automata.contains (input_automaton)) {
	return compose_result (COMPOSE_INPUT_AUTOMATON_DNE);
      }

      OM& output_member = ptr_to_member (output_automaton, output_member_ptr);
      IM& input_member = ptr_to_member (input_automaton, input_member_ptr);
      
      action<OM> o (output_automaton, output_member);
      action<IM> i (input_automaton, input_member, composer_automaton);

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

      if (!m_automata.contains (output_automaton)) {
	return compose_result (COMPOSE_OUTPUT_AUTOMATON_DNE);
      }

      if (!m_automata.contains (input_automaton)) {
	return compose_result (COMPOSE_INPUT_AUTOMATON_DNE);
      }

      OM& output_member = ptr_to_member (output_automaton, output_member_ptr);
      IM& input_member = ptr_to_member (input_automaton, input_member_ptr);

      action<OM> o (output_automaton, output_member, output_parameter);
      action<IM> i (input_automaton, input_member, output_automaton);

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

      if (!m_automata.contains (output_automaton)) {
	return compose_result (COMPOSE_OUTPUT_AUTOMATON_DNE);
      }

      if (!m_automata.contains (input_automaton)) {
	return compose_result (COMPOSE_INPUT_AUTOMATON_DNE);
      }

      OM& output_member = ptr_to_member (output_automaton, output_member_ptr);
      IM& input_member = ptr_to_member (input_automaton, input_member_ptr);
    
      action<OM> o (output_automaton, output_member);
      action<IM> i (input_automaton, input_member, input_parameter, input_automaton);
    
      return compose (o, i);
    }

    enum decompose_result_type {
      DECOMPOSE_COMPOSER_AUTOMATON_DNE,
      DECOMPOSE_OUTPUT_AUTOMATON_DNE,
      DECOMPOSE_INPUT_AUTOMATON_DNE,
      DECOMPOSE_OUTPUT_PARAMETER_DNE,
      DECOMPOSE_INPUT_PARAMETER_DNE,
      DECOMPOSE_EXISTS,
      DECOMPOSE_SUCCESS,
    };
    
    struct decompose_result
    {
      decompose_result_type type;
    
      decompose_result (decompose_result_type type) :
	type (type)
      { }
    };
  
  private:
    template <class OM, class IM>
    decompose_result
    decompose (const action<OM>& output,
	       action<IM>& input)
    {
      if (!m_automata.contains (input.get_composer_handle ())) {
	// Owner DNE.
	return decompose_result (DECOMPOSE_COMPOSER_AUTOMATON_DNE);
      }
    
      if (!output.parameter_exists ()) {
	return decompose_result (DECOMPOSE_OUTPUT_PARAMETER_DNE);
      }
    
      if (!input.parameter_exists ()) {
	return decompose_result (DECOMPOSE_INPUT_PARAMETER_DNE);
      }
    
      std::list<composition_interface*>::iterator pos = std::find_if (m_compositions.begin (),
								      m_compositions.end (),
								      composition_equal (output, input));
    
      if (pos == m_compositions.end ()) {
	// Not composed.
	return decompose_result (DECOMPOSE_EXISTS);
      }
    
      composition<OM>* c = static_cast<composition<OM>*> (*pos);
  
      // Compose.
      c->decompose (input);

      if (c->empty ()) {
	delete c;
	m_compositions.erase (pos);
      }

      return decompose_result (DECOMPOSE_SUCCESS);
    }

  public:
    template <class OI, class OM, class II, class IM>
    decompose_result
    decompose (const automaton_handle<OI>& output_automaton,
	       OM OI::*output_member_ptr,
	       const automaton_handle<II>& input_automaton,
	       IM II::*input_member_ptr,
	       const generic_automaton_handle& composer_automaton)
    {
      boost::unique_lock<boost::shared_mutex> lock (m_mutex);

      if (!m_automata.contains (output_automaton)) {
        return decompose_result (DECOMPOSE_OUTPUT_AUTOMATON_DNE);
      }

      if (!m_automata.contains (input_automaton)) {
        return decompose_result (DECOMPOSE_INPUT_AUTOMATON_DNE);
      }

      OM& output_member = ptr_to_member (output_automaton, output_member_ptr);
      IM& input_member = ptr_to_member (input_automaton, input_member_ptr);
      
      action<OM> o (output_automaton, output_member);
      action<IM> i (input_automaton, input_member, composer_automaton);
      
      return decompose (o, i);
    }
  
    template <class OI, class OM, class II, class IM, class T>
    decompose_result
    decompose (const automaton_handle<OI>& output_automaton,
	       OM OI::*output_member_ptr,
	       const parameter_handle<T>& output_parameter,	     
	       const automaton_handle<II>& input_automaton,
	       IM II::*input_member_ptr)
    {
      boost::unique_lock<boost::shared_mutex> lock (m_mutex);

      if (!m_automata.contains (output_automaton)) {
        return decompose_result (DECOMPOSE_OUTPUT_AUTOMATON_DNE);
      }

      if (!m_automata.contains (input_automaton)) {
        return decompose_result (DECOMPOSE_INPUT_AUTOMATON_DNE);
      }

      OM& output_member = ptr_to_member (output_automaton, output_member_ptr);
      IM& input_member = ptr_to_member (input_automaton, input_member_ptr);

      action<OM> o (output_automaton, output_member, output_parameter);
      action<IM> i (input_automaton, input_member, output_automaton);

      return decompose (o, i);
    }

    template <class OI, class OM, class II, class IM, class T>
    decompose_result
    decompose (const automaton_handle<OI>& output_automaton,
	       OM OI::*output_member_ptr,
	       const automaton_handle<II>& input_automaton,
	       IM II::*input_member_ptr,
	       const parameter_handle<T>& input_parameter)
    {
      boost::unique_lock<boost::shared_mutex> lock (m_mutex);
      
      if (!m_automata.contains (output_automaton)) {
        return decompose_result (DECOMPOSE_OUTPUT_AUTOMATON_DNE);
      }

      if (!m_automata.contains (input_automaton)) {
        return decompose_result (DECOMPOSE_INPUT_AUTOMATON_DNE);
      }

      OM& output_member = ptr_to_member (output_automaton, output_member_ptr);
      IM& input_member = ptr_to_member (input_automaton, input_member_ptr);
    
      action<OM> o (output_automaton, output_member);
      action<IM> i (input_automaton, input_member, input_parameter, input_automaton);
    
      return decompose (o, i);
    }

    enum rescind_result_type {
      RESCIND_AUTOMATON_DNE,
      RESCIND_EXISTS,
      RESCIND_SUCCESS,
    };
    
    template <class T>
    struct rescind_result
    {
      rescind_result_type type;
      T* parameter;
      
      rescind_result (T* param) :
	type (RESCIND_SUCCESS),
	parameter (param)
      { }
      
      rescind_result (rescind_result_type type) :
  	type (type)
      {
	BOOST_ASSERT (type != RESCIND_SUCCESS);
      }
    };
    
    template <class T>
    rescind_result<T>
    rescind (const generic_automaton_handle& a,
  	     const parameter_handle<T>& p,
	     rescind_listener_interface& listener)
    {
      boost::unique_lock<boost::shared_mutex> lock (m_mutex);
      
      if (!m_automata.contains (a)) {
  	return rescind_result<T> (RESCIND_AUTOMATON_DNE);	
      }
      
      automaton_interface* pa = a.value ();
      
      if (!pa->parameter_exists (p)) {
  	return rescind_result<T> (RESCIND_EXISTS);
      }

      pa->rescind_parameter (p);

      for (std::list<composition_interface*>::iterator pos = m_compositions.begin ();
	   pos != m_compositions.end ();
	   ) {
	(*pos)->decompose_parameter (a, p, listener);
	if ((*pos)->empty ()) {
	  delete *pos;
	  pos = m_compositions.erase (pos);
	}
	else {
	  ++pos;
	}
      }

      return rescind_result<T> (p.value ());
    }

    enum destroy_result_type {
      DESTROY_DESTROYER_DNE,
      DESTROY_DESTROYER_NOT_CREATOR,
      DESTROY_EXISTS,
      DESTROY_SUCCESS,
    };
    
    struct destroy_result
    {
      destroy_result_type type;
      
      destroy_result (const destroy_result_type type) :
  	type (type)
      { }
    };

  private:
    class child_automaton
    {
    private:
      generic_automaton_handle& m_handle;
    public:
      child_automaton (generic_automaton_handle& handle) :
	m_handle (handle) { }

      bool operator() (const std::pair<generic_automaton_handle, generic_automaton_handle>& p) const {
	return p.second == m_handle;
      }
    };

    destroy_result inner_destroy (const generic_automaton_handle& automaton,
				  destroy_listener_interface& listener)
    {
      if (!m_automata.contains (automaton)) {
	return destroy_result (DESTROY_EXISTS);
      }

      std::set<generic_automaton_handle> closed_list;
      std::set<generic_automaton_handle> open_list;

      // Put the target in the open list.
      open_list.insert (automaton);

      while (!open_list.empty ()) {
	// Grab an item from the open list.
	std::set<generic_automaton_handle>::iterator pos = open_list.begin ();
	const generic_automaton_handle x = *pos;
	open_list.erase (pos);

	// If its not already in the closed list.
	if (closed_list.count (x) == 0) {
	  // Put it in the closed list.
	  closed_list.insert (x);
	  for (std::list<std::pair<generic_automaton_handle, generic_automaton_handle> >::const_iterator pos = m_parent_child.begin ();
	       pos != m_parent_child.end ();
	       ++pos) {
	    if (x == pos->first) {
	      // And put all of its children in the open list.
	      open_list.insert (pos->second);
	    }
	  }
	}
      }

      BOOST_FOREACH (generic_automaton_handle handle, closed_list) {
	// Update the list of automata.
	m_automata.erase (handle);
	delete handle.value ();

	// Update parent-child relationships.
	std::list<std::pair<generic_automaton_handle, generic_automaton_handle> >::iterator pos = std::find_if (m_parent_child.begin (), m_parent_child.end (), child_automaton (handle));
	if (pos != m_parent_child.end ()) {
	  listener.destroyed (pos->first, pos->second);
	  m_parent_child.erase (pos);
	}
	
	// Update compositions.
	for (std::list<composition_interface*>::iterator pos = m_compositions.begin ();
	     pos != m_compositions.end ();
	     ) {
	  (*pos)->decompose_automaton (handle, listener);
	  if ((*pos)->empty ()) {
	    delete *pos;
	    pos = m_compositions.erase (pos);
	  }
	  else {
	    ++pos;
	  }
	}
      }

      return destroy_result (DESTROY_SUCCESS);
    }

  public:

    destroy_result destroy (const generic_automaton_handle& automaton,
			    destroy_listener_interface& listener)
    {
      boost::unique_lock<boost::shared_mutex> lock (m_mutex);

      return inner_destroy (automaton, listener);
    }

    destroy_result destroy (const generic_automaton_handle& destroyer,
			    const generic_automaton_handle& automaton,
			    destroy_listener_interface& listener)
    {
      boost::unique_lock<boost::shared_mutex> lock (m_mutex);

      if (!m_automata.contains (destroyer)) {
	return destroy_result (DESTROY_DESTROYER_DNE);
      }

      if (std::find (m_parent_child.begin (),
		     m_parent_child.end (),
		     std::make_pair (destroyer, automaton)) == m_parent_child.end ()) {
	return destroy_result (DESTROY_DESTROYER_NOT_CREATOR);
      }

      return inner_destroy (automaton, listener);
    }

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

      if (!m_automata.contains (output_automaton)) {
	return execute_result (EXECUTE_AUTOMATON_DNE);
      }

      OM& output_member = ptr_to_member (output_automaton, output_member_ptr);

      action<OM> ac (output_automaton, output_member);

      return execute_output (ac);
    }

    template <class OI, class OM, class T>
    execute_result
    execute_output (const automaton_handle<OI>& output_automaton,
		    OM OI::*output_member_ptr,
		    const parameter_handle<T>& output_parameter)
    {
      boost::shared_lock<boost::shared_mutex> lock (m_mutex);

      if (!m_automata.contains (output_automaton)) {
	return execute_result (EXECUTE_AUTOMATON_DNE);
      }

      OM& output_member = ptr_to_member (output_automaton, output_member_ptr);

      action<OM> ac (output_automaton, output_member, output_parameter);

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
