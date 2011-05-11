#ifndef __system_hpp__
#define __system_hpp__

#include <boost/foreach.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/utility.hpp>
#include <set>
#include <list>
#include "automaton.hpp"
#include "binding.hpp"

namespace ioa {

  class system :
    public system_interface,
    public boost::noncopyable
  {
  private:    
    struct automaton_interface_instance_equal
    {
      void* instance;
      
      automaton_interface_instance_equal (void* instance) :
    	instance (instance)
      { }
      
      bool operator() (const std::pair<generic_automaton_record* const, serial_type>& automaton) const {
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
    locker<void*> m_instances;
    std::map<generic_automaton_handle, generic_automaton_record*> m_automata;
    std::list<std::pair<generic_automaton_handle, generic_automaton_handle> > m_parent_child;
    std::list<binding_interface*> m_bindings;
    
  public:
    
    void clear (void) {
      m_instances.clear ();
      for (std::map<generic_automaton_handle, generic_automaton_record*>::const_iterator pos = m_automata.begin ();
	   pos != m_automata.end ();
	   ++pos) {
	delete pos->second;
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
    
  public:
    
    template <class T>
    void
    create (T*& instance,
	    create_listener_interface& listener)
    {
      BOOST_ASSERT (instance != 0);
      
      boost::unique_lock<boost::shared_mutex> lock (m_mutex);
      
      if (m_instances.contains (instance)) {
	listener.instance_exists (instance);
	return;
      }
      
      automaton_record<T>* created = new automaton_record<T> (instance);
      automaton_handle<T> handle = m_instances.insert (instance);
      m_automata.insert (std::make_pair (handle, created));

      listener.automaton_created (handle);
      return;
    }
    
    template <class T>
    void
    create (const generic_automaton_handle& creator,
    	    T* instance,
	    create_listener_interface& listener)
    {
      BOOST_ASSERT (instance != 0);
    
      boost::unique_lock<boost::shared_mutex> lock (m_mutex);
    
      if (!m_instances.contains (creator)) {
	delete instance;
	listener.automaton_dne ();
	return;
      }

      if (m_instances.contains (instance)) {
	listener.instance_exists (creator, instance);
	return;
      }
      
      automaton_record<T>* created = new automaton_record<T> (instance);
      automaton_handle<T> handle = m_instances.insert (instance);
      m_automata.insert (std::make_pair (handle, created));
      m_parent_child.push_back (std::make_pair (creator, handle));

      listener.automaton_created (creator, handle);
      return;
    }
    
    void
    declare (const generic_automaton_handle& a,
  	     void* parameter,
	     declare_listener_interface& listener)
    {
      boost::unique_lock<boost::shared_mutex> lock (m_mutex);
      
      if (!m_instances.contains (a)) {
	listener.automaton_dne ();
  	return;
      }
      
      generic_automaton_record* pa = m_automata[a];
      
      if (pa->parameter_exists (parameter)) {
	listener.parameter_exists (a, parameter);
	return;
      }
      
      listener.parameter_declared (a, pa->declare_parameter (parameter));
      return;
    }
    
  private:
    bool parameter_exists (const action_interface& ac) {
      if (ac.is_parameterized ()) {
	return m_automata[ac.get_automaton_handle ()]->parameter_exists (ac.get_parameter_handle ());
      }
      else {
	return true;
      }
    }

    template <class OM, class IM>
    void
    _bind (const action<OM>& output,
	   const action<IM>& input,
	   const generic_automaton_handle& binder,
	   bind_listener_interface& listener)
    {
      if (!m_instances.contains (binder)) {
  	// Binder DNE.
	listener.automaton_dne ();
  	return;
      }

      if (!m_instances.contains (output.get_automaton_handle ())) {
	listener.bind_output_automaton_dne (binder);
	return;
      }

      if (!m_instances.contains (input.get_automaton_handle ())) {
	listener.bind_input_automaton_dne (binder);
	return;
      }
      
      if (!parameter_exists (output)) {
	listener.bind_output_parameter_dne (binder);
	return;
      }
      
      if (!parameter_exists (input)) {
	listener.bind_input_parameter_dne (binder);
	return;
      }
      
      std::list<binding_interface*>::const_iterator pos = std::find_if (m_bindings.begin (),
									m_bindings.end (),
									binding_equal (output, input, binder));
      
      if (pos != m_bindings.end ()) {
  	// Bound.
	listener.binding_exists (binder);
	return;
      }
      
      std::list<binding_interface*>::const_iterator in_pos = std::find_if (m_bindings.begin (),
									   m_bindings.end (),
									   binding_input_equal (input));
      
      if (in_pos != m_bindings.end ()) {
  	// Input unavailable.
	listener.input_action_unavailable (binder);
	return;
      }
      
      std::list<binding_interface*>::const_iterator out_pos = std::find_if (m_bindings.begin (),
									    m_bindings.end (),
									    binding_output_equal (output));
      
      if (output.get_automaton_handle () == input.get_automaton_handle () ||
  	  (out_pos != m_bindings.end () && (*out_pos)->involves_input_automaton (input.get_automaton_handle ()))) {
  	// Output unavailable.
	listener.output_action_unavailable (binder);
	return;
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

      listener.bound (binder);
      return;
    }

    template <class OM, class IM>
    void
    bind (const action<OM>& output,
	  output_category /* */,
	  unparameterized /* */,
	  const action<IM>& input,
	  input_category /* */,
	  unparameterized /* */,
	  const generic_automaton_handle& binder,
	  bind_listener_interface& listener)
    {
      return _bind (output, input, binder, listener);
    }    

    template <class OM, class IM>
    void
    bind (const action<OM>& output,
	  output_category /* */,
	  parameterized /* */,
	  const action<IM>& input,
	  input_category /* */,
	  unparameterized /* */,
	  bind_listener_interface& listener)
    {
      return _bind (output, input, output.get_automaton_handle (), listener);
    }    

    template <class OM, class IM>
    void
    bind (const action<OM>& output,
	  output_category /* */,
	  unparameterized /* */,
	  const action<IM>& input,
	  input_category /* */,
	  parameterized /* */,
	  bind_listener_interface& listener)
    {
      return _bind (output, input, input.get_automaton_handle (), listener);
    }    

  public:
    template <class OM, class IM>
    void
    bind (const action<OM>& output,
	  const action<IM>& input,
	  const generic_automaton_handle& binder,
	  bind_listener_interface& listener)
    {
      boost::unique_lock<boost::shared_mutex> lock (m_mutex);
      
      return bind (output,
		   typename action<OM>::action_category (),
		   typename action<OM>::parameter_status (),
		   input,
		   typename action<IM>::action_category (),
		   typename action<IM>::parameter_status (),
		   binder,
		   listener);
    }

    template <class OM, class IM>
    void
    bind (const action<OM>& output,
	  const action<IM>& input,
	  bind_listener_interface& listener)
    {
      boost::unique_lock<boost::shared_mutex> lock (m_mutex);

      return bind (output,
		   typename action<OM>::action_category (),
		   typename action<OM>::parameter_status (),
		   input,
		   typename action<IM>::action_category (),
		   typename action<IM>::parameter_status (),
		   listener);
    }
  
  private:
    template <class OM, class IM>
    void
    _unbind (const action<OM>& output,
	     const action<IM>& input,
	     const generic_automaton_handle& binder,
	     unbind_listener_interface& listener)
    {
      if (!m_instances.contains (binder)) {
	listener.automaton_dne ();
	return;
      }

      if (!m_instances.contains (output.get_automaton_handle ())) {
	listener.unbind_output_automaton_dne (binder);
	return;
      }

      if (!m_instances.contains (input.get_automaton_handle ())) {
	listener.unbind_input_automaton_dne (binder);
	return;
      }
      
      if (!parameter_exists (output)) {
	listener.unbind_output_parameter_dne (binder);
	return;
      }
      
      if (!parameter_exists (input)) {
	listener.unbind_input_parameter_dne (binder);
	return;
      }
      
      std::list<binding_interface*>::iterator pos = std::find_if (m_bindings.begin (),
								  m_bindings.end (),
								  binding_equal (output, input, binder));
      
      if (pos == m_bindings.end ()) {
	// Not bound.
	listener.binding_dne (binder);
	return;
      }
      
      binding<OM>* c = static_cast<binding<OM>*> (*pos);
      
      // Unbind.
      c->unbind (input, binder);
      
      if (c->empty ()) {
	delete c;
	m_bindings.erase (pos);
      }

      listener.unbound (binder);
      return;
    }

    template <class OM, class IM>
    void
    unbind (const action<OM>& output,
	    unparameterized /* */,
	    const action<IM>& input,
	    unparameterized /* */,
	    const generic_automaton_handle& binder,
	    unbind_listener_interface& listener)
    {
      return _unbind (output, input, binder, listener);
    }    
    
    template <class OM, class IM>
    void
    unbind (const action<OM>& output,
	    parameterized /* */,
	    const action<IM>& input,
	    unparameterized /* */,
	    unbind_listener_interface& listener)
    {
      return _unbind (output, input, output.get_automaton_handle (), listener);
    }    
    
    template <class OM, class IM>
    void
    unbind (const action<OM>& output,
	    unparameterized /* */,
	    const action<IM>& input,
	    parameterized /* */,
	    unbind_listener_interface& listener)
    {
      return _unbind (output, input, input.get_automaton_handle (), listener);
    }    
    
  public:
    template <class OM, class IM>
    void
    unbind (const action<OM>& output,
	    const action<IM>& input,
	    const generic_automaton_handle& binder,
	    unbind_listener_interface& listener)
    {
      boost::unique_lock<boost::shared_mutex> lock (m_mutex);

      return unbind (output, typename action<OM>::parameter_status (), input, typename action<IM>::parameter_status (), binder, listener);
    }

    template <class OM, class IM>
    void
    unbind (const action<OM>& output,
	    const action<IM>& input,
	    unbind_listener_interface& listener)
    {
      boost::unique_lock<boost::shared_mutex> lock (m_mutex);

      return unbind (output, typename action<OM>::parameter_status (), input, typename action<IM>::parameter_status (), listener);
    }

    void
    rescind (const generic_automaton_handle& a,
  	     const generic_parameter_handle& p,
	     rescind_listener_interface& listener)
    {
      boost::unique_lock<boost::shared_mutex> lock (m_mutex);
      
      if (!m_instances.contains (a)) {
	listener.automaton_dne ();
  	return;
      }
      
      generic_automaton_record* pa = m_automata[a];
      
      if (!pa->parameter_exists (p)) {
	listener.parameter_dne (a, p);
	return;
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
      
      listener.parameter_rescinded (a, p.value ());
      return;
    }

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

    void
    inner_destroy (const generic_automaton_handle& automaton,
		   destroy_listener_interface& listener)
    {
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
	{
	  // Update the list of automata.
	  m_instances.erase (handle);
	  delete m_automata[handle];
	  m_automata.erase (handle);
	}

	listener.automaton_destroyed (handle);

	{
	  // Update parent-child relationships.
	  std::list<std::pair<generic_automaton_handle, generic_automaton_handle> >::iterator pos = std::find_if (m_parent_child.begin (), m_parent_child.end (), child_automaton (handle));
	  if (pos != m_parent_child.end ()) {
	    listener.automaton_destroyed (pos->first, pos->second);
	    m_parent_child.erase (pos);
	  }
	}
	
	{
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
      }
    }

  public:

    void
    destroy (const generic_automaton_handle& target,
	     destroy_listener_interface& listener)
    {
      boost::unique_lock<boost::shared_mutex> lock (m_mutex);
      
      if (!m_instances.contains (target)) {
	listener.target_automaton_dne (target);
	return;
      }

      return inner_destroy (target, listener);
    }

    void
    destroy (const generic_automaton_handle& automaton,
	     const generic_automaton_handle& target,
	     destroy_listener_interface& listener)
    {
      boost::unique_lock<boost::shared_mutex> lock (m_mutex);

      if (!m_instances.contains (automaton)) {
	listener.automaton_dne ();
	return;
      }

      if (!m_instances.contains (target)) {
	listener.target_automaton_dne (automaton, target);
	return;
      }

      if (std::find (m_parent_child.begin (),
		     m_parent_child.end (),
		     std::make_pair (automaton, target)) == m_parent_child.end ()) {
	listener.destroyer_not_creator (automaton, target);
	return;
      }

      return inner_destroy (target, listener);
    }

  private:
    void lock_automaton (const generic_automaton_handle& handle)
    {
      m_automata[handle]->lock ();
    }

    void unlock_automaton (const generic_automaton_handle& handle)
    {
      m_automata[handle]->unlock ();
    }

    void
    execute_executable (const executable_action_interface& ac,
			scheduler_interface& scheduler)
    {
      lock_automaton (ac.get_automaton_handle ());
      scheduler.set_current_handle (ac.get_automaton_handle ());
      ac.execute ();
      scheduler.clear_current_handle ();
      unlock_automaton (ac.get_automaton_handle ());
    }

  public:
    void
    execute (const output_action_interface& ac,
	     scheduler_interface& scheduler,
	     execute_listener_interface& listener)
    {
      boost::shared_lock<boost::shared_mutex> lock (m_mutex);

      if (!m_instances.contains (ac.get_automaton_handle ())) {
	listener.automaton_dne ();
	return;
      }

      if (!parameter_exists (ac)) {
	listener.execute_parameter_dne ();
	return;
      }

      std::list<binding_interface*>::const_iterator out_pos = std::find_if (m_bindings.begin (),
									    m_bindings.end (),
									    binding_output_equal (ac));

      if (out_pos == m_bindings.end ()) {
	// Not bound.
	execute_executable (ac, scheduler);
      }
      else {	
	(*out_pos)->execute (scheduler, *this);
      }
    }

    void
    execute (const independent_action_interface& ac,
	     scheduler_interface& scheduler,
	     execute_listener_interface& listener)
    {
      boost::shared_lock<boost::shared_mutex> lock (m_mutex);
      
      if (!m_instances.contains (ac.get_automaton_handle ())) {
	listener.automaton_dne ();
	return;
      }

      if (!parameter_exists (ac)) {
	listener.execute_parameter_dne ();
	return;
      }
      
      execute_executable (ac, scheduler);
    }

    void
    execute (const generic_automaton_handle& automaton,
	     runnable_interface& c,
	     scheduler_interface& scheduler,
	     system_execute_listener_interface& listener) {
      boost::shared_lock<boost::shared_mutex> lock (m_mutex);
      
      if (!m_instances.contains (automaton)) {
	// The system deleted an automata that still had outstanding system events.
	listener.automaton_dne ();
	return;
      }

      lock_automaton (automaton);
      scheduler.set_current_handle (automaton);
      c ();
      scheduler.clear_current_handle ();
      unlock_automaton (automaton);
    }
  };

  template <class T>
  automaton_handle<T> cast_automaton (const generic_automaton_handle& handle)
  {
    locker_key<T*> key (handle.serial (), static_cast<T*> (handle.value ()));
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
