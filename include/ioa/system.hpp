#ifndef __system_hpp__
#define __system_hpp__

#include <ioa/automaton_record.hpp>
#include <ioa/binding.hpp>

#include <ioa/generator_interface.hpp>
#include <ioa/unique_lock.hpp>
#include <ioa/shared_lock.hpp>

#include <map>
#include <list>
#include <iostream>

namespace ioa {

  class action_executor_interface
  {
  public:
    virtual ~action_executor_interface () { }
    virtual bool fetch_instance () = 0;
    virtual void operator() () const = 0;
    // TODO:  Remove this method from the interface and all descendants.
    virtual automaton_interface* get_instance () const = 0;
    virtual const action_interface& get_action () const = 0;
  };
  
  class output_executor_interface :
    public action_executor_interface
  {
  public:
    virtual ~output_executor_interface () { }
  };

  class input_executor_interface :
    public action_executor_interface
  {
  public:
    virtual ~input_executor_interface () { }
  };
  
  class internal_executor_interface :
    public action_executor_interface
  {
  public:
    virtual ~internal_executor_interface () { }
  };
  
  class event_executor_interface :
    public action_executor_interface
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
      
      bool operator() (const binding_interface* c) const {
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
      
      bool operator() (const binding_interface* c) const {
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
      
      bool operator() (const binding_interface* c) const {
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

      bool operator() (const binding_interface* c) const {
	return c->involves_aid_key (m_aid, m_key);
      }
    };
    
    static shared_mutex m_mutex;
    static sequential_set<aid_t> m_aids;
    static std::set<automaton_interface*> m_instances;
    static std::map<aid_t, automaton_record*> m_records;
    static std::list<binding_interface*> m_bindings;

    static void inner_destroy (automaton_record* automaton);
    static void execute0 (const action_executor_interface& exec);

    template <class I>
    static I* automaton_handle_to_instance (const automaton_handle<I>& handle) {
      if (!m_aids.contains (handle)) {
	return 0;
      }
      return dynamic_cast<I*> (m_records[handle]->get_instance ());
    }
    
  public:

    static void clear (void);
    static aid_t create (std::auto_ptr<generator_interface> generator);
    static aid_t create (const aid_t automaton,
			 std::auto_ptr<generator_interface> generator,
			 void* const key);

    static int
    bind (bind_executor_interface& bind_exec,
	  const aid_t binder,
	  void* const key)
    {
      unique_lock lock (m_mutex);

      if (!m_aids.contains (binder)) {
  	// Binder DNE.
  	return -1;
      }

      if (m_records[binder]->bind_key_exists (key)) {
	// Bind key already in use.
	system_scheduler::bind_key_exists (binder, key);
	return -1;
      }

      output_executor_interface& output = bind_exec.get_output ();
      input_executor_interface& input = bind_exec.get_input ();

      if (!output.fetch_instance ()) {
      	system_scheduler::output_automaton_dne (binder, key);
      	return -1;
      }

      if (!input.fetch_instance ()) {
      	system_scheduler::input_automaton_dne (binder, key);
      	return -1;
      }
      
      std::list<binding_interface*>::const_iterator pos = std::find_if (m_bindings.begin (),
      									m_bindings.end (),
      									binding_equal (output.get_action (), input.get_action (), binder));
      
      if (pos != m_bindings.end ()) {
      	// Bound.
      	system_scheduler::binding_exists (binder, key);
      	return -1;
      }
      
      std::list<binding_interface*>::const_iterator in_pos = std::find_if (m_bindings.begin (),
      									   m_bindings.end (),
      									   binding_input_equal (input.get_action ()));
      
      if (in_pos != m_bindings.end ()) {
      	// Input unavailable.
      	system_scheduler::input_action_unavailable (binder, key);
      	return -1;
      }
      
      std::list<binding_interface*>::const_iterator out_pos = std::find_if (m_bindings.begin (),
      									    m_bindings.end (),
      									    binding_output_equal (output.get_action ()));
      
      if (output.get_action ().get_aid () == input.get_action ().get_aid () ||
      	  (out_pos != m_bindings.end () && (*out_pos)->involves_input_automaton (input.get_action ().get_aid ()))) {
      	// Output unavailable.
      	system_scheduler::output_action_unavailable (binder, key);
      	return -1;
      }
      
      // binding* c;
      
      // if (out_pos != m_bindings.end ()) {
      // 	c = *out_pos;
      // }
      // else {
      // 	c = new binding ();
      // 	m_bindings.push_front (c);
      // }
    
      // // Bind.
      // c->bind (output, input, binder, key);
      // m_records[binder]->add_bind_key (key);

      return 0;
    }    

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

    static void lock_automaton (const aid_t handle);
    static void unlock_automaton (const aid_t handle);

    template <class I, class M, class K> class action_executor_impl;
    
    template <class I, class M>
    class action_executor_impl<I, M, output_category> :
      public output_executor_interface
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
	m_action (*m_instance);
      }
    
      automaton_interface* get_instance () const {
	return m_instance;
      }
      
      const action_interface& get_action () const {
	return m_action;
      }
      
    };

    template <class I, class M>
    class action_executor_impl<I, M, input_category> :
      public input_executor_interface
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
	m_action (*m_instance);
      }
    
      automaton_interface* get_instance () const {
	return m_instance;
      }
      
      const action_interface& get_action () const {
	return m_action;
      }
      
    };
    
    template <class I, class M>
    class action_executor_impl<I, M, internal_category> :
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
	m_action (*m_instance);
      }
      
      automaton_interface* get_instance () const {
	return m_instance;
      }
      
      const action_interface& get_action () const {
	return m_action;
      }
      
    };
    
    template <class I, class M>
    class action_executor_impl<I, M, event_category> :
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
	m_action (*m_instance);
      }
      
      automaton_interface* get_instance () const {
	return m_instance;
      }
      
      const action_interface& get_action () const {
	return m_action;
      }
      
    };
    
    template <class I, class M>
    class action_executor :
      public action_executor_impl<I, M, typename M::action_category>
    {
    public:
      action_executor (const action<I, M>& ac) :
	action_executor_impl<I, M, typename M::action_category> (ac)
      { }
    };


    template <class OI, class OM, class II, class IM>
    class bind_executor :
      public bind_executor_interface
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
