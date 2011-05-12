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

  class parameter_record_interface
  {
  public:
    virtual ~parameter_record_interface () { }
  };

  template <class RSL>
  class parameter_record :
    public parameter_record_interface
  {
  private:
    generic_automaton_handle m_automaton;
    generic_parameter_handle m_parameter;
    RSL& m_rsl;

  public:
    parameter_record (const generic_automaton_handle& automaton,
		      const generic_parameter_handle& parameter,
		      RSL& rsl) :
      m_automaton (automaton),
      m_parameter (parameter),
      m_rsl (rsl)
    { }

    ~parameter_record () {
      m_rsl.parameter_rescinded (m_automaton, m_parameter);
    }

  };

  class automaton_record_interface :
    public boost::mutex
  {
  private:
    std::auto_ptr<automaton_interface> m_instance;
    locker<void*> m_parameters;
    std::map<void*, parameter_record_interface*> m_records;

  protected:
    generic_automaton_handle m_automaton;
    
  public:
    automaton_record_interface (const generic_automaton_handle& automaton) :
      m_instance (automaton.value ()),
      m_automaton (automaton)
    { }

    virtual ~automaton_record_interface () {
      for (std::map<void*, parameter_record_interface*>::const_iterator pos = m_records.begin ();
	   pos != m_records.end ();
	   ++pos) {
	delete pos->second;
      }
    }
    
    automaton_interface* get_instance () const {
      return m_instance.get ();
    }

    bool parameter_exists (void* parameter) const {
      return m_parameters.contains (parameter);
    }

    bool parameter_exists (const generic_parameter_handle& parameter) const {
      return m_parameters.contains (parameter);
    }

    template <class RSL>
    generic_parameter_handle declare_parameter (void* parameter,
						RSL& rsl) {
      BOOST_ASSERT (!parameter_exists (parameter));
      generic_parameter_handle handle = m_parameters.insert (parameter);
      m_records.insert (std::make_pair (parameter, new parameter_record<RSL> (m_automaton, handle, rsl)));
      return handle;
    }

    void rescind_parameter (const generic_parameter_handle& parameter) {
      BOOST_ASSERT (parameter_exists (parameter));
      delete m_records[parameter.value ()];
      m_records.erase (parameter.value ());
      m_parameters.erase (parameter);
    }

    generic_parameter_handle parameter_handle (void* parameter) {
      return m_parameters.find (parameter);
    }
  };

  template <class DSL>
  class root_automaton_record :
    public automaton_record_interface
  {
  private:
    DSL& m_dsl;

  public:
    root_automaton_record (const generic_automaton_handle& automaton,
			   DSL& dsl) :
      automaton_record_interface (automaton),
      m_dsl (dsl)
    { }

    ~root_automaton_record () {
      m_dsl.automaton_destroyed (m_automaton);
    }

  };

  template <class DSL>
  class automaton_record :
    public automaton_record_interface
  {
  private:
    generic_automaton_handle m_parent;
    DSL& m_dsl;

  public:
    automaton_record (const generic_automaton_handle& parent,
		      const generic_automaton_handle& automaton,
		      DSL& dsl) :
      automaton_record_interface (automaton),
      m_parent (parent),
      m_dsl (dsl)
    { }
    
    ~automaton_record () {
      m_dsl.automaton_destroyed (m_parent, m_automaton);
    }

  };

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
      
      bool operator() (const std::pair<automaton_record_interface* const, serial_type>& automaton) const {
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
    std::map<generic_automaton_handle, automaton_record_interface*> m_automata;
    std::list<std::pair<generic_automaton_handle, generic_automaton_handle> > m_parent_child;
    std::list<binding_interface*> m_bindings;
    
  public:
    
    void clear (void) {
      m_instances.clear ();
      for (std::map<generic_automaton_handle, automaton_record_interface*>::const_iterator pos = m_automata.begin ();
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

    /*
      Generative operations (create, declare, bind) take a listener that is sent a message with
      the appropriate success/failure status of the generative action.
      Generative operations also take a listener for successes of the corresponding destructive
      operations (destroy, rescind, unbind).
      Destructive operations take a listener for failures.

      Codex
      CSFL = Create Success/Failure Listener
      DSL = Destroy Success Listener
      DFL = Destroy Failure Listener
    */
    
    template <class CSFL, class DSL>
    void
    create (automaton_interface* instance,
	    CSFL& csfl,
	    DSL& dsl)
    {
      BOOST_ASSERT (instance != 0);

      boost::unique_lock<boost::shared_mutex> lock (m_mutex);
      
      if (m_instances.contains (instance)) {
	csfl.instance_exists (instance);
	return;
      }

      generic_automaton_handle handle = m_instances.insert (instance);      
      automaton_record_interface* record = new root_automaton_record<DSL> (handle, dsl);
      m_automata.insert (std::make_pair (handle, record));

      csfl.automaton_created (handle);
      return;
    }

    template <class CSFL, class DSL>
    void
    create (const generic_automaton_handle& automaton,
    	    automaton_interface* instance,
	    CSFL& csfl,
	    DSL& dsl)
    {
      BOOST_ASSERT (instance != 0);
    
      boost::unique_lock<boost::shared_mutex> lock (m_mutex);
    
      if (!m_instances.contains (automaton)) {
	delete instance;
	csfl.automaton_dne (automaton, instance);
	return;
      }

      if (m_instances.contains (instance)) {
	csfl.instance_exists (automaton, instance);
	return;
      }

      generic_automaton_handle handle = m_instances.insert (instance);      
      automaton_record_interface* record = new automaton_record<DSL> (automaton, handle, dsl);
      m_automata.insert (std::make_pair (handle, record));
      m_parent_child.push_back (std::make_pair (automaton, handle));

      csfl.automaton_created (automaton, handle);
      return;
    }
    
    template <class DSFL, class RSL>
    void
    declare (const generic_automaton_handle& automaton,
  	     void* parameter,
	     DSFL& dsfl,
	     RSL& rsl)
    {
      boost::unique_lock<boost::shared_mutex> lock (m_mutex);
      
      if (!m_instances.contains (automaton)) {
	dsfl.automaton_dne (automaton, parameter);
  	return;
      }
      
      automaton_record_interface* record = m_automata[automaton];
      
      if (record->parameter_exists (parameter)) {
	dsfl.parameter_exists (automaton, record->parameter_handle (parameter));
	return;
      }
      
      dsfl.parameter_declared (automaton, record->declare_parameter (parameter, rsl));
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

    template <class OM, class IM, class BSFL, class USL>
    void
    _bind (const action<OM>& output,
	   const action<IM>& input,
	   const generic_automaton_handle& binder,
	   BSFL& bsfl,
	   USL& usl)
    {
      if (!m_instances.contains (binder)) {
  	// Binder DNE.
	bsfl.automaton_dne (output, input, binder);
  	return;
      }

      if (!m_instances.contains (output.get_automaton_handle ())) {
	bsfl.output_automaton_dne (output, input, binder);
	return;
      }

      if (!m_instances.contains (input.get_automaton_handle ())) {
	bsfl.input_automaton_dne (output, input, binder);
	return;
      }
      
      if (!parameter_exists (output)) {
	bsfl.output_parameter_dne (output, input, binder);
	return;
      }
      
      if (!parameter_exists (input)) {
	bsfl.input_parameter_dne (output, input, binder);
	return;
      }
      
      std::list<binding_interface*>::const_iterator pos = std::find_if (m_bindings.begin (),
									m_bindings.end (),
									binding_equal (output, input, binder));
      
      if (pos != m_bindings.end ()) {
  	// Bound.
	bsfl.binding_exists (output, input, binder);
	return;
      }
      
      std::list<binding_interface*>::const_iterator in_pos = std::find_if (m_bindings.begin (),
									   m_bindings.end (),
									   binding_input_equal (input));
      
      if (in_pos != m_bindings.end ()) {
  	// Input unavailable.
	bsfl.input_action_unavailable (output, input, binder);
	return;
      }
      
      std::list<binding_interface*>::const_iterator out_pos = std::find_if (m_bindings.begin (),
									    m_bindings.end (),
									    binding_output_equal (output));
      
      if (output.get_automaton_handle () == input.get_automaton_handle () ||
  	  (out_pos != m_bindings.end () && (*out_pos)->involves_input_automaton (input.get_automaton_handle ()))) {
  	// Output unavailable.
	bsfl.output_action_unavailable (output, input, binder);
	return;
      }
      
      binding<OM>* c;
      
      if (out_pos != m_bindings.end ()) {
	c = static_cast<binding<OM>*> (*out_pos);
      }
      else {
	c = new binding<OM> ();
	m_bindings.push_front (c);
      }
    
      // Bind.
      c->bind (output, input, binder, usl);

      bsfl.bound (output, input, binder);
      return;
    }

    template <class OM, class IM, class BSFL, class USL>
    void
    bind (const action<OM>& output,
	  output_category /* */,
	  unparameterized /* */,
	  const action<IM>& input,
	  input_category /* */,
	  unparameterized /* */,
	  const generic_automaton_handle& binder,
	  BSFL& bsfl,
	  USL& usl)
    {
      return _bind (output, input, binder, bsfl, usl);
    }    

    template <class OM, class IM, class BSFL, class USL>
    void
    bind (const action<OM>& output,
	  output_category /* */,
	  parameterized /* */,
	  const action<IM>& input,
	  input_category /* */,
	  unparameterized /* */,
	  BSFL& bsfl,
	  USL& usl)
    {
      return _bind (output, input, output.get_automaton_handle (), bsfl, usl);
    }    

    template <class OM, class IM, class BSFL, class USL>
    void
    bind (const action<OM>& output,
	  output_category /* */,
	  unparameterized /* */,
	  const action<IM>& input,
	  input_category /* */,
	  parameterized /* */,
	  BSFL& bsfl,
	  USL& usl)
    {
      return _bind (output, input, input.get_automaton_handle (), bsfl, usl);
    }    

  public:
    template <class OM, class IM, class BSFL, class USL>
    void
    bind (const action<OM>& output,
	  const action<IM>& input,
	  const generic_automaton_handle& binder,
	  BSFL& bsfl,
	  USL& usl)
    {
      boost::unique_lock<boost::shared_mutex> lock (m_mutex);
      
      return bind (output,
		   typename action<OM>::action_category (),
		   typename action<OM>::parameter_status (),
		   input,
		   typename action<IM>::action_category (),
		   typename action<IM>::parameter_status (),
		   binder,
		   bsfl,
		   usl);
    }

    template <class OM, class IM, class BSFL, class USL>
    void
    bind (const action<OM>& output,
	  const action<IM>& input,
	  BSFL& bsfl,
	  USL& usl)
    {
      boost::unique_lock<boost::shared_mutex> lock (m_mutex);

      return bind (output,
		   typename action<OM>::action_category (),
		   typename action<OM>::parameter_status (),
		   input,
		   typename action<IM>::action_category (),
		   typename action<IM>::parameter_status (),
		   bsfl,
		   usl);
    }
  
  private:
    template <class OM, class IM, class UFL>
    void
    _unbind (const action<OM>& output,
	     const action<IM>& input,
	     const generic_automaton_handle& binder,
	     UFL& ufl)
    {
      if (!m_instances.contains (binder)) {
	ufl.automaton_dne (output, input, binder);
	return;
      }

      if (!m_instances.contains (output.get_automaton_handle ())) {
	ufl.output_automaton_dne (output, input, binder);
	return;
      }

      if (!m_instances.contains (input.get_automaton_handle ())) {
	ufl.input_automaton_dne (output, input, binder);
	return;
      }
      
      if (!parameter_exists (output)) {
	ufl.output_parameter_dne (output, input, binder);
	return;
      }
      
      if (!parameter_exists (input)) {
	ufl.input_parameter_dne (output, input, binder);
	return;
      }
      
      std::list<binding_interface*>::iterator pos = std::find_if (m_bindings.begin (),
								  m_bindings.end (),
								  binding_equal (output, input, binder));
      
      if (pos == m_bindings.end ()) {
	// Not bound.
	ufl.binding_dne (output, input, binder);
	return;
      }
      
      binding<OM>* c = static_cast<binding<OM>*> (*pos);
      
      // Unbind.
      c->unbind (output, input, binder);
      
      if (c->empty ()) {
	delete c;
	m_bindings.erase (pos);
      }

      return;
    }

    template <class OM, class IM, class UFL>
    void
    unbind (const action<OM>& output,
	    unparameterized /* */,
	    const action<IM>& input,
	    unparameterized /* */,
	    const generic_automaton_handle& binder,
	    UFL& ufl)
    {
      return _unbind (output, input, binder, ufl);
    }    
    
    template <class OM, class IM, class UFL>
    void
    unbind (const action<OM>& output,
	    parameterized /* */,
	    const action<IM>& input,
	    unparameterized /* */,
	    UFL& ufl)
    {
      return _unbind (output, input, output.get_automaton_handle (), ufl);
    }    
    
    template <class OM, class IM, class UFL>
    void
    unbind (const action<OM>& output,
	    unparameterized /* */,
	    const action<IM>& input,
	    parameterized /* */,
	    UFL& ufl)
    {
      return _unbind (output, input, input.get_automaton_handle (), ufl);
    }    
    
  public:
    template <class OM, class IM, class UFL>
    void
    unbind (const action<OM>& output,
	    const action<IM>& input,
	    const generic_automaton_handle& binder,
	    UFL& ufl)
    {
      boost::unique_lock<boost::shared_mutex> lock (m_mutex);

      return unbind (output, typename action<OM>::parameter_status (), input, typename action<IM>::parameter_status (), binder, ufl);
    }

    template <class OM, class IM, class UFL>
    void
    unbind (const action<OM>& output,
	    const action<IM>& input,
	    UFL& ufl)
    {
      boost::unique_lock<boost::shared_mutex> lock (m_mutex);

      return unbind (output, typename action<OM>::parameter_status (), input, typename action<IM>::parameter_status (), ufl);
    }

    template <class RFL>
    void
    rescind (const generic_automaton_handle& automaton,
  	     const generic_parameter_handle& parameter,
	     RFL& rfl)
    {
      boost::unique_lock<boost::shared_mutex> lock (m_mutex);
      
      if (!m_instances.contains (automaton)) {
	rfl.automaton_dne (automaton, parameter);
  	return;
      }
      
      automaton_record_interface* record = m_automata[automaton];
      
      if (!record->parameter_exists (parameter)) {
	rfl.parameter_dne (automaton, parameter);
	return;
      }
      
      record->rescind_parameter (parameter);
      
      for (std::list<binding_interface*>::iterator pos = m_bindings.begin ();
	   pos != m_bindings.end ();
	   ) {
	(*pos)->unbind_parameter (automaton, parameter);
	if ((*pos)->empty ()) {
	  delete *pos;
	  pos = m_bindings.erase (pos);
	}
	else {
	  ++pos;
	}
      }
      
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
    inner_destroy (const generic_automaton_handle& automaton)
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

	{
	  // Update parent-child relationships.
	  std::list<std::pair<generic_automaton_handle, generic_automaton_handle> >::iterator pos = std::find_if (m_parent_child.begin (), m_parent_child.end (), child_automaton (handle));
	  if (pos != m_parent_child.end ()) {
	    m_parent_child.erase (pos);
	  }
	}
	
	{
	  // Update bindings.
	  for (std::list<binding_interface*>::iterator pos = m_bindings.begin ();
	       pos != m_bindings.end ();
	       ) {
	    (*pos)->unbind_automaton (handle);
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

    template <class DFL>
    void
    destroy (const generic_automaton_handle& target,
	     DFL& dfl)
    {
      boost::unique_lock<boost::shared_mutex> lock (m_mutex);
      
      if (!m_instances.contains (target)) {
	dfl.target_automaton_dne (target);
	return;
      }

      return inner_destroy (target);
    }

    template <class DFL>
    void
    destroy (const generic_automaton_handle& automaton,
	     const generic_automaton_handle& target,
	     DFL& dfl)
    {
      boost::unique_lock<boost::shared_mutex> lock (m_mutex);

      if (!m_instances.contains (automaton)) {
	dfl.automaton_dne (automaton, target);
	return;
      }

      if (!m_instances.contains (target)) {
	dfl.target_automaton_dne (automaton, target);
	return;
      }

      if (std::find (m_parent_child.begin (),
		     m_parent_child.end (),
		     std::make_pair (automaton, target)) == m_parent_child.end ()) {
	dfl.destroyer_not_creator (automaton, target);
	return;
      }

      return inner_destroy (target);
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
    // TODO:  Execute failure listeners.

    template <class L>
    void
    execute (const output_action_interface& ac,
	     scheduler_interface& scheduler,
	     L& listener)
    {
      boost::shared_lock<boost::shared_mutex> lock (m_mutex);

      if (!m_instances.contains (ac.get_automaton_handle ())) {
	listener.automaton_dne ();
	return;
      }

      if (!parameter_exists (ac)) {
	listener.parameter_dne ();
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

    template <class L>
    void
    execute (const independent_action_interface& ac,
	     scheduler_interface& scheduler,
	     L& listener)
    {
      boost::shared_lock<boost::shared_mutex> lock (m_mutex);
      
      if (!m_instances.contains (ac.get_automaton_handle ())) {
	listener.automaton_dne ();
	return;
      }

      if (!parameter_exists (ac)) {
	listener.parameter_dne ();
	return;
      }
      
      execute_executable (ac, scheduler);
    }

    template <class L>
    void
    execute (const generic_automaton_handle& automaton,
	     runnable_interface& c,
	     scheduler_interface& scheduler,
	     L& listener) {
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

    template <class L>
    void
    bound (const output_action_interface& ac,
	   scheduler_interface& scheduler,
	   L& listener)
    {
      boost::shared_lock<boost::shared_mutex> lock (m_mutex);
      
      if (!m_instances.contains (ac.get_automaton_handle ())) {
	listener.automaton_dne ();
	return;
      }

      if (!parameter_exists (ac)) {
	listener.parameter_dne ();
	return;
      }

      lock_automaton (ac.get_automaton_handle ());
      scheduler.set_current_handle (ac.get_automaton_handle ());
      ac.bound ();
      scheduler.clear_current_handle ();
      unlock_automaton (ac.get_automaton_handle ());
    }

    template <class L>
    void
    bound (const input_action_interface& ac,
	   scheduler_interface& scheduler,
	   L& listener)
    {
      boost::shared_lock<boost::shared_mutex> lock (m_mutex);
      
      if (!m_instances.contains (ac.get_automaton_handle ())) {
	listener.automaton_dne ();
	return;
      }

      if (!parameter_exists (ac)) {
	listener.parameter_dne ();
	return;
      }

      lock_automaton (ac.get_automaton_handle ());
      scheduler.set_current_handle (ac.get_automaton_handle ());
      ac.bound ();
      scheduler.clear_current_handle ();
      unlock_automaton (ac.get_automaton_handle ());
    }

    template <class L>
    void
    unbound (const output_action_interface& ac,
	     scheduler_interface& scheduler,
	     L& listener)
    {
      boost::shared_lock<boost::shared_mutex> lock (m_mutex);
      
      if (!m_instances.contains (ac.get_automaton_handle ())) {
	listener.automaton_dne ();
	return;
      }
      
      if (!parameter_exists (ac)) {
	listener.parameter_dne ();
	return;
      }
      
      lock_automaton (ac.get_automaton_handle ());
      scheduler.set_current_handle (ac.get_automaton_handle ());
      ac.unbound ();
      scheduler.clear_current_handle ();
      unlock_automaton (ac.get_automaton_handle ());
    }

    template <class L>
    void
    unbound (const input_action_interface& ac,
	     scheduler_interface& scheduler,
	     L& listener)
    {
      boost::shared_lock<boost::shared_mutex> lock (m_mutex);
      
      if (!m_instances.contains (ac.get_automaton_handle ())) {
	listener.automaton_dne ();
	return;
      }
      
      if (!parameter_exists (ac)) {
	listener.parameter_dne ();
	return;
      }
      
      lock_automaton (ac.get_automaton_handle ());
      scheduler.set_current_handle (ac.get_automaton_handle ());
      ac.unbound ();
      scheduler.clear_current_handle ();
      unlock_automaton (ac.get_automaton_handle ());
    }

  };

}

#endif
