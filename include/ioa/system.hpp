#ifndef __system_hpp__
#define __system_hpp__

#include <ioa/automaton_record.hpp>

#include <ioa/generator_interface.hpp>
#include <ioa/unique_lock.hpp>
#include <ioa/shared_lock.hpp>

#include <map>
#include <list>

// TODO:  Cleanup redundancy.

namespace ioa {

  template <class OVS, class OVT, class IVS, class IVT> struct bind_check;
  
  template <>
  struct bind_check<unvalued, null_type, unvalued, null_type> { };

  // These two lines are extremely important.
  // They ensure that the types match when binding an input to an output.
  template <class VT>
  struct bind_check<valued, VT, valued, VT> { };

  class action_executor_interface
  {
  public:
    virtual ~action_executor_interface () { }
    virtual bool fetch_instance () = 0;
    virtual const action_interface& get_action () const = 0;
  };

  class input_executor_interface :
    public action_executor_interface
  {
  public:
    virtual ~input_executor_interface () { }
    virtual input_executor_interface* clone () const = 0;
  };

  class unvalued_input_executor_interface :
    public input_executor_interface
  {
  public:
    virtual ~unvalued_input_executor_interface () { }
    virtual void operator() () const = 0;
  };

  template <typename T>
  class valued_input_executor_interface :
    public input_executor_interface
  {
  public:
    virtual ~valued_input_executor_interface () { }
    virtual void operator() (const T& t) const = 0;
  };

  class local_executor_interface :
    public action_executor_interface
  {
  public:
    virtual ~local_executor_interface () { }
    virtual void operator() () const = 0;
  };
  
  class output_executor_interface :
    public local_executor_interface
  {
  public:
    virtual ~output_executor_interface () { }
    virtual output_executor_interface* clone () const = 0;
    virtual bool involves_output (const action_interface&) const = 0;
    virtual bool involves_input (const action_interface&) const = 0;
    virtual bool involves_input_automaton (const aid_t) const = 0;
    virtual bool involves_binding (const action_interface&,
				   const action_interface&,
				   const aid_t) const = 0;
    virtual bool involves_aid_key (const aid_t,
				   void* const) const = 0;
    virtual bool empty () const = 0;
    virtual void bind (const input_executor_interface&,
		       const aid_t,
		       void* const) = 0;
    virtual void unbind (const aid_t,
			 void* const) = 0;
    virtual void unbind_automaton (const aid_t) = 0;
  };

  class unvalued_output_executor_interface :
    public output_executor_interface
  {
  public:
    virtual ~unvalued_output_executor_interface () { }
  };

  template <typename T>
  class valued_output_executor_interface :
    public output_executor_interface
  {
  public:
    virtual ~valued_output_executor_interface () { }
  };
  
  class internal_executor_interface :
    public local_executor_interface
  {
  public:
    virtual ~internal_executor_interface () { }
  };
  
  class event_executor_interface :
    public local_executor_interface
  {
  public:
    virtual ~event_executor_interface () { }
  };

  class bind_executor_interface
  {
  public:
    virtual ~bind_executor_interface () { }
    virtual output_executor_interface& get_output () = 0;
    virtual input_executor_interface& get_input () = 0;
  };

  // TODO:  Memory allocation.

  class system
  {

  private:    
    // Can't construct one.
    system () { }
    
    class binding_equal
    {
    private:
      const action_interface& m_output;
      const action_interface& m_input;
      const aid_t m_binder;

    public:
      binding_equal (const action_interface& output,
		     const action_interface& input,
		     const aid_t binder) :
  	m_output (output),
  	m_input (input),
	m_binder (binder)
      { }
      
      bool operator() (const output_executor_interface* c) const {
  	return c->involves_binding (m_output, m_input, m_binder);
      }
    };
    
    class binding_output_equal
    {
    private:
      const action_interface& m_output;
      
    public:
      binding_output_equal (const action_interface& output) :
  	m_output (output)
      { }
      
      bool operator() (const output_executor_interface* c) const {
  	return c->involves_output (m_output);
      }
    };
    
    class binding_input_equal
    {
    private:
      const action_interface& m_input;
      
    public:
      binding_input_equal (const action_interface& input) :
  	m_input (input)
      { }
      
      bool operator() (const output_executor_interface* c) const {
  	return c->involves_input (m_input);
      }
    };

    class binding_aid_key_equal
    {
    private:
      aid_t const m_aid;
      void* const m_key;

    public:
      binding_aid_key_equal (aid_t const aid,
			     void* const key) :
	m_aid (aid),
	m_key (key)
      { }

      bool operator() (const output_executor_interface* c) const {
	return c->involves_aid_key (m_aid, m_key);
      }
    };
    
    static shared_mutex m_mutex;
    static sequential_set<aid_t> m_aids;
    static std::set<automaton_interface*> m_instances;
    static std::map<aid_t, automaton_record*> m_records;
    static std::list<output_executor_interface*> m_bindings;

    template <class I>
    static I* automaton_handle_to_instance (const automaton_handle<I>& handle) {
      if (!m_aids.contains (handle)) {
	return 0;
      }
      return dynamic_cast<I*> (m_records[handle]->get_instance ());
    }

    static void lock_automaton (const aid_t handle);
    static void unlock_automaton (const aid_t handle);
    static void inner_destroy (automaton_record* automaton);
    
  public:

    static void clear (void);
    static aid_t create (std::auto_ptr<generator_interface> generator);
    static aid_t create (const aid_t automaton,
			 std::auto_ptr<generator_interface> generator,
			 void* const key);
    static int bind (bind_executor_interface& bind_exec,
		     const aid_t binder,
		     void* const key);
    static int unbind (const aid_t binder,
		       void* const key);
    static int destroy (const aid_t target);
    static int destroy (const aid_t automaton,
			void* const key);
    static int execute (output_executor_interface& exec);
    static int execute (internal_executor_interface& exec);
    static int execute (const aid_t from,
			event_executor_interface& exec,
			void* const key);

    template <class I, class M, class K, class VS, class VT> class action_executor_impl;

    template <class I, class M>
    class action_executor_impl<I, M, input_category, unvalued, null_type> :
      public unvalued_input_executor_interface
    {
    private:
      const action<I, M> m_action;
      I* m_instance;
      
    public:
      action_executor_impl (const action<I, M>& action) :
	m_action (action),
	m_instance (0)
      { }

      action_executor_impl (const action_executor_impl& other) :
	m_action (other.m_action),
	m_instance (other.m_instance)
      { }
      
      bool fetch_instance () {
	m_instance = system::automaton_handle_to_instance (m_action.automaton);
	return m_instance != 0;
      }
      
      void operator() () const {
	assert (m_instance != 0);
	system_scheduler::set_current_aid (m_action.get_aid (), *m_instance);
	m_action (*m_instance);
	system_scheduler::clear_current_aid ();
      }
    
      const action_interface& get_action () const {
	return m_action;
      }

      action_executor_impl* clone () const {
	return new action_executor_impl (*this);
      }
    };

    template <class I, class M, class VT>
    class action_executor_impl<I, M, input_category, valued, VT> :
      public valued_input_executor_interface<VT>
    {
    private:
      const action<I, M> m_action;
      I* m_instance;
      
    public:
      action_executor_impl (const action<I, M>& action) :
	m_action (action),
	m_instance (0)
      { }

      action_executor_impl (const action_executor_impl& other) :
	m_action (other.m_action),
	m_instance (other.m_instance)
      { }
      
      bool fetch_instance () {
	m_instance = system::automaton_handle_to_instance (m_action.automaton);
	return m_instance != 0;
      }
      
      void operator() (const VT& t) const {
	assert (m_instance != 0);
	system_scheduler::set_current_aid (m_action.get_aid (), *m_instance);
	m_action (*m_instance, t);
	system_scheduler::clear_current_aid ();
      }
    
      const action_interface& get_action () const {
	return m_action;
      }

      action_executor_impl* clone () const {
	return new action_executor_impl (*this);
      }      
    };
    
    template <class I, class M>
    class action_executor_impl<I, M, output_category, unvalued, null_type> :
      public unvalued_output_executor_interface
    {
    private:
      struct record
      {
	const unvalued_input_executor_interface* const m_input;
	const aid_t m_binder;
	void* const m_key;

	record (const unvalued_input_executor_interface* const input,
		const aid_t binder,
		void* const key) :
	  m_input (input),
	  m_binder (binder),
	  m_key (key)
	{
	  system_scheduler::bound (m_binder, m_key);
	  
	  // TODO
	  // system_scheduler::set_current_aid (m_output_action.get_aid (), m_output_ref);
	  // m_output_action.bound (m_output_ref);
	  // system_scheduler::clear_current_aid ();
	  
	  // system_scheduler::set_current_aid (m_input_action.get_aid (), m_input_ref);
	  // m_input_action.bound (m_input_ref);
	  // system_scheduler::clear_current_aid ();
	}

	~record () {
	  delete m_input;

	  system_scheduler::unbound (m_binder, m_key);

	  // TODO
	  // system_scheduler::set_current_aid (m_output_action.get_aid (), m_output_ref);
	  // m_output_action.unbound (m_output_ref);
	  // system_scheduler::clear_current_aid ();
	  
	  // system_scheduler::set_current_aid (m_input_action.get_aid (), m_input_ref);
	  // m_input_action.unbound (m_input_ref);
	  // system_scheduler::clear_current_aid ();
	}

      };

      const action<I, M> m_action;
      I* m_instance;
      std::map<aid_t, record*> m_records;
      
    public:
      action_executor_impl (const action<I, M>& action) :
	m_action (action),
	m_instance (0)
      { }

      action_executor_impl (const action_executor_impl& other) :
	m_action (other.m_action),
	m_instance (other.m_instance)
      { }

      ~action_executor_impl () {
	for (typename std::map<aid_t, record*>::const_iterator pos = m_records.begin ();
	     pos != m_records.end ();
	     ++pos) {
	  delete pos->second;
	}
      }
      
      bool fetch_instance () {
	m_instance = system::automaton_handle_to_instance (m_action.automaton);
	return m_instance != 0;
      }
      
      void operator() () const {
	assert (m_instance != 0);

	// Lock the automata in order.
	bool output_processed;
	
	// Lock in order.
	output_processed = false;
	for (typename std::map<aid_t, record*>::const_iterator pos = m_records.begin ();
	     pos != m_records.end ();
	     ++pos) {
	  if (!output_processed &&
	      m_action.get_aid () < pos->first) {
	    system::lock_automaton (m_action.get_aid ());
	    output_processed = true;
	  }
	  system::lock_automaton (pos->first);
	}


	// Execute.
	system_scheduler::set_current_aid (m_action.get_aid (), *m_instance);
	bool t = m_action (*m_instance);
	system_scheduler::clear_current_aid ();

	if (t) {
	  for (typename std::map<aid_t, record*>::const_iterator pos = m_records.begin ();
	       pos != m_records.end ();
	       ++pos) {
	    (*(pos->second->m_input)) ();
	  }	  
	}

	// Unlock.
	output_processed = false;
	for (typename std::map<aid_t, record*>::const_iterator pos = m_records.begin ();
	     pos != m_records.end ();
	     ++pos) {
	  if (!output_processed &&
	      m_action.get_aid () < pos->first) {
	    system::unlock_automaton (m_action.get_aid ());
	    output_processed = true;
	  }
	  system::unlock_automaton (pos->first);
	}
      }

      const action_interface& get_action () const {
	return m_action;
      }

      output_executor_interface* clone () const {
	return new action_executor_impl (*this);
      }

      bool involves_output (const action_interface& output) const {
	return m_action == output;
      }

      bool involves_input (const action_interface& input) const {
	typename std::map<aid_t, record*>::const_iterator pos = m_records.find (input.get_aid ());
	if (pos == m_records.end ()) {
	  return false;
	}
	else {
	  return pos->second->m_input->get_action () == input;
	}
      }

      bool involves_input_automaton (const aid_t aid) const {
	return m_records.find (aid) != m_records.end ();
      }

      bool involves_binding (const action_interface& output,
			     const action_interface& input,
			     const aid_t binder) const {
	typename std::map<aid_t, record*>::const_iterator pos = m_records.find (input.get_aid ());
	if (pos == m_records.end ()) {
	  return false;
	}
	else {
	  return m_action == output && pos->second->m_input->get_action () == input && pos->second->m_binder == binder;
	}
      }

      bool involves_aid_key (const aid_t binder,
			     void* const key) const {
	for (typename std::map<aid_t, record*>::const_iterator pos = m_records.begin ();
	     pos != m_records.end ();
	     ++pos) {
	  if (pos->second->m_binder == binder &&
	      pos->second->m_key == key) {
	    return true;
	  }
	}
	return false;
      }

      bool empty () const {
	return m_records.empty ();
      }

      void bind (const input_executor_interface& input,
		 const aid_t binder,
		 void* const key) {
	// TODO:  I think we can prove that this dynamic cast will always succeed.  Thus, we can make it a static cast.
	const unvalued_input_executor_interface* i = dynamic_cast<const unvalued_input_executor_interface*> (input.clone ());
	assert (i != 0);
	m_records.insert (std::make_pair (input.get_action ().get_aid (), new record (i, binder, key)));
      }

      void unbind (const aid_t binder,
		   void* const key) {
	for (typename std::map<aid_t, record*>::iterator pos = m_records.begin ();
	     pos != m_records.end ();
	     ) {
	  if (pos->second->m_binder == binder &&
	      pos->second->m_key == key) {
	    delete pos->second;
	    m_records.erase (pos++);
	    break;
	  }
	  else {
	    ++pos;
	  }
	}
      }

      void unbind_automaton (const aid_t aid) {
	if (m_action.get_aid () == aid) {
	  // We are the aid so delete everything.
	  for (typename std::map<aid_t, record*>::const_iterator pos = m_records.begin ();
	       pos != m_records.end ();
	       ++pos) {
	    delete pos->second;
	  }
	  m_records.clear ();
	}
	else {
	  // Look for the aid
	  for (typename std::map<aid_t, record*>::iterator pos = m_records.begin ();
	       pos != m_records.end ();
	       ) {
	    if (pos->first == aid ||
		pos->second->m_binder == aid) {
	      delete pos->second;
	      m_records.erase (pos++);
	    }
	    else {
	      ++pos;
	    }
	  }
	}
      }
      
    };

    template <class I, class M, class VT>
    class action_executor_impl<I, M, output_category, valued, VT> :
      public valued_output_executor_interface<VT>
    {
    private:
      struct record
      {
	const valued_input_executor_interface<VT>* const m_input;
	const aid_t m_binder;
	void* const m_key;

	record (const valued_input_executor_interface<VT>* const input,
		const aid_t binder,
		void* const key) :
	  m_input (input),
	  m_binder (binder),
	  m_key (key)
	{
	  system_scheduler::bound (m_binder, m_key);
	  
	  // TODO
	  // system_scheduler::set_current_aid (m_output_action.get_aid (), m_output_ref);
	  // m_output_action.bound (m_output_ref);
	  // system_scheduler::clear_current_aid ();
	  
	  // system_scheduler::set_current_aid (m_input_action.get_aid (), m_input_ref);
	  // m_input_action.bound (m_input_ref);
	  // system_scheduler::clear_current_aid ();
	}

	~record () {
	  delete m_input;

	  system_scheduler::unbound (m_binder, m_key);

	  // TODO
	  // system_scheduler::set_current_aid (m_output_action.get_aid (), m_output_ref);
	  // m_output_action.unbound (m_output_ref);
	  // system_scheduler::clear_current_aid ();
	  
	  // system_scheduler::set_current_aid (m_input_action.get_aid (), m_input_ref);
	  // m_input_action.unbound (m_input_ref);
	  // system_scheduler::clear_current_aid ();
	}

      };

      const action<I, M> m_action;
      I* m_instance;
      std::map<aid_t, record*> m_records;
      
    public:
      action_executor_impl (const action<I, M>& action) :
	m_action (action),
	m_instance (0)
      { }

      action_executor_impl (const action_executor_impl& other) :
	m_action (other.m_action),
	m_instance (other.m_instance)
      { }

      ~action_executor_impl () {
	for (typename std::map<aid_t, record*>::const_iterator pos = m_records.begin ();
	     pos != m_records.end ();
	     ++pos) {
	  delete pos->second;
	}
      }
      
      bool fetch_instance () {
	m_instance = system::automaton_handle_to_instance (m_action.automaton);
	return m_instance != 0;
      }
      
      void operator() () const {
	assert (m_instance != 0);

	// Lock the automata in order.
	bool output_processed;
	
	// Lock in order.
	output_processed = false;
	for (typename std::map<aid_t, record*>::const_iterator pos = m_records.begin ();
	     pos != m_records.end ();
	     ++pos) {
	  if (!output_processed &&
	      m_action.get_aid () < pos->first) {
	    system::lock_automaton (m_action.get_aid ());
	    output_processed = true;
	  }
	  system::lock_automaton (pos->first);
	}


	// Execute.
	system_scheduler::set_current_aid (m_action.get_aid (), *m_instance);
	std::pair<bool, VT> t = m_action (*m_instance);
	system_scheduler::clear_current_aid ();

	if (t.first) {
	  for (typename std::map<aid_t, record*>::const_iterator pos = m_records.begin ();
	       pos != m_records.end ();
	       ++pos) {
	    (*(pos->second->m_input)) (t.second);
	  }	  
	}

	// Unlock.
	output_processed = false;
	for (typename std::map<aid_t, record*>::const_iterator pos = m_records.begin ();
	     pos != m_records.end ();
	     ++pos) {
	  if (!output_processed &&
	      m_action.get_aid () < pos->first) {
	    system::unlock_automaton (m_action.get_aid ());
	    output_processed = true;
	  }
	  system::unlock_automaton (pos->first);
	}
      }

      const action_interface& get_action () const {
	return m_action;
      }

      output_executor_interface* clone () const {
	return new action_executor_impl (*this);
      }

      bool involves_output (const action_interface& output) const {
	return m_action == output;
      }

      bool involves_input (const action_interface& input) const {
	typename std::map<aid_t, record*>::const_iterator pos = m_records.find (input.get_aid ());
	if (pos == m_records.end ()) {
	  return false;
	}
	else {
	  return pos->second->m_input->get_action () == input;
	}
      }

      bool involves_input_automaton (const aid_t aid) const {
	return m_records.find (aid) != m_records.end ();
      }

      bool involves_binding (const action_interface& output,
			     const action_interface& input,
			     const aid_t binder) const {
	typename std::map<aid_t, record*>::const_iterator pos = m_records.find (input.get_aid ());
	if (pos == m_records.end ()) {
	  return false;
	}
	else {
	  return m_action == output && pos->second->m_input->get_action () == input && pos->second->m_binder == binder;
	}
      }

      bool involves_aid_key (const aid_t binder,
			     void* const key) const {
	for (typename std::map<aid_t, record*>::const_iterator pos = m_records.begin ();
	     pos != m_records.end ();
	     ++pos) {
	  if (pos->second->m_binder == binder &&
	      pos->second->m_key == key) {
	    return true;
	  }
	}
	return false;
      }

      bool empty () const {
	return m_records.empty ();
      }

      void bind (const input_executor_interface& input,
		 const aid_t binder,
		 void* const key) {
	// TODO:  I think we can prove that this dynamic cast will always succeed.  Thus, we can make it a static cast.
	const valued_input_executor_interface<VT>* i = dynamic_cast<const valued_input_executor_interface<VT>*> (input.clone ());
	assert (i != 0);
	m_records.insert (std::make_pair (input.get_action ().get_aid (), new record (i, binder, key)));
      }

      void unbind (const aid_t binder,
		   void* const key) {
	for (typename std::map<aid_t, record*>::iterator pos = m_records.begin ();
	     pos != m_records.end ();
	     ) {
	  if (pos->second->m_binder == binder &&
	      pos->second->m_key == key) {
	    delete pos->second;
	    m_records.erase (pos++);
	    break;
	  }
	  else {
	    ++pos;
	  }
	}
      }

      void unbind_automaton (const aid_t aid) {
	if (m_action.get_aid () == aid) {
	  // We are the aid so delete everything.
	  for (typename std::map<aid_t, record*>::const_iterator pos = m_records.begin ();
	       pos != m_records.end ();
	       ++pos) {
	    delete pos->second;
	  }
	  m_records.clear ();
	}
	else {
	  // Look for the aid
	  for (typename std::map<aid_t, record*>::iterator pos = m_records.begin ();
	       pos != m_records.end ();
	       ) {
	    if (pos->first == aid ||
		pos->second->m_binder == aid) {
	      delete pos->second;
	      m_records.erase (pos++);
	    }
	    else {
	      ++pos;
	    }
	  }
	}
      }

    };
    
    template <class I, class M>
    class action_executor_impl<I, M, internal_category, unvalued, null_type> :
      public internal_executor_interface
    {
    private:
      const action<I, M> m_action;
      I* m_instance;
      
    public:
      action_executor_impl (const action<I, M>& action) :
	m_action (action),
	m_instance (0)
      { }
      
      bool fetch_instance () {
	m_instance = system::automaton_handle_to_instance (m_action.automaton);
	return m_instance != 0;
      }
      
      void operator() () const {
	assert (m_instance != 0);

	system::lock_automaton (m_action.get_aid ());
	system_scheduler::set_current_aid (m_action.get_aid (), *m_instance);
	m_action (*m_instance);
	system_scheduler::clear_current_aid ();
	unlock_automaton (m_action.get_aid ());
      }
      
      const action_interface& get_action () const {
	return m_action;
      }
      
    };
    
    template <class I, class M, class VS, class VT>
    class action_executor_impl<I, M, event_category, VS, VT> :
      public event_executor_interface
    {
    private:
      const action<I, M> m_action;
      I* m_instance;
      
    public:
      action_executor_impl (const action<I, M>& action) :
	m_action (action),
	m_instance (0)
      { }
      
      bool fetch_instance () {
	m_instance = system::automaton_handle_to_instance (m_action.automaton);
	return m_instance != 0;
      }
      
      void operator() () const {
	assert (m_instance != 0);

	system::lock_automaton (m_action.get_aid ());
	system_scheduler::set_current_aid (m_action.get_aid (), *m_instance);
	m_action (*m_instance);
	system_scheduler::clear_current_aid ();
	unlock_automaton (m_action.get_aid ());
      }
      
      const action_interface& get_action () const {
	return m_action;
      }
      
    };
    
    template <class I, class M>
    class action_executor :
      public action_executor_impl<I, M, typename M::action_category, typename M::value_status, typename M::value_type>
    {
    public:
      action_executor (const action<I, M>& ac) :
	action_executor_impl<I, M, typename M::action_category, typename M::value_status, typename M::value_type> (ac)
      { }
    };

    template <class OI, class OM, class II, class IM>
    class bind_executor :
      public bind_executor_interface,
      private bind_check<typename OM::value_status, typename OM::value_type, typename IM::value_status, typename IM::value_type>
    {
    private:
      action_executor<OI, OM> m_output;
      action_executor<II, IM> m_input;

    public:
      bind_executor (const action<OI, OM>& output,
		     const action<II, IM>& input) :
	m_output (output),
	m_input (input)
      { }

      output_executor_interface& get_output () {
	return m_output;
      }

      input_executor_interface& get_input () {
	return m_input;
      }

    };
      
  };

}

#endif
