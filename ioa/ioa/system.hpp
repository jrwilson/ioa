#ifndef __system_hpp__
#define __system_hpp__

#include "sequential_set.hpp"
#include <boost/foreach.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/utility.hpp>
#include <list>
#include "automaton_handle.hpp"
#include "parameter_handle.hpp"
#include "binding.hpp"

namespace ioa {

  // TODO:  Memory allocation.

  class parameter_record_interface
  {
  public:
    virtual ~parameter_record_interface () { }
    virtual const aid_t get_aid () const = 0;
    virtual const pid_t get_pid () const = 0;
    virtual void* get_parameter () const = 0;
  };

  template <class I, class D>
  class parameter_record :
    public parameter_record_interface
  {
  private:
    I* m_instance;
    aid_t m_aid;
    pid_t m_pid;
    void* m_param;
    scheduler_interface& m_scheduler;
    D& m_d;

  public:
    parameter_record (I* instance,
		      aid_t aid,
		      pid_t pid,
		      void* param,
		      scheduler_interface& scheduler,
		      D& d) :
      m_instance (instance),
      m_aid (aid),
      m_pid (pid),
      m_param (param),
      m_scheduler (scheduler),
      m_d (d)
    { }

    ~parameter_record () {

      m_scheduler.set_current_aid (m_aid, m_instance);
      m_instance->parameter_rescinded (m_d);
      m_scheduler.clear_current_aid ();
    }

    const aid_t get_aid () const {
      return m_aid;
    }
    
    const pid_t get_pid () const {
      return m_pid;
    }

    void* get_parameter () const {
      return m_param;
    }

  };

  class automaton_record_interface :
    public boost::mutex
  {
  private:
    std::auto_ptr<automaton_interface> m_instance;
    aid_t m_aid;
    sequential_set<pid_t> m_pids;
    std::map<void*, parameter_record_interface*> m_param_to_record;
    std::map<pid_t, parameter_record_interface*> m_pid_to_record;
    std::set<automaton_record_interface*> m_children;
    automaton_record_interface* m_parent;

  public:
    automaton_record_interface (automaton_interface* instance,
				const aid_t aid) :
      m_instance (instance),
      m_aid (aid),
      m_parent (0)
    { }

    virtual ~automaton_record_interface () {
      BOOST_ASSERT (m_pids.empty ());
      BOOST_ASSERT (m_param_to_record.empty ());
      BOOST_ASSERT (m_pid_to_record.empty ());
      BOOST_ASSERT (m_children.empty ());
    }


    const aid_t get_aid () const {
      return m_aid;
    }

    automaton_interface* get_instance () const {
      return m_instance.get ();
    }

    bool parameter_exists (void* parameter) const {
      return m_param_to_record.count (parameter) != 0;
    }

    template <class P>
    bool parameter_exists (const parameter_handle<P>& parameter) const {
      return m_pid_to_record.count (parameter.pid ()) != 0;
    }

    template <class P>
    P* get_parameter (const parameter_handle<P>& parameter) const {
      std::map<pid_t, parameter_record_interface*>::const_iterator pos = m_pid_to_record.find (parameter.pid ());
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
      BOOST_ASSERT (get_aid () == record->get_aid ());
      pid_t pid = record->get_pid ();
      BOOST_ASSERT (m_pids.contains (pid));
      m_param_to_record.insert (std::make_pair (record->get_parameter (), record));
      m_pid_to_record.insert (std::make_pair (pid, record));
    }

    template <class P>
    void rescind_parameter (const parameter_handle<P>& parameter) {
      BOOST_ASSERT (parameter_exists (parameter));
      parameter_record_interface* record = m_pid_to_record[parameter.pid ()];
      m_pid_to_record.erase (parameter.pid ());
      m_param_to_record.erase (record->get_parameter ());
      delete record;
    }

    void rescind_all () {
      m_pids.clear ();
      for (std::map<pid_t, parameter_record_interface*>::iterator pos = m_pid_to_record.begin ();
	   pos != m_pid_to_record.end ();
	   ++pos) {
	delete pos->second;
      }
      m_pid_to_record.clear ();
      m_param_to_record.clear ();
    }

    void add_child (automaton_record_interface* child) {
      m_children.insert (child);
      child->set_parent (this);
    }

    void remove_child (automaton_record_interface* child) {
      m_children.erase (child);
    }

    automaton_record_interface* get_child () const {
      if (m_children.empty ()) {
	return 0;
      }
      else {
	return *(m_children.begin ());
      }
    }

    void set_parent (automaton_record_interface* parent) {
      m_parent = parent;
    }
    
    automaton_record_interface* get_parent () const {
      return m_parent;
    }
  };

  class root_automaton_record :
    public automaton_record_interface
  {
  public:
    root_automaton_record (automaton_interface* instance,
			   const aid_t aid) :
      automaton_record_interface (instance, aid)
    { }
  };

  template <class P, class D>
  class automaton_record :
    public automaton_record_interface
  {
  private:
    P* m_parent_instance;
    scheduler_interface& m_scheduler;
    D& m_d;
    
  public:
    automaton_record (automaton_interface* instance,
		      const aid_t aid,
		      P* parent_instance,
		      scheduler_interface& scheduler,
		      D& d) :
      automaton_record_interface (instance, aid),
      m_parent_instance (parent_instance),
      m_scheduler (scheduler),
      m_d (d)
    { }
    
    ~automaton_record () {
      BOOST_ASSERT (this->get_parent () != 0);
      aid_t parent_aid = this->get_parent ()->get_aid ();
      m_scheduler.set_current_aid (parent_aid, m_parent_instance);
      m_parent_instance->automaton_destroyed (m_d);
      m_scheduler.clear_current_aid ();
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
    std::list<binding_interface*> m_bindings;
    
  public:
    
    void clear (void) {
      // Delete all root automata.
      while (!m_records.empty ()) {
      	for (std::map<aid_t, automaton_record_interface*>::const_iterator pos = m_records.begin ();
      	     pos != m_records.end ();
      	     ++pos) {
      	  if (pos->second->get_parent () == 0) {
      	    inner_destroy (pos->second);
      	    break;
      	  }
      	}
      }

      BOOST_ASSERT (m_aids.empty ());
      BOOST_ASSERT (m_instances.empty ());
      BOOST_ASSERT (m_records.empty ());
      BOOST_ASSERT (m_bindings.empty ());
    }

    ~system (void) {
      clear ();
    }
    
    template <class I>
    automaton_handle<I> cast_aid (const I* /* */, const aid_t aid) const {
      return automaton_handle<I> (aid);
    }

    template <class G>
    automaton_handle<typename G::result_type>
    create (G generator,
	    scheduler_interface& scheduler)
    {
      boost::unique_lock<boost::shared_mutex> lock (m_mutex);

      // Take an aid.
      automaton_handle<typename G::result_type> handle (m_aids.take ());

      // Set the current aid.
      scheduler.set_current_aid (handle.aid ());

      // Run the generator.
      typename G::result_type* instance = generator ();
      BOOST_ASSERT (instance != 0);

      // Clear the current aid.
      scheduler.clear_current_aid ();
      
      if (m_instances.count (instance) != 0) {
	// Root automaton instance exists.  Bad news.
	m_aids.replace (handle.aid ());
      	return automaton_handle<typename G::result_type> ();
      }

      m_instances.insert (instance);
      automaton_record_interface* record = new root_automaton_record (instance, handle.aid ());
      m_records.insert (std::make_pair (handle.aid (), record));

      // Initialize the automaton.
      scheduler.set_current_aid (handle.aid (), instance);
      instance->init ();
      scheduler.clear_current_aid ();

      return handle;
    }

    template <class P, class G, class D>
    automaton_handle<typename G::result_type>
    create (const automaton_handle<P>& automaton,
    	    G generator,
	    scheduler_interface& scheduler,
	    D& d)
    {
      boost::unique_lock<boost::shared_mutex> lock (m_mutex);
      
      if (!m_aids.contains (automaton.aid ())) {
	return automaton_handle<typename G::result_type> ();
      }

      P* p = aid_to_instance<P> (automaton.aid ());

      // Take an aid.
      automaton_handle<typename G::result_type> handle (m_aids.take ());

      // Set the current aid.
      scheduler.set_current_aid (handle.aid ());

      // Run the generator.
      typename G::result_type* instance = generator ();
      BOOST_ASSERT (instance != 0);

      // Clear the current aid.
      scheduler.clear_current_aid ();

      if (m_instances.count (instance) != 0) {
	// Return the aid.
	m_aids.replace (handle.aid ());

	scheduler.set_current_aid (automaton.aid (), p);
	p->instance_exists (instance, d);
	scheduler.clear_current_aid ();

	return automaton_handle<typename G::result_type> ();
      }

      m_instances.insert (instance);      
      automaton_record_interface* record = new automaton_record<P, D> (instance, handle.aid (), p, scheduler, d);
      m_records.insert (std::make_pair (handle.aid (), record));
      automaton_record_interface* parent = m_records[automaton.aid ()];
      parent->add_child (record);

      // Tell the parent the child was created.
      scheduler.set_current_aid (automaton.aid (), p);
      p->automaton_created (handle, d);
      scheduler.clear_current_aid ();

      // Initialize the child.
      scheduler.set_current_aid (handle.aid (), instance);
      instance->init ();
      scheduler.clear_current_aid ();

      return handle;
    }
    
    template <class I, class P, class D>
    parameter_handle<P>
    declare (const automaton_handle<I>& automaton,
  	     P* parameter,
	     scheduler_interface& scheduler,
	     D& d)
    {
      boost::unique_lock<boost::shared_mutex> lock (m_mutex);
      
      if (!m_aids.contains (automaton.aid ())) {
  	return parameter_handle<P> ();
      }

      I* instance = aid_to_instance<I> (automaton.aid ());
      
      automaton_record_interface* record = m_records[automaton.aid ()];
      
      if (record->parameter_exists (parameter)) {
	scheduler.set_current_aid (automaton.aid (), instance);
	instance->parameter_exists (d);
	scheduler.clear_current_aid ();
	return parameter_handle<P> ();
      }

      parameter_handle<P> handle = record->declare_parameter (parameter);
      parameter_record_interface* p_record = new parameter_record<I, D> (instance, automaton.aid (), handle.pid (), parameter, scheduler, d);
      record->insert_parameter_record (p_record);

      scheduler.set_current_aid (automaton.aid (), instance);
      instance->parameter_declared (handle, d);
      scheduler.clear_current_aid ();

      return handle;
    }
    
  private:
    template <class I, class M>
    bool parameter_exists (const action<I, M>& ac, unparameterized /* */) {
      return true;
    }

    template <class I, class M>
    bool parameter_exists (const action<I, M>& ac, parameterized /* */) {
      return m_records[ac.automaton.aid ()]->parameter_exists (ac.parameter);
    }

    template <class I, class M>
    concrete_action<I, M> reify0 (const action<I, M>& ac,
				  unparameterized /* */) {
      I* instance = dynamic_cast<I*> (m_records[ac.automaton.aid ()]->get_instance ());
      BOOST_ASSERT (instance != 0);
      return concrete_action<I, M> (ac, instance);
    }

    template <class I, class M>
    concrete_action<I, M> reify0 (const action<I, M>& ac,
				  parameterized /* */) {
      I* instance = dynamic_cast<I*> (m_records[ac.automaton.aid ()]->get_instance ());
      typename action<I,M>::parameter_type* parameter = m_records[ac.automaton.aid ()]->get_parameter (ac.parameter);

      return concrete_action<I, M> (ac, instance, parameter);
    }

    template <class I, class M>
    concrete_action<I, M> reify (const action<I, M>& ac) {
      return reify0 (ac, typename action<I, M>::parameter_status ());
    }

    template <class OI, class OM, class II, class IM, class I, class D>
    bool
    _bind (const action<OI, OM>& output,
	   const action<II, IM>& input,
	   const automaton_handle<I>& binder,
	   scheduler_interface& scheduler,
	   D& d)
    {
      if (!m_aids.contains (binder.aid ())) {
  	// Binder DNE.
  	return false;
      }

      I* instance = aid_to_instance<I> (binder.aid ());

      if (!m_aids.contains (output.automaton.aid ())) {
	scheduler.set_current_aid (binder.aid (), instance);
	instance->bind_output_automaton_dne (d);
	scheduler.clear_current_aid ();
	return false;
      }

      if (!m_aids.contains (input.automaton.aid ())) {
	scheduler.set_current_aid (binder.aid (), instance);
	instance->bind_input_automaton_dne (d);
	scheduler.clear_current_aid ();
	return false;
      }
      
      if (!parameter_exists (output, typename action<OI, OM>::parameter_status ())) {
	scheduler.set_current_aid (binder.aid (), instance);
	instance->bind_output_parameter_dne (d);
	scheduler.clear_current_aid ();
	return false;
      }
      
      if (!parameter_exists (input, typename action<II, IM>::parameter_status ())) {
	scheduler.set_current_aid (binder.aid (), instance);
	instance->bind_input_parameter_dne (d);
	scheduler.clear_current_aid ();
	return false;
      }
      
      concrete_action<OI, OM> c_output = reify (output);
      concrete_action<II, IM> c_input = reify (input);

      std::list<binding_interface*>::const_iterator pos = std::find_if (m_bindings.begin (),
									m_bindings.end (),
									binding_equal (c_output, c_input, binder.aid ()));
      
      if (pos != m_bindings.end ()) {
  	// Bound.
	scheduler.set_current_aid (binder.aid (), instance);
	instance->binding_exists (d);
	scheduler.clear_current_aid ();
	return false;
      }
      
      std::list<binding_interface*>::const_iterator in_pos = std::find_if (m_bindings.begin (),
									   m_bindings.end (),
									   binding_input_equal (c_input));
      
      if (in_pos != m_bindings.end ()) {
  	// Input unavailable.
	scheduler.set_current_aid (binder.aid (), instance);
	instance->input_action_unavailable (d);
	scheduler.clear_current_aid ();
	return false;
      }
      
      std::list<binding_interface*>::const_iterator out_pos = std::find_if (m_bindings.begin (),
									    m_bindings.end (),
									    binding_output_equal (c_output));
      
      if (output.automaton.aid () == input.automaton.aid () ||
  	  (out_pos != m_bindings.end () && (*out_pos)->involves_input_automaton (input.automaton.aid ()))) {
  	// Output unavailable.
	scheduler.set_current_aid (binder.aid (), instance);
	instance->output_action_unavailable (d);
	scheduler.clear_current_aid ();
	return false;
      }
      
      binding<OM>* c;
      
      if (out_pos != m_bindings.end ()) {
	c = dynamic_cast<binding<OM>*> (*out_pos);
	BOOST_ASSERT (c != 0);
      }
      else {
	c = new binding<OM> ();
	m_bindings.push_front (c);
      }
    
      // Bind.
      c->bind (c_output, c_input, instance, binder.aid (), scheduler, d);

      scheduler.set_current_aid (binder.aid (), instance);
      instance->bound (d);
      scheduler.clear_current_aid ();

      scheduler.set_current_aid (c_output.automaton.aid (), c_output.get_instance ());
      c_output.bound ();
      scheduler.clear_current_aid ();

      scheduler.set_current_aid (c_input.automaton.aid (), c_input.get_instance ());
      c_input.bound ();
      scheduler.clear_current_aid ();

      return true;
    }

    template <class OI, class OM, class II, class IM, class I, class D>
    bool
    bind (const action<OI, OM>& output,
	  output_category /* */,
	  unparameterized /* */,
	  const action<II, IM>& input,
	  input_category /* */,
	  unparameterized /* */,
	  const automaton_handle<I>& binder,
	  scheduler_interface& scheduler,
	  D& d)
    {
      return _bind (output, input, binder, scheduler, d);
    }    

    template <class OI, class OM, class II, class IM, class D>
    bool
    bind (const action<OI, OM>& output,
	  output_category /* */,
	  parameterized /* */,
	  const action<II, IM>& input,
	  input_category /* */,
	  unparameterized /* */,
	  scheduler_interface& scheduler,
	  D& d)
    {
      return _bind (output, input, output.automaton, scheduler, d);
    }    

    template <class OI, class OM, class II, class IM, class D>
    bool
    bind (const action<OI, OM>& output,
	  output_category /* */,
	  unparameterized /* */,
	  const action<II, IM>& input,
	  input_category /* */,
	  parameterized /* */,
	  scheduler_interface& scheduler,
	  D& d)
    {
      return _bind (output, input, input.automaton, scheduler, d);
    }    

  public:
    template <class OI, class OM, class II, class IM, class I, class D>
    bool
    bind (const action<OI, OM>& output,
	  const action<II, IM>& input,
	  const automaton_handle<I>& binder,
	  scheduler_interface& scheduler,
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
		   scheduler,
		   d);
    }

    template <class OI, class OM, class II, class IM, class D>
    bool
    bind (const action<OI, OM>& output,
	  const action<II, IM>& input,
	  scheduler_interface& scheduler,
	  D& d)
    {
      boost::unique_lock<boost::shared_mutex> lock (m_mutex);

      return bind (output,
		   typename action<OI, OM>::action_category (),
		   typename action<OI, OM>::parameter_status (),
		   input,
		   typename action<II, IM>::action_category (),
		   typename action<II, IM>::parameter_status (),
		   scheduler,
		   d);
    }
  
  private:
    template <class OI, class OM, class II, class IM, class I, class D>
    bool
    _unbind (const action<OI, OM>& output,
	     const action<II, IM>& input,
	     const automaton_handle<I>& binder,
	     scheduler_interface& scheduler,
	     D& d)
    {
      if (!m_aids.contains (binder.aid ())) {
	return false;
      }


      I* instance = aid_to_instance<I> (binder.aid ());

      if (!m_aids.contains (output.automaton.aid ())) {
	scheduler.set_current_aid (binder.aid (), instance);
	instance->unbind_output_automaton_dne (d);
	scheduler.clear_current_aid ();
	return false;
      }

      if (!m_aids.contains (input.automaton.aid ())) {
	scheduler.set_current_aid (binder.aid (), instance);
	instance->unbind_input_automaton_dne (d);
	scheduler.clear_current_aid ();
	return false;
      }
      
      if (!parameter_exists (output, typename action<OI, OM>::parameter_status ())) {
	scheduler.set_current_aid (binder.aid (), instance);
	instance->unbind_output_parameter_dne (d);
	scheduler.clear_current_aid ();
	return false;
      }
      
      if (!parameter_exists (input, typename action<II, IM>::parameter_status ())) {
	scheduler.set_current_aid (binder.aid (), instance);
	instance->unbind_input_parameter_dne (d);
	scheduler.clear_current_aid ();
	return false;
      }

      concrete_action<OI, OM> c_output = reify (output);
      concrete_action<II, IM> c_input = reify (input);
      
      std::list<binding_interface*>::iterator pos = std::find_if (m_bindings.begin (),
								  m_bindings.end (),
								  binding_equal (c_output, c_input, binder.aid ()));
      
      if (pos == m_bindings.end ()) {
	// Not bound.
	scheduler.set_current_aid (binder.aid (), instance);
	instance->binding_dne (d);
	scheduler.clear_current_aid ();
	return false;
      }
      
      binding<OM>* c = dynamic_cast<binding<OM>*> (*pos);
      BOOST_ASSERT (c != 0);
      
      // Unbind.
      c->unbind (c_output, c_input, instance, binder.aid ());
      
      if (c->empty ()) {
	delete c;
	m_bindings.erase (pos);
      }

      scheduler.set_current_aid (binder.aid (), instance);
      instance->unbound (d);
      scheduler.clear_current_aid ();
      return true;
    }

    template <class OI, class OM, class II, class IM, class I, class D>
    bool
    unbind (const action<OI, OM>& output,
	    unparameterized /* */,
	    const action<II, IM>& input,
	    unparameterized /* */,
	    const automaton_handle<I>& binder,
	    scheduler_interface& scheduler,
	    D& d)
    {
      return _unbind (output, input, binder, scheduler, d);
    }    
    
    template <class OI, class OM, class II, class IM, class D>
    bool
    unbind (const action<OI, OM>& output,
	    parameterized /* */,
	    const action<II, IM>& input,
	    unparameterized /* */,
	    scheduler_interface& scheduler,
	    D& d)
    {
      return _unbind (output, input, output.automaton, scheduler, d);
    }    
    
    template <class OI, class OM, class II, class IM, class D>
    bool
    unbind (const action<OI, OM>& output,
	    unparameterized /* */,
	    const action<II, IM>& input,
	    parameterized /* */,
	    scheduler_interface& scheduler,
	    D& d)
    {
      return _unbind (output, input, input.automaton, scheduler, d);
    }    
    
  public:
    template <class OI, class OM, class II, class IM, class I, class D>
    bool
    unbind (const action<OI, OM>& output,
	    const action<II, IM>& input,
	    const automaton_handle<I>& binder,
	    scheduler_interface& scheduler,
	    D& d)
    {
      boost::unique_lock<boost::shared_mutex> lock (m_mutex);

      return unbind (output, typename action<OI, OM>::parameter_status (), input, typename action<II, IM>::parameter_status (), binder, scheduler, d);
    }

    template <class OI, class OM, class II, class IM, class D>
    bool
    unbind (const action<OI, OM>& output,
	    const action<II, IM>& input,
	    scheduler_interface& scheduler,
	    D& d)
    {
      boost::unique_lock<boost::shared_mutex> lock (m_mutex);

      return unbind (output, typename action<OI, OM>::parameter_status (), input, typename action<II, IM>::parameter_status (), scheduler, d);
    }

    template <class I, class P, class D>
    bool
    rescind (const automaton_handle<I>& automaton,
  	     const parameter_handle<P>& parameter,
	     scheduler_interface& scheduler,
	     D& d)
    {
      boost::unique_lock<boost::shared_mutex> lock (m_mutex);
      
      if (!m_aids.contains (automaton.aid ())) {
  	return false;
      }
      
      automaton_record_interface* record = m_records[automaton.aid ()];

      I* instance = aid_to_instance<I> (automaton.aid ());
      
      if (!record->parameter_exists (parameter)) {
	scheduler.set_current_aid (automaton.aid (), instance);
	instance->parameter_dne (d);
	scheduler.clear_current_aid ();
	return false;
      }
      
      record->rescind_parameter (parameter);
      
      for (std::list<binding_interface*>::iterator pos = m_bindings.begin ();
	   pos != m_bindings.end ();
	   ) {
	(*pos)->unbind_parameter (automaton.aid (), parameter.pid ());
	if ((*pos)->empty ()) {
	  delete *pos;
	  pos = m_bindings.erase (pos);
	}
	else {
	  ++pos;
	}
      }
      
      return true;
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

    void
    inner_destroy (automaton_record_interface* automaton)
    {

      for (automaton_record_interface* child = automaton->get_child ();
      	   child != 0;
      	   child = automaton->get_child ()) {
      	inner_destroy (child);
      }

      // Update bindings.
      for (std::list<binding_interface*>::iterator pos = m_bindings.begin ();
      	   pos != m_bindings.end ();
      	   ) {
      	(*pos)->unbind_automaton (automaton->get_aid ());
      	if ((*pos)->empty ()) {
      	  delete *pos;
      	  pos = m_bindings.erase (pos);
      	}
      	else {
      	  ++pos;
      	}
      }

      // Rescind parameters.
      automaton->rescind_all ();
      
      // Update parent-child relationships.
      automaton_record_interface* parent = automaton->get_parent ();
      if (parent != 0) {
      	parent->remove_child (automaton);
      }

      m_aids.replace (automaton->get_aid ());
      m_instances.erase (automaton->get_instance ());
      m_records.erase (automaton->get_aid ());
      delete automaton;
    }

  public:

    template <class I>
    bool
    destroy (const automaton_handle<I>& target)
    {
      boost::unique_lock<boost::shared_mutex> lock (m_mutex);
      
      if (!m_aids.contains (target.aid ())) {
	return false;
      }

      inner_destroy (m_records[target.aid ()]);
      return true;
    }

    template <class P, class I, class D>
    bool
    destroy (const automaton_handle<P>& automaton,
	     const automaton_handle<I>& target,
	     scheduler_interface& scheduler,
	     D& d)
    {
      boost::unique_lock<boost::shared_mutex> lock (m_mutex);

      if (!m_aids.contains (automaton.aid ())) {
	return false;
      }

      I* instance = aid_to_instance<I> (automaton.aid ());

      if (!m_aids.contains (target.aid ())) {
	scheduler.set_current_aid (automaton.aid (), instance);
	instance->target_automaton_dne (d);
	scheduler.clear_current_aid ();
	return false;
      }

      automaton_record_interface* parent = m_records[automaton.aid ()];
      automaton_record_interface* child = m_records[target.aid ()];

      if (parent != child->get_parent ()) {
	scheduler.set_current_aid (automaton.aid (), instance);
	instance->destroyer_not_creator (d);
	scheduler.clear_current_aid ();
	return false;
      }

      inner_destroy (child);
      return true;
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
      lock_automaton (ac.automaton.aid ());
      scheduler.set_current_aid (ac.automaton.aid (), ac.get_instance ());
      ac.execute ();
      scheduler.clear_current_aid ();
      unlock_automaton (ac.automaton.aid ());
    }

    template <class I, class M>
    void
    execute1 (const concrete_action<I, M>& ac,
	      output_category /* */,
	      scheduler_interface& scheduler)
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

    template <class I, class M>
    void
    execute1 (const concrete_action<I, M>& ac,
	      internal_category /* */,
	      scheduler_interface& scheduler)
    {
      execute0 (ac, scheduler);
    }

    template <class I, class M>
    void
    execute1 (const concrete_action<I, M>& ac,
	      event_category /* */,
	      scheduler_interface& scheduler)
    {
      execute0 (ac, scheduler);
    }

  public:
    
    template <class I, class M>
    bool
    execute (const action<I, M>& ac,
	     scheduler_interface& scheduler)
    {
      boost::shared_lock<boost::shared_mutex> lock (m_mutex);
      
      if (!m_aids.contains (ac.automaton.aid ())) {
	return false;
      }

      if (!parameter_exists (ac, typename action<I, M>::parameter_status ())) {
	return false;
      }

      concrete_action<I, M> rac = reify (ac);
      execute1 (rac, typename concrete_action<I, M>::action_category (), scheduler);
      return true;
    }

  private:

    template <class I>
    I* aid_to_instance (const aid_t aid) {
      I* instance = dynamic_cast<I*> (m_records[aid]->get_instance ());
      BOOST_ASSERT (instance != 0);
      return instance;
    }

  };

}

#endif
