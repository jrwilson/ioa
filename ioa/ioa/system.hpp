#ifndef __system_hpp__
#define __system_hpp__

#include <boost/foreach.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <set>
#include <list>
#include "automaton.hpp"
#include "action.hpp"
#include "binding.hpp"

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
    
    class binding_equal
    {
    private:
      const output_action_interface& m_output;
      const input_action_interface& m_input;
      const generic_automaton_handle& m_binder;

    public:
      binding_equal (const output_action_interface& output,
		     const input_action_interface& input,
		     const generic_automaton_handle& binder) :
  	m_output (output),
  	m_input (input),
	m_binder (binder)
      { }
      
      bool operator() (const binding_interface* c) const {
  	return c->involves_output (m_output) && c->involves_input (m_input, m_binder);
      }
    };
    
    class binding_output_equal
    {
    private:
      const output_action_interface& m_output;
      
    public:
      binding_output_equal (const output_action_interface& output) :
  	m_output (output)
      { }
      
      bool operator() (const binding_interface* c) const {
  	return c->involves_output (m_output);
      }
    };
    
    class binding_input_equal
    {
    private:
      const input_action_interface& m_input;
      
    public:
      binding_input_equal (const input_action_interface& input) :
  	m_input (input)
      { }
      
      bool operator() (const binding_interface* c) const {
  	return c->involves_input (m_input);
      }
    };
    
    boost::shared_mutex m_mutex;
    locker<automaton_interface*> m_automata;
    std::list<std::pair<generic_automaton_handle, generic_automaton_handle> > m_parent_child;
    std::list<binding_interface*> m_bindings;
    
  public:
    
    void clear (void) {
      for (locker<automaton_interface*>::const_iterator pos = m_automata.begin ();
	   pos != m_automata.end ();
	   ++pos) {
	delete pos->first;
      }
      m_automata.clear ();
      m_parent_child.clear ();
      for (std::list<binding_interface*>::const_iterator pos = m_bindings.begin ();
	   pos != m_bindings.end ();
	   ++pos) {
	delete (*pos);
      }
      m_bindings.clear ();
    }

    ~system (void) {
      clear ();
    }
    
    enum create_result_type {
      CREATE_CREATOR_DNE,
      CREATE_EXISTS,
      CREATE_SUCCESS,
    };
    
    struct create_result
    {
      create_result_type type;
      generic_automaton_handle automaton;
      
      create_result (const generic_automaton_handle& automaton) :
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
    create_result inner_create (T* instance)
    {
      BOOST_ASSERT (instance != 0);
      
      locker<automaton_interface*>::const_iterator pos = std::find_if (m_automata.begin (),
								       m_automata.end (),
								       automaton_interface_instance_equal (instance));
      
      if (pos != m_automata.end ()) {
      	return create_result (CREATE_EXISTS);
      }
      
      automaton<T>* created = new automaton<T> (instance);
      automaton_handle<T> handle = m_automata.insert (created);
      
      return create_result (handle);
    }

  public:
    
    template <class T>
    create_result create (T* instance)
    {
      BOOST_ASSERT (instance != 0);
      
      boost::unique_lock<boost::shared_mutex> lock (m_mutex);

      return inner_create (instance);
    }
    
    template <class T>
    create_result
    create (const generic_automaton_handle& creator,
    	    T* instance)
    {
      BOOST_ASSERT (instance != 0);
    
      boost::unique_lock<boost::shared_mutex> lock (m_mutex);
    
      if (!m_automata.contains (creator)) {
	delete instance;
     	return create_result (CREATE_CREATOR_DNE);
      }

      create_result result = inner_create (instance);
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
    
    struct declare_result
    {
      declare_result_type type;
      generic_parameter_handle parameter;
      
      declare_result (const generic_parameter_handle& param) :
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
    declare_result
    declare (const generic_automaton_handle& a,
  	     T* parameter)
    {
      boost::unique_lock<boost::shared_mutex> lock (m_mutex);
      
      if (!m_automata.contains (a)) {
  	return declare_result (DECLARE_AUTOMATON_DNE);	
      }
      
      automaton_interface* pa = a.value ();
      
      if (pa->parameter_exists (parameter)) {
  	return declare_result (DECLARE_EXISTS);
      }
      
      return declare_result (pa->declare_parameter (parameter));
    }
    
    enum bind_result_type {
      BIND_BINDER_AUTOMATON_DNE,
      BIND_OUTPUT_AUTOMATON_DNE,
      BIND_INPUT_AUTOMATON_DNE,
      BIND_OUTPUT_PARAMETER_DNE,
      BIND_INPUT_PARAMETER_DNE,
      BIND_EXISTS,
      BIND_INPUT_ACTION_UNAVAILABLE,
      BIND_OUTPUT_ACTION_UNAVAILABLE,
      BIND_SUCCESS,
    };
    
    struct bind_result
    {
      bind_result_type type;
      
      bind_result (bind_result_type type) :
  	type (type)
      { }
    };
    
  private:
    template <class OM, class IM>
    bind_result
    _bind (const action<OM>& output,
	   const action<IM>& input,
	   const generic_automaton_handle& binder)
    {
      if (!m_automata.contains (output.get_automaton_handle ())) {
	return bind_result (BIND_OUTPUT_AUTOMATON_DNE);
      }

      if (!m_automata.contains (input.get_automaton_handle ())) {
	return bind_result (BIND_INPUT_AUTOMATON_DNE);
      }

      if (!m_automata.contains (binder)) {
  	// Binder DNE.
  	return bind_result (BIND_BINDER_AUTOMATON_DNE);
      }
      
      if (!output.parameter_exists ()) {
  	return bind_result (BIND_OUTPUT_PARAMETER_DNE);
      }
      
      if (!input.parameter_exists ()) {
  	return bind_result (BIND_INPUT_PARAMETER_DNE);
      }
      
      std::list<binding_interface*>::const_iterator pos = std::find_if (m_bindings.begin (),
									m_bindings.end (),
									binding_equal (output, input, binder));
      
      if (pos != m_bindings.end ()) {
  	// Bound.
  	return bind_result (BIND_EXISTS);
      }
      
      std::list<binding_interface*>::const_iterator in_pos = std::find_if (m_bindings.begin (),
									   m_bindings.end (),
									   binding_input_equal (input));
      
      if (in_pos != m_bindings.end ()) {
  	// Input unavailable.
  	return bind_result (BIND_INPUT_ACTION_UNAVAILABLE);
      }
      
      std::list<binding_interface*>::const_iterator out_pos = std::find_if (m_bindings.begin (),
									    m_bindings.end (),
									    binding_output_equal (output));
      
      if (output.get_automaton_handle () == input.get_automaton_handle () ||
  	  (out_pos != m_bindings.end () && (*out_pos)->involves_input_automaton (input.get_automaton_handle ()))) {
  	// Output unavailable.
  	return bind_result (BIND_OUTPUT_ACTION_UNAVAILABLE);
      }
      
      binding<OM>* c;
      
      if (out_pos != m_bindings.end ()) {
	c = static_cast<binding<OM>*> (*out_pos);
      }
      else {
	c = new binding<OM> (output);
	m_bindings.push_front (c);
      }
    
      // Bind.
      c->bind (input, binder);
      return bind_result (BIND_SUCCESS);
    }

    template <class OM, class IM>
    bind_result
    bind (const action<OM>& output,
	  output_category /* */,
	  unparameterized /* */,
	  const action<IM>& input,
	  input_category /* */,
	  unparameterized /* */,
	  const generic_automaton_handle& binder)
    {
      return _bind (output, input, binder);
    }    

    template <class OM, class IM>
    bind_result
    bind (const action<OM>& output,
	  output_category /* */,
	  parameterized /* */,
	  const action<IM>& input,
	  input_category /* */,
	  unparameterized /* */)
    {
      return _bind (output, input, output.get_automaton_handle ());
    }    

    template <class OM, class IM>
    bind_result
    bind (const action<OM>& output,
	  output_category /* */,
	  unparameterized /* */,
	  const action<IM>& input,
	  input_category /* */,
	  parameterized /* */)
    {
      return _bind (output, input, input.get_automaton_handle ());
    }    

  public:
    template <class OM, class IM>
    bind_result
    bind (const action<OM>& output,
	  const action<IM>& input,
	  const generic_automaton_handle& binder)
    {
      boost::unique_lock<boost::shared_mutex> lock (m_mutex);
      
      return bind (output,
		   typename action<OM>::action_category (),
		   typename action<OM>::parameter_status (),
		   input,
		   typename action<IM>::action_category (),
		   typename action<IM>::parameter_status (),
		   binder);
    }

    template <class OM, class IM>
    bind_result
    bind (const action<OM>& output,
	  const action<IM>& input)
    {
      boost::unique_lock<boost::shared_mutex> lock (m_mutex);

      return bind (output,
		   typename action<OM>::action_category (),
		   typename action<OM>::parameter_status (),
		   input,
		   typename action<IM>::action_category (),
		   typename action<IM>::parameter_status ());
    }
    
    enum unbind_result_type {
      UNBIND_BINDER_AUTOMATON_DNE,
      UNBIND_OUTPUT_AUTOMATON_DNE,
      UNBIND_INPUT_AUTOMATON_DNE,
      UNBIND_OUTPUT_PARAMETER_DNE,
      UNBIND_INPUT_PARAMETER_DNE,
      UNBIND_EXISTS,
      UNBIND_SUCCESS,
    };
    
    struct unbind_result
    {
      unbind_result_type type;
    
      unbind_result (unbind_result_type type) :
	type (type)
      { }
    };
  
  private:
    template <class OM, class IM>
    unbind_result
    _unbind (const action<OM>& output,
	     const action<IM>& input,
	     const generic_automaton_handle& binder)
    {
      if (!m_automata.contains (output.get_automaton_handle ())) {
        return unbind_result (UNBIND_OUTPUT_AUTOMATON_DNE);
      }

      if (!m_automata.contains (input.get_automaton_handle ())) {
        return unbind_result (UNBIND_INPUT_AUTOMATON_DNE);
      }

      if (!m_automata.contains (binder)) {
	// Owner DNE.
	return unbind_result (UNBIND_BINDER_AUTOMATON_DNE);
      }
      
      if (!output.parameter_exists ()) {
	return unbind_result (UNBIND_OUTPUT_PARAMETER_DNE);
      }
      
      if (!input.parameter_exists ()) {
	return unbind_result (UNBIND_INPUT_PARAMETER_DNE);
      }
      
      std::list<binding_interface*>::iterator pos = std::find_if (m_bindings.begin (),
								  m_bindings.end (),
								  binding_equal (output, input, binder));
      
      if (pos == m_bindings.end ()) {
	// Not bound.
	return unbind_result (UNBIND_EXISTS);
      }
      
      binding<OM>* c = static_cast<binding<OM>*> (*pos);
      
      // Unbind.
      c->unbind (input, binder);
      
      if (c->empty ()) {
	delete c;
	m_bindings.erase (pos);
      }
      
      return unbind_result (UNBIND_SUCCESS);
    }

    template <class OM, class IM>
    unbind_result
    unbind (const action<OM>& output,
	    unparameterized /* */,
	    const action<IM>& input,
	    unparameterized /* */,
	    const generic_automaton_handle& binder)
    {
      return _unbind (output, input, binder);
    }    
    
    template <class OM, class IM>
    unbind_result
    unbind (const action<OM>& output,
	    parameterized /* */,
	    const action<IM>& input,
	    unparameterized /* */)
    {
      return _unbind (output, input, output.get_automaton_handle ());
    }    
    
    template <class OM, class IM>
    unbind_result
    unbind (const action<OM>& output,
	    unparameterized /* */,
	    const action<IM>& input,
	    parameterized /* */)
    {
      return _unbind (output, input, input.get_automaton_handle ());
    }    
    
  public:
    template <class OM, class IM>
    unbind_result
    unbind (const action<OM>& output,
	    const action<IM>& input,
	    const generic_automaton_handle& binder)
    {
      boost::unique_lock<boost::shared_mutex> lock (m_mutex);

      return unbind (output, typename action<OM>::parameter_status (), input, typename action<IM>::parameter_status (), binder);
    }

    template <class OM, class IM>
    unbind_result
    unbind (const action<OM>& output,
	    const action<IM>& input)
    {
      boost::unique_lock<boost::shared_mutex> lock (m_mutex);

      return unbind (output, typename action<OM>::parameter_status (), input, typename action<IM>::parameter_status ());
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

      for (std::list<binding_interface*>::iterator pos = m_bindings.begin ();
	   pos != m_bindings.end ();
	   ) {
	(*pos)->unbind_parameter (a, p, listener);
	if ((*pos)->empty ()) {
	  delete *pos;
	  pos = m_bindings.erase (pos);
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
	
	// Update bindings.
	for (std::list<binding_interface*>::iterator pos = m_bindings.begin ();
	     pos != m_bindings.end ();
	     ) {
	  (*pos)->unbind_automaton (handle, listener);
	  if ((*pos)->empty ()) {
	    delete *pos;
	    pos = m_bindings.erase (pos);
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
    execute_executable (const executable_action_interface& ac,
			scheduler_interface& scheduler)
    {
      ac.lock_automaton ();
      scheduler.set_current_handle (ac.get_automaton_handle ());
      ac.execute ();
      scheduler.clear_current_handle ();
      ac.unlock_automaton ();
      return execute_result (EXECUTE_SUCCESS);
    }

  public:
    execute_result
    execute (const output_action_interface& ac,
	     scheduler_interface& scheduler)
    {
      boost::shared_lock<boost::shared_mutex> lock (m_mutex);

      if (!m_automata.contains (ac.get_automaton_handle ())) {
	return execute_result (EXECUTE_AUTOMATON_DNE);
      }
      
      if (!ac.parameter_exists ()) {
	return execute_result (EXECUTE_PARAMETER_DNE);
      }

      std::list<binding_interface*>::const_iterator out_pos = std::find_if (m_bindings.begin (),
									    m_bindings.end (),
									    binding_output_equal (ac));

      if (out_pos == m_bindings.end ()) {
	// Not bound.
	return execute_executable (ac, scheduler);
      }
      else {	
	(*out_pos)->execute (scheduler);
	return execute_result (EXECUTE_SUCCESS);
      }
    }

    execute_result
    execute (const independent_action_interface& ac,
	     scheduler_interface& scheduler)
    {
      boost::shared_lock<boost::shared_mutex> lock (m_mutex);
      
      if (!m_automata.contains (ac.get_automaton_handle ())) {
	return execute_result (EXECUTE_AUTOMATON_DNE);
      }
      
      if (!ac.parameter_exists ()) {
	return execute_result (EXECUTE_PARAMETER_DNE);
      }
      
      return execute_executable (ac, scheduler);
    }

  };

  template <class T>
  automaton_handle<T> cast_automaton (const generic_automaton_handle& handle)
  {
    locker_key<automaton<T>*> key (handle.serial (), static_cast<automaton<T>*> (handle.value ()));
    return automaton_handle<T> (key);
  }

  template <class T>
  parameter_handle<T> cast_parameter (const generic_parameter_handle& handle)
  {
    locker_key<T*> key (handle.serial (), static_cast<T*> (handle.value ()));
    return parameter_handle<T> (key);
  }

}

#endif
