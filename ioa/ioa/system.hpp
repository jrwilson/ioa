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

  template <class T>
  class sequential_set
  {
  private:
    T m_counter;
    std::set<T> m_used;
  public:
    sequential_set () :
      m_counter (0)
      { }

    T take () {
      do {
	++m_counter;
	if (m_counter < 0) {
	  m_counter = 0;
	}
      } while (m_used.count (m_counter) != 0);
      m_used.insert (m_counter);
      return m_counter;
    }

    void replace (const T& t) {
      m_used.erase (t);
    }

    bool contains (const T& t) const {
      return m_used.count (t) != 0;
    }

    void clear () {
      m_used.clear ();
    }
  };

  class parameter_record_interface
  {
  public:
    virtual ~parameter_record_interface () { }
    virtual const aid_t get_aid () const = 0;
    virtual const pid_t get_pid () const = 0;
    virtual void* get_parameter () const = 0;
  };

  template <class I, class P, class RSL, class D>
  class parameter_record :
    public parameter_record_interface
  {
  private:
    automaton_handle<I> m_automaton;
    parameter_handle<P> m_parameter;
    P* m_param;
    RSL& m_rsl;
    D& m_d;

  public:
    parameter_record (const automaton_handle<I>& automaton,
		      const parameter_handle<P>& parameter,
		      P* param,
		      RSL& rsl,
		      D& d) :
      m_automaton (automaton),
      m_parameter (parameter),
      m_param (param),
      m_rsl (rsl),
      m_d (d)
    { }

    ~parameter_record () {
      m_rsl.parameter_rescinded (m_automaton, m_d);
    }

    const aid_t get_aid () const {
      return m_automaton.aid;
    }
    
    const pid_t get_pid () const {
      return m_parameter.pid;
    }

    void* get_parameter () const {
      return m_param;
    }

  };

  class automaton_record_interface :
    public boost::mutex
  {
  private:
    sequential_set<pid_t> m_pids;
    std::map<void*, parameter_record_interface*> m_param_to_record;
    std::map<pid_t, parameter_record_interface*> m_pid_to_record;

  public:
    virtual ~automaton_record_interface () {
      for (std::map<void*, parameter_record_interface*>::const_iterator pos = m_param_to_record.begin ();
	   pos != m_param_to_record.end ();
	   ++pos) {
	delete pos->second;
      }
    }

    virtual const aid_t get_handle () const = 0;
    virtual void* get_instance () const = 0;

    bool parameter_exists (void* parameter) const {
      return m_param_to_record.count (parameter) != 0;
    }

    template <class P>
    bool parameter_exists (const parameter_handle<P>& parameter) const {
      return m_pid_to_record.count (parameter.pid) != 0;
    }

    template <class P>
    P* get_parameter (const parameter_handle<P>& parameter) const {
      std::map<pid_t, parameter_record_interface*>::const_iterator pos = m_pid_to_record.find (parameter.pid);
      BOOST_ASSERT (pos != m_pid_to_record.end ());
      return static_cast<P*> (pos->second->get_parameter ());
    }

    template <class P>
    parameter_handle<P> declare_parameter (P* parameter) {
      BOOST_ASSERT (!parameter_exists (parameter));
      // NB: Casting.
      return parameter_handle<P> (m_pids.take ());
    }

    void insert_parameter_record (parameter_record_interface* record) {
      BOOST_ASSERT (get_handle () == record->get_aid ());
      pid_t pid = record->get_pid ();
      BOOST_ASSERT (m_pids.contains (pid));
      m_param_to_record.insert (std::make_pair (record->get_parameter (), record));
      m_pid_to_record.insert (std::make_pair (pid, record));
    }

    template <class P>
    void rescind_parameter (const parameter_handle<P>& parameter) {
      BOOST_ASSERT (parameter_exists (parameter));
      parameter_record_interface* record = m_pid_to_record[parameter.pid];
      m_pid_to_record.erase (parameter.pid);
      m_param_to_record.erase (record->get_parameter ());
      delete record;
    }
  };

  template <class I>
  class automaton_record_impl :
    public automaton_record_interface
  {
  private:
    std::auto_ptr<I> m_instance;

  protected:
    automaton_handle<I> m_automaton;
    
  public:
    automaton_record_impl (I* instance,
			   const automaton_handle<I>& automaton) :
      m_instance (instance),
      m_automaton (automaton)
    { }

    virtual ~automaton_record_impl () { }

    void* get_instance () const {
      return m_instance.get ();
    }

    const aid_t get_handle () const {
      return m_automaton.aid;
    }
  };

  template <class I, class DSL>
  class root_automaton_record :
    public automaton_record_impl<I>
  {
  private:
    DSL& m_dsl;

  public:
    root_automaton_record (I* instance,
			   const automaton_handle<I>& automaton,
			   DSL& dsl) :
      automaton_record_impl<I> (instance, automaton),
      m_dsl (dsl)
    { }

    ~root_automaton_record () {
      m_dsl.automaton_destroyed (this->m_automaton);
    }

  };

  template <class P, class I, class DSL, class D>
  class automaton_record :
    public automaton_record_impl<I>
  {
  private:
    automaton_handle<P> m_parent;
    DSL& m_dsl;
    D& m_d;

  public:
    automaton_record (const automaton_handle<P>& parent,
		      I* instance,
		      const automaton_handle<I>& automaton,
		      DSL& dsl,
		      D& d) :
      automaton_record_impl<I> (instance, automaton),
      m_parent (parent),
      m_dsl (dsl),
      m_d (d)
    { }
    
    ~automaton_record () {
      m_dsl.automaton_destroyed (m_parent, m_d);
    }

  };

  class system :
    public system_interface,
    public boost::noncopyable
  {
  private:    
    
    class binding_equal
    {
    private:
      const output_action_interface& m_output;
      const input_action_interface& m_input;
      const aid_t m_binder;

    public:
      binding_equal (const output_action_interface& output,
		     const input_action_interface& input,
		     const aid_t binder) :
  	m_output (output),
  	m_input (input),
	m_binder (binder)
      { }
      
      bool operator() (const binding_interface* c) const {
  	return c->involves_binding (m_output, m_input, m_binder);
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
    sequential_set<aid_t> m_aids;
    std::set<void*> m_instances;
    std::map<aid_t, automaton_record_interface*> m_records;
    std::list<std::pair<aid_t, aid_t> > m_parent_child;
    std::list<binding_interface*> m_bindings;
    
  public:
    
    void clear (void) {
      m_aids.clear ();
      m_instances.clear ();
      for (std::map<aid_t, automaton_record_interface*>::const_iterator pos = m_records.begin ();
	   pos != m_records.end ();
	   ++pos) {
	delete pos->second;
      }
      m_records.clear ();
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
    
    template <class I, class CSFL, class DSL>
    void
    create (I* instance,
	    CSFL& csfl,
	    DSL& dsl)
    {
      BOOST_ASSERT (instance != 0);

      boost::unique_lock<boost::shared_mutex> lock (m_mutex);
      
      if (m_instances.count (instance) != 0) {
	csfl.instance_exists (instance);
	return;
      }

      // TODO:  Generate instance, set current aid.

      // NB: Casting.
      automaton_handle<I> handle (m_aids.take ());
      m_instances.insert (instance);
      automaton_record_interface* record = new root_automaton_record<I, DSL> (instance, handle, dsl);
      m_records.insert (std::make_pair (handle.aid, record));

      csfl.automaton_created (handle);
      return;
    }

    template <class P, class G, class CSFL, class DSL, class D>
    void
    create (const automaton_handle<P>& automaton,
    	    G generator,
	    CSFL& csfl,
	    DSL& dsl,
	    D& d)
    {
      boost::unique_lock<boost::shared_mutex> lock (m_mutex);
      
      if (!m_aids.contains (automaton.aid)) {
	csfl.automaton_dne (automaton, d);
	return;
      }
      
      typename G::result_type* instance = generator ();
      BOOST_ASSERT (instance != 0);

      if (m_instances.count (instance) != 0) {
	csfl.instance_exists (automaton, instance, d);
	return;
      }

      // TODO:  Generate instance, set current aid.

      // NB: Casting.
      automaton_handle<typename G::result_type> handle (m_aids.take ());
      m_instances.insert (instance);      
      automaton_record_interface* record = new automaton_record<P, typename G::result_type, DSL, D> (automaton, instance, handle, dsl, d);
      m_records.insert (std::make_pair (handle.aid, record));
      m_parent_child.push_back (std::make_pair (automaton.aid, handle.aid));
      
      csfl.automaton_created (automaton, handle, d);
      return;
    }
    
    template <class I, class P, class DSFL, class RSL, class D>
    void
    declare (const automaton_handle<I>& automaton,
  	     P* parameter,
	     DSFL& dsfl,
	     RSL& rsl,
	     D& d)
    {
      boost::unique_lock<boost::shared_mutex> lock (m_mutex);
      
      if (!m_aids.contains (automaton.aid)) {
	dsfl.automaton_dne (automaton, parameter, d);
  	return;
      }
      
      automaton_record_interface* record = m_records[automaton.aid];
      
      if (record->parameter_exists (parameter)) {
	dsfl.parameter_exists (automaton, d);
	return;
      }

      parameter_handle<P> handle = record->declare_parameter (parameter);
      parameter_record_interface* p_record = new parameter_record<I, P, RSL, D> (automaton, handle, parameter, rsl, d);
      record->insert_parameter_record (p_record);

      dsfl.parameter_declared (automaton, handle, d);
      return;
    }
    
  private:
    template <class I, class M>
    bool parameter_exists (const action<I, M>& ac, unparameterized /* */) {
      return true;
    }

    template <class I, class M>
    bool parameter_exists (const action<I, M>& ac, parameterized /* */) {
      return m_records[ac.automaton.aid]->parameter_exists (ac.parameter);
    }

    template <class I, class M>
    concrete_action<I, M> reify0 (const action<I, M>& ac,
				  unparameterized /* */) {
      I* instance = static_cast<I*> (m_records[ac.automaton.aid]->get_instance ());
      return concrete_action<I, M> (ac, instance);
    }

    template <class I, class M>
    concrete_action<I, M> reify0 (const action<I, M>& ac,
				  parameterized /* */) {
      I* instance = static_cast<I*> (m_records[ac.automaton.aid]->get_instance ());
      typename action<I,M>::parameter_type* parameter = m_records[ac.automaton.aid]->get_parameter (ac.parameter);

      return concrete_action<I, M> (ac, instance, parameter);
    }

    template <class I, class M>
    concrete_action<I, M> reify (const action<I, M>& ac) {
      return reify0 (ac, typename action<I, M>::parameter_status ());
    }

    template <class OI, class OM, class II, class IM, class I, class BSFL, class USL, class D>
    void
    _bind (const action<OI, OM>& output,
	   const action<II, IM>& input,
	   const automaton_handle<I>& binder,
	   BSFL& bsfl,
	   USL& usl,
	   D& d)
    {
      if (!m_aids.contains (binder.aid)) {
  	// Binder DNE.
	bsfl.automaton_dne (output, input, binder);
  	return;
      }

      if (!m_aids.contains (output.automaton.aid)) {
	bsfl.output_automaton_dne (output, input, binder, d);
	return;
      }

      if (!m_aids.contains (input.automaton.aid)) {
	bsfl.input_automaton_dne (output, input, binder, d);
	return;
      }
      
      if (!parameter_exists (output, typename action<OI, OM>::parameter_status ())) {
	bsfl.output_parameter_dne (output, input, binder, d);
	return;
      }
      
      if (!parameter_exists (input, typename action<II, IM>::parameter_status ())) {
	bsfl.input_parameter_dne (output, input, binder, d);
	return;
      }
      
      concrete_action<OI, OM> c_output = reify (output);
      concrete_action<II, IM> c_input = reify (input);

      std::list<binding_interface*>::const_iterator pos = std::find_if (m_bindings.begin (),
									m_bindings.end (),
									binding_equal (c_output, c_input, binder.aid));
      
      if (pos != m_bindings.end ()) {
  	// Bound.
	bsfl.binding_exists (output, input, binder, d);
	return;
      }
      
      std::list<binding_interface*>::const_iterator in_pos = std::find_if (m_bindings.begin (),
									   m_bindings.end (),
									   binding_input_equal (c_input));
      
      if (in_pos != m_bindings.end ()) {
  	// Input unavailable.
	bsfl.input_action_unavailable (output, input, binder, d);
	return;
      }
      
      std::list<binding_interface*>::const_iterator out_pos = std::find_if (m_bindings.begin (),
									    m_bindings.end (),
									    binding_output_equal (c_output));
      
      if (output.automaton.aid == input.automaton.aid ||
  	  (out_pos != m_bindings.end () && (*out_pos)->involves_input_automaton (input.automaton.aid))) {
  	// Output unavailable.
	bsfl.output_action_unavailable (output, input, binder, d);
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
      c->bind (c_output, c_input, binder, usl, d);

      bsfl.bound (output, input, binder, d);
      return;
    }

    template <class OI, class OM, class II, class IM, class I, class BSFL, class USL, class D>
    void
    bind (const action<OI, OM>& output,
	  output_category /* */,
	  unparameterized /* */,
	  const action<II, IM>& input,
	  input_category /* */,
	  unparameterized /* */,
	  const automaton_handle<I>& binder,
	  BSFL& bsfl,
	  USL& usl,
	  D& d)
    {
      return _bind (output, input, binder, bsfl, usl, d);
    }    

    template <class OI, class OM, class II, class IM, class BSFL, class USL, class D>
    void
    bind (const action<OI, OM>& output,
	  output_category /* */,
	  parameterized /* */,
	  const action<II, IM>& input,
	  input_category /* */,
	  unparameterized /* */,
	  BSFL& bsfl,
	  USL& usl,
	  D& d)
    {
      return _bind (output, input, output.automaton, bsfl, usl, d);
    }    

    template <class OI, class OM, class II, class IM, class BSFL, class USL, class D>
    void
    bind (const action<OI, OM>& output,
	  output_category /* */,
	  unparameterized /* */,
	  const action<II, IM>& input,
	  input_category /* */,
	  parameterized /* */,
	  BSFL& bsfl,
	  USL& usl,
	  D& d)
    {
      return _bind (output, input, input.automaton, bsfl, usl, d);
    }    

  public:
    template <class OI, class OM, class II, class IM, class I, class BSFL, class USL, class D>
    void
    bind (const action<OI, OM>& output,
	  const action<II, IM>& input,
	  const automaton_handle<I>& binder,
	  BSFL& bsfl,
	  USL& usl,
	  D& d)
    {
      boost::unique_lock<boost::shared_mutex> lock (m_mutex);
      
      return bind (output,
		   typename action<OI, OM>::action_category (),
		   typename action<OI, OM>::parameter_status (),
		   input,
		   typename action<II, IM>::action_category (),
		   typename action<II, IM>::parameter_status (),
		   binder,
		   bsfl,
		   usl,
		   d);
    }

    template <class OI, class OM, class II, class IM, class BSFL, class USL, class D>
    void
    bind (const action<OI, OM>& output,
	  const action<II, IM>& input,
	  BSFL& bsfl,
	  USL& usl,
	  D& d)
    {
      boost::unique_lock<boost::shared_mutex> lock (m_mutex);

      return bind (output,
		   typename action<OI, OM>::action_category (),
		   typename action<OI, OM>::parameter_status (),
		   input,
		   typename action<II, IM>::action_category (),
		   typename action<II, IM>::parameter_status (),
		   bsfl,
		   usl,
		   d);
    }
  
  private:
    template <class OI, class OM, class II, class IM, class I, class UFL, class D>
    void
    _unbind (const action<OI, OM>& output,
	     const action<II, IM>& input,
	     const automaton_handle<I>& binder,
	     UFL& ufl,
	     D& d)
    {
      if (!m_aids.contains (binder.aid)) {
	ufl.automaton_dne (output, input, binder);
	return;
      }

      if (!m_aids.contains (output.automaton.aid)) {
	ufl.output_automaton_dne (output, input, binder, d);
	return;
      }

      if (!m_aids.contains (input.automaton.aid)) {
	ufl.input_automaton_dne (output, input, binder, d);
	return;
      }
      
      if (!parameter_exists (output, typename action<OI, OM>::parameter_status ())) {
	ufl.output_parameter_dne (output, input, binder, d);
	return;
      }
      
      if (!parameter_exists (input, typename action<II, IM>::parameter_status ())) {
	ufl.input_parameter_dne (output, input, binder, d);
	return;
      }

      concrete_action<OI, OM> c_output = reify (output);
      concrete_action<II, IM> c_input = reify (input);
      
      std::list<binding_interface*>::iterator pos = std::find_if (m_bindings.begin (),
								  m_bindings.end (),
								  binding_equal (c_output, c_input, binder.aid));
      
      if (pos == m_bindings.end ()) {
	// Not bound.
	ufl.binding_dne (output, input, binder, d);
	return;
      }
      
      binding<OM>* c = static_cast<binding<OM>*> (*pos);
      
      // Unbind.
      c->unbind (c_output, c_input, binder);
      
      if (c->empty ()) {
	delete c;
	m_bindings.erase (pos);
      }

      return;
    }

    template <class OI, class OM, class II, class IM, class I, class UFL, class D>
    void
    unbind (const action<OI, OM>& output,
	    unparameterized /* */,
	    const action<II, IM>& input,
	    unparameterized /* */,
	    const automaton_handle<I>& binder,
	    UFL& ufl,
	    D& d)
    {
      return _unbind (output, input, binder, ufl, d);
    }    
    
    template <class OI, class OM, class II, class IM, class UFL, class D>
    void
    unbind (const action<OI, OM>& output,
	    parameterized /* */,
	    const action<II, IM>& input,
	    unparameterized /* */,
	    UFL& ufl,
	    D& d)
    {
      return _unbind (output, input, output.automaton, ufl, d);
    }    
    
    template <class OI, class OM, class II, class IM, class UFL, class D>
    void
    unbind (const action<OI, OM>& output,
	    unparameterized /* */,
	    const action<II, IM>& input,
	    parameterized /* */,
	    UFL& ufl,
	    D& d)
    {
      return _unbind (output, input, input.automaton, ufl, d);
    }    
    
  public:
    template <class OI, class OM, class II, class IM, class I, class UFL, class D>
    void
    unbind (const action<OI, OM>& output,
	    const action<II, IM>& input,
	    const automaton_handle<I>& binder,
	    UFL& ufl,
	    D& d)
    {
      boost::unique_lock<boost::shared_mutex> lock (m_mutex);

      return unbind (output, typename action<OI, OM>::parameter_status (), input, typename action<II, IM>::parameter_status (), binder, ufl, d);
    }

    template <class OI, class OM, class II, class IM, class UFL, class D>
    void
    unbind (const action<OI, OM>& output,
	    const action<II, IM>& input,
	    UFL& ufl,
	    D& d)
    {
      boost::unique_lock<boost::shared_mutex> lock (m_mutex);

      return unbind (output, typename action<OI, OM>::parameter_status (), input, typename action<II, IM>::parameter_status (), ufl, d);
    }

    template <class I, class P, class RFL, class D>
    void
    rescind (const automaton_handle<I>& automaton,
  	     const parameter_handle<P>& parameter,
	     RFL& rfl,
	     D& d)
    {
      boost::unique_lock<boost::shared_mutex> lock (m_mutex);
      
      if (!m_aids.contains (automaton.aid)) {
	rfl.automaton_dne (automaton, parameter);
  	return;
      }
      
      automaton_record_interface* record = m_records[automaton.aid];
      
      if (!record->parameter_exists (parameter)) {
	rfl.parameter_dne (automaton, d);
	return;
      }
      
      record->rescind_parameter (parameter);
      
      for (std::list<binding_interface*>::iterator pos = m_bindings.begin ();
	   pos != m_bindings.end ();
	   ) {
	(*pos)->unbind_parameter (automaton.aid, parameter.pid);
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
      aid_t m_handle;
    public:
      child_automaton (aid_t handle) :
	m_handle (handle) { }

      bool operator() (const std::pair<aid_t, aid_t>& p) const {
	return p.second == m_handle;
      }
    };

    template <class I>
    void
    inner_destroy (const automaton_handle<I>& automaton)
    {
      std::set<aid_t> closed_list;
      std::set<aid_t> open_list;

      // Put the target in the open list.
      open_list.insert (automaton.aid);

      while (!open_list.empty ()) {
	// Grab an item from the open list.
	std::set<aid_t>::iterator pos = open_list.begin ();
	const aid_t x = *pos;
	open_list.erase (pos);

	// If its not already in the closed list.
	if (closed_list.count (x) == 0) {
	  // Put it in the closed list.
	  closed_list.insert (x);
	  for (std::list<std::pair<aid_t, aid_t> >::const_iterator pos = m_parent_child.begin ();
	       pos != m_parent_child.end ();
	       ++pos) {
	    if (x == pos->first) {
	      // And put all of its children in the open list.
	      open_list.insert (pos->second);
	    }
	  }
	}
      }

      BOOST_FOREACH (aid_t handle, closed_list) {
	{
	  // Update the list of automata.
	  m_aids.replace (handle);
	  m_instances.erase (m_records[handle]->get_instance ());
	  delete m_records[handle];
	  m_records.erase (handle);
	}

	{
	  // Update parent-child relationships.
	  std::list<std::pair<aid_t, aid_t> >::iterator pos = std::find_if (m_parent_child.begin (), m_parent_child.end (), child_automaton (handle));
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

    template <class I, class DFL>
    void
    destroy (const automaton_handle<I>& target,
	     DFL& dfl)
    {
      boost::unique_lock<boost::shared_mutex> lock (m_mutex);
      
      if (!m_aids.contains (target.aid)) {
	dfl.target_automaton_dne (target);
	return;
      }

      return inner_destroy (target);
    }

    template <class P, class I, class DFL, class D>
    void
    destroy (const automaton_handle<P>& automaton,
	     const automaton_handle<I>& target,
	     DFL& dfl,
	     D& d)
    {
      boost::unique_lock<boost::shared_mutex> lock (m_mutex);

      if (!m_aids.contains (automaton.aid)) {
	dfl.automaton_dne (automaton, target, d);
	return;
      }

      if (!m_aids.contains (target.aid)) {
	dfl.target_automaton_dne (automaton, d);
	return;
      }

      if (std::find (m_parent_child.begin (),
		     m_parent_child.end (),
		     std::make_pair (automaton.aid, target.aid)) == m_parent_child.end ()) {
	dfl.destroyer_not_creator (automaton, d);
	return;
      }

      return inner_destroy (target);
    }

  private:
    void lock_automaton (const aid_t handle)
    {
      m_records[handle]->lock ();
    }

    void unlock_automaton (const aid_t handle)
    {
      m_records[handle]->unlock ();
    }

    template <class I, class M>
    void
    execute0 (const concrete_action<I, M>& ac,
	      scheduler_interface& scheduler)
    {
      lock_and_set (scheduler, ac.automaton.aid);
      ac.execute ();
      clear_and_unlock (scheduler, ac.automaton.aid);
    }

    // TODO:  Execute failure listeners.

    template <class I, class M, class L>
    void
    execute1 (const concrete_action<I, M>& ac,
	      output_category /* */,
	      scheduler_interface& scheduler,
	      L& listener)
    {      
      std::list<binding_interface*>::const_iterator out_pos = std::find_if (m_bindings.begin (),
									    m_bindings.end (),
									    binding_output_equal (ac));
      
      if (out_pos == m_bindings.end ()) {
	// Not bound.
	execute0 (ac, scheduler);
      }
      else {	
	(*out_pos)->execute (scheduler, *this);
      }
    }

    template <class I, class M, class L>
    void
    execute1 (const concrete_action<I, M>& ac,
	      internal_category /* */,
	      scheduler_interface& scheduler,
	      L& listener)
    {
      execute0 (ac, scheduler);
    }

    template <class I, class M, class L>
    void
    execute1 (const concrete_action<I, M>& ac,
	      event_category /* */,
	      scheduler_interface& scheduler,
	      L& listener)
    {
      execute0 (ac, scheduler);
    }

  public:
    
    template <class I, class M, class L>
    void
    execute (const action<I, M>& ac,
	     scheduler_interface& scheduler,
	     L& listener)
    {
      boost::shared_lock<boost::shared_mutex> lock (m_mutex);
      
      if (!m_aids.contains (ac.automaton.aid)) {
	listener.automaton_dne ();
	return;
      }

      if (!parameter_exists (ac, typename action<I, M>::parameter_status ())) {
	listener.parameter_dne ();
	return;
      }

      concrete_action<I, M> rac = reify (ac);
      execute1 (rac, typename concrete_action<I, M>::action_category (), scheduler, listener);
    }

    // TODO:  Test functions to follow.
    // TODO:  Remove duplicated code.

  private:

    void lock_and_set (scheduler_interface& scheduler,
		       const aid_t aid) {
      lock_automaton (aid);
      scheduler.set_current_aid (aid);
    }

    void clear_and_unlock (scheduler_interface& scheduler,
			   const aid_t aid) {
      scheduler.clear_current_aid ();
      unlock_automaton (aid);
    }

  public:

    template <class I, class T, class L, class D>
    void instance_exists (const automaton_handle<I>& automaton,
			  const T* i,
			  scheduler_interface& scheduler,
			  L& listener,
			  D& d)
    {
      boost::shared_lock<boost::shared_mutex> lock (m_mutex);
      
      if (!m_aids.contains (automaton.aid)) {
      	listener.automaton_dne ();
      	return;
      }

      lock_and_set (scheduler, automaton.aid);
      I* instance = static_cast<I*> (m_records[automaton.aid]->get_instance ());
      instance->instance_exists (i, d);
      clear_and_unlock (scheduler, automaton.aid);
    }

    template <class I, class T, class L, class D>
    void automaton_created (const automaton_handle<I>& automaton,
			    const automaton_handle<T>& child,
			    scheduler_interface& scheduler,
			    L& listener,
			    D& d)
    {
      boost::shared_lock<boost::shared_mutex> lock (m_mutex);
      
      if (!m_aids.contains (automaton.aid)) {
      	listener.automaton_dne ();
      	return;
      }

      lock_and_set (scheduler, automaton.aid);
      I* instance = static_cast<I*> (m_records[automaton.aid]->get_instance ());
      instance->automaton_created (child, d);
      clear_and_unlock (scheduler, automaton.aid);
    }

    template <class I, class L>
    void init (const automaton_handle<I>& automaton,
	       scheduler_interface& scheduler,
	       L& listener)
    {
      boost::shared_lock<boost::shared_mutex> lock (m_mutex);
      
      if (!m_aids.contains (automaton.aid)) {
      	listener.automaton_dne ();
      	return;
      }

      lock_and_set (scheduler, automaton.aid);
      I* instance = static_cast<I*> (m_records[automaton.aid]->get_instance ());
      instance->init ();
      clear_and_unlock (scheduler, automaton.aid);
    }

    template <class I, class L, class D>
    void parameter_exists (const automaton_handle<I>& automaton,
			   scheduler_interface& scheduler,
			   L& listener,
			   D& d)
    {
      boost::shared_lock<boost::shared_mutex> lock (m_mutex);
      
      if (!m_aids.contains (automaton.aid)) {
      	listener.automaton_dne ();
      	return;
      }

      lock_and_set (scheduler, automaton.aid);
      I* instance = static_cast<I*> (m_records[automaton.aid]->get_instance ());
      instance->parameter_exists (d);
      clear_and_unlock (scheduler, automaton.aid);
    }

    template <class I, class P, class L, class D>
    void parameter_declared (const automaton_handle<I>& automaton,
			     const parameter_handle<P>& parameter,
			     scheduler_interface& scheduler,
			     L& listener,
			     D& d)
    {
      boost::shared_lock<boost::shared_mutex> lock (m_mutex);
      
      if (!m_aids.contains (automaton.aid)) {
      	listener.automaton_dne ();
      	return;
      }

      lock_and_set (scheduler, automaton.aid);
      I* instance = static_cast<I*> (m_records[automaton.aid]->get_instance ());
      instance->parameter_declared (parameter, d);
      clear_and_unlock (scheduler, automaton.aid);
    }

    template <class I, class L, class D>
    void bind_output_automaton_dne (const automaton_handle<I>& automaton,
				    scheduler_interface& scheduler,
				    L& listener,
				    D& d)
    {
      boost::shared_lock<boost::shared_mutex> lock (m_mutex);
      
      if (!m_aids.contains (automaton.aid)) {
      	listener.automaton_dne ();
      	return;
      }

      lock_and_set (scheduler, automaton.aid);
      I* instance = static_cast<I*> (m_records[automaton.aid]->get_instance ());
      instance->bind_output_automaton_dne (d);
      clear_and_unlock (scheduler, automaton.aid);
    }

    template <class I, class L, class D>
    void bind_input_automaton_dne (const automaton_handle<I>& automaton,
				   scheduler_interface& scheduler,
				   L& listener,
				   D& d)
    {
      boost::shared_lock<boost::shared_mutex> lock (m_mutex);
      
      if (!m_aids.contains (automaton.aid)) {
      	listener.automaton_dne ();
      	return;
      }

      lock_and_set (scheduler, automaton.aid);
      I* instance = static_cast<I*> (m_records[automaton.aid]->get_instance ());
      instance->bind_input_automaton_dne (d);
      clear_and_unlock (scheduler, automaton.aid);
    }

    template <class I, class L, class D>
    void bind_output_parameter_dne (const automaton_handle<I>& automaton,
				    scheduler_interface& scheduler,
				    L& listener,
				    D& d)
    {
      boost::shared_lock<boost::shared_mutex> lock (m_mutex);
      
      if (!m_aids.contains (automaton.aid)) {
      	listener.automaton_dne ();
      	return;
      }

      lock_and_set (scheduler, automaton.aid);
      I* instance = static_cast<I*> (m_records[automaton.aid]->get_instance ());
      instance->bind_output_parameter_dne (d);
      clear_and_unlock (scheduler, automaton.aid);
    }

    template <class I, class L, class D>
    void bind_input_parameter_dne (const automaton_handle<I>& automaton,
				   scheduler_interface& scheduler,
				   L& listener,
				   D& d)
    {
      boost::shared_lock<boost::shared_mutex> lock (m_mutex);
      
      if (!m_aids.contains (automaton.aid)) {
      	listener.automaton_dne ();
      	return;
      }

      lock_and_set (scheduler, automaton.aid);
      I* instance = static_cast<I*> (m_records[automaton.aid]->get_instance ());
      instance->bind_input_parameter_dne (d);
      clear_and_unlock (scheduler, automaton.aid);
    }
    
    template <class I, class L, class D>
    void binding_exists (const automaton_handle<I>& automaton,
			 scheduler_interface& scheduler,
			 L& listener,
			 D& d)
    {
      boost::shared_lock<boost::shared_mutex> lock (m_mutex);
      
      if (!m_aids.contains (automaton.aid)) {
      	listener.automaton_dne ();
      	return;
      }

      lock_and_set (scheduler, automaton.aid);
      I* instance = static_cast<I*> (m_records[automaton.aid]->get_instance ());
      instance->binding_exists (d);
      clear_and_unlock (scheduler, automaton.aid);
    }

    template <class I, class L, class D>
    void input_action_unavailable (const automaton_handle<I>& automaton,
				   scheduler_interface& scheduler,
				   L& listener,
				   D& d)
    {
      boost::shared_lock<boost::shared_mutex> lock (m_mutex);
      
      if (!m_aids.contains (automaton.aid)) {
      	listener.automaton_dne ();
      	return;
      }

      lock_and_set (scheduler, automaton.aid);
      I* instance = static_cast<I*> (m_records[automaton.aid]->get_instance ());
      instance->input_action_unavailable (d);
      clear_and_unlock (scheduler, automaton.aid);
    }

    template <class I, class L, class D>
    void output_action_unavailable (const automaton_handle<I>& automaton,
				    scheduler_interface& scheduler,
				   L& listener,
				   D& d)
    {
      boost::shared_lock<boost::shared_mutex> lock (m_mutex);
      
      if (!m_aids.contains (automaton.aid)) {
      	listener.automaton_dne ();
      	return;
      }

      lock_and_set (scheduler, automaton.aid);
      I* instance = static_cast<I*> (m_records[automaton.aid]->get_instance ());
      instance->output_action_unavailable (d);
      clear_and_unlock (scheduler, automaton.aid);
    }

    template <class I, class L, class D>
    void bound (const automaton_handle<I>& automaton,
		scheduler_interface& scheduler,
		L& listener,
		D& d)
    {
      boost::shared_lock<boost::shared_mutex> lock (m_mutex);
      
      if (!m_aids.contains (automaton.aid)) {
      	listener.automaton_dne ();
      	return;
      }

      lock_and_set (scheduler, automaton.aid);
      I* instance = static_cast<I*> (m_records[automaton.aid]->get_instance ());
      instance->bound (d);
      clear_and_unlock (scheduler, automaton.aid);
    }

    template <class I, class M, class L>
    void
    action_bound (const action<I, M>& ac,
		  scheduler_interface& scheduler,
		  L& listener)
    {
      boost::shared_lock<boost::shared_mutex> lock (m_mutex);
      
      if (!m_aids.contains (ac.automaton.aid)) {
	listener.automaton_dne ();
	return;
      }

      if (!parameter_exists (ac, typename action<I, M>::parameter_status ())) {
	listener.parameter_dne ();
	return;
      }

      concrete_action<I, M> rac = reify (ac);

      lock_and_set (scheduler, rac.automaton.aid);
      rac.bound ();
      clear_and_unlock (scheduler, rac.automaton.aid);
    }

    template <class I, class L, class D>
    void unbind_output_automaton_dne (const automaton_handle<I>& automaton,
				      scheduler_interface& scheduler,
				      L& listener,
				      D& d)
    {
      boost::shared_lock<boost::shared_mutex> lock (m_mutex);
      
      if (!m_aids.contains (automaton.aid)) {
      	listener.automaton_dne ();
      	return;
      }

      lock_and_set (scheduler, automaton.aid);
      I* instance = static_cast<I*> (m_records[automaton.aid]->get_instance ());
      instance->unbind_output_automaton_dne (d);
      clear_and_unlock (scheduler, automaton.aid);
    }

    template <class I, class L, class D>
    void unbind_input_automaton_dne (const automaton_handle<I>& automaton,
				     scheduler_interface& scheduler,
				     L& listener,
				     D& d)
    {
      boost::shared_lock<boost::shared_mutex> lock (m_mutex);
      
      if (!m_aids.contains (automaton.aid)) {
      	listener.automaton_dne ();
      	return;
      }

      lock_and_set (scheduler, automaton.aid);
      I* instance = static_cast<I*> (m_records[automaton.aid]->get_instance ());
      instance->unbind_input_automaton_dne (d);
      clear_and_unlock (scheduler, automaton.aid);
    }

    template <class I, class L, class D>
    void unbind_output_parameter_dne (const automaton_handle<I>& automaton,
				      scheduler_interface& scheduler,
				      L& listener,
				      D& d)
    {
      boost::shared_lock<boost::shared_mutex> lock (m_mutex);
      
      if (!m_aids.contains (automaton.aid)) {
      	listener.automaton_dne ();
      	return;
      }

      lock_and_set (scheduler, automaton.aid);
      I* instance = static_cast<I*> (m_records[automaton.aid]->get_instance ());
      instance->unbind_output_parameter_dne (d);
      clear_and_unlock (scheduler, automaton.aid);
    }

    template <class I, class L, class D>
    void unbind_input_parameter_dne (const automaton_handle<I>& automaton,
				     scheduler_interface& scheduler,
				     L& listener,
				     D& d)
    {
      boost::shared_lock<boost::shared_mutex> lock (m_mutex);
      
      if (!m_aids.contains (automaton.aid)) {
      	listener.automaton_dne ();
      	return;
      }

      lock_and_set (scheduler, automaton.aid);
      I* instance = static_cast<I*> (m_records[automaton.aid]->get_instance ());
      instance->unbind_input_parameter_dne (d);
      clear_and_unlock (scheduler, automaton.aid);
    }
    
    template <class I, class L, class D>
    void binding_dne (const automaton_handle<I>& automaton,
		      scheduler_interface& scheduler,
		      L& listener,
		      D& d)
    {
      boost::shared_lock<boost::shared_mutex> lock (m_mutex);
      
      if (!m_aids.contains (automaton.aid)) {
      	listener.automaton_dne ();
      	return;
      }

      lock_and_set (scheduler, automaton.aid);
      I* instance = static_cast<I*> (m_records[automaton.aid]->get_instance ());
      instance->binding_dne (d);
      clear_and_unlock (scheduler, automaton.aid);
    }
    
    template <class I, class L, class D>
    void unbound (const automaton_handle<I>& automaton,
		  scheduler_interface& scheduler,
		  L& listener,
		  D& d)
    {
      boost::shared_lock<boost::shared_mutex> lock (m_mutex);
      
      if (!m_aids.contains (automaton.aid)) {
      	listener.automaton_dne ();
      	return;
      }

      lock_and_set (scheduler, automaton.aid);
      I* instance = static_cast<I*> (m_records[automaton.aid]->get_instance ());
      instance->unbound (d);
      clear_and_unlock (scheduler, automaton.aid);
    }

    template <class I, class M, class L>
    void
    action_unbound (const action<I, M>& ac,
		    scheduler_interface& scheduler,
		    L& listener)
    {
      boost::shared_lock<boost::shared_mutex> lock (m_mutex);
      
      if (!m_aids.contains (ac.automaton.aid)) {
	listener.automaton_dne ();
	return;
      }

      if (!parameter_exists (ac, typename action<I, M>::parameter_status ())) {
	listener.parameter_dne ();
	return;
      }

      concrete_action<I, M> rac = reify (ac);

      lock_and_set (scheduler, rac.automaton.aid);
      rac.unbound ();
      clear_and_unlock (scheduler, rac.automaton.aid);
    }

    template <class I, class L, class D>
    void parameter_dne (const automaton_handle<I>& automaton,
			scheduler_interface& scheduler,
			L& listener,
			D& d)
    {
      boost::shared_lock<boost::shared_mutex> lock (m_mutex);
      
      if (!m_aids.contains (automaton.aid)) {
      	listener.automaton_dne ();
      	return;
      }

      lock_and_set (scheduler, automaton.aid);
      I* instance = static_cast<I*> (m_records[automaton.aid]->get_instance ());
      instance->parameter_dne (d);
      clear_and_unlock (scheduler, automaton.aid);
    }

    template <class I, class L, class D>
    void parameter_rescinded (const automaton_handle<I>& automaton,
			      scheduler_interface& scheduler,
			      L& listener,
			      D& d)
    {
      boost::shared_lock<boost::shared_mutex> lock (m_mutex);
      
      if (!m_aids.contains (automaton.aid)) {
      	listener.automaton_dne ();
      	return;
      }

      lock_and_set (scheduler, automaton.aid);
      I* instance = static_cast<I*> (m_records[automaton.aid]->get_instance ());
      instance->parameter_rescinded (d);
      clear_and_unlock (scheduler, automaton.aid);
    }

    template <class I, class L, class D>
    void target_automaton_dne (const automaton_handle<I>& automaton,
			       scheduler_interface& scheduler,
			       L& listener,
			       D& d)
    {
      boost::shared_lock<boost::shared_mutex> lock (m_mutex);
      
      if (!m_aids.contains (automaton.aid)) {
      	listener.automaton_dne ();
      	return;
      }

      lock_and_set (scheduler, automaton.aid);
      I* instance = static_cast<I*> (m_records[automaton.aid]->get_instance ());
      instance->target_automaton_dne (d);
      clear_and_unlock (scheduler, automaton.aid);
    }

    template <class I, class L, class D>
    void destroyer_not_creator (const automaton_handle<I>& automaton,
				scheduler_interface& scheduler,
				L& listener,
				D& d)
    {
      boost::shared_lock<boost::shared_mutex> lock (m_mutex);
      
      if (!m_aids.contains (automaton.aid)) {
      	listener.automaton_dne ();
      	return;
      }

      lock_and_set (scheduler, automaton.aid);
      I* instance = static_cast<I*> (m_records[automaton.aid]->get_instance ());
      instance->destroyer_not_creator (d);
      clear_and_unlock (scheduler, automaton.aid);
    }

    template <class I, class L, class D>
    void automaton_destroyed (const automaton_handle<I>& automaton,
			      scheduler_interface& scheduler,
			      L& listener,
			      D& d)
    {
      boost::shared_lock<boost::shared_mutex> lock (m_mutex);
      
      if (!m_aids.contains (automaton.aid)) {
      	listener.automaton_dne ();
      	return;
      }

      lock_and_set (scheduler, automaton.aid);
      I* instance = static_cast<I*> (m_records[automaton.aid]->get_instance ());
      instance->automaton_destroyed (d);
      clear_and_unlock (scheduler, automaton.aid);
    }
    
  };

}

#endif
