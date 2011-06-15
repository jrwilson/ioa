#ifndef __model_hpp__
#define __model_hpp__

#include <ioa/action.hpp>
#include <ioa/sequential_set.hpp>
#include <map>
#include <list>
#include <ioa/shared_ptr.hpp>
#include <ioa/automaton_record.hpp>
#include <ioa/shared_mutex.hpp>
#include <ioa/generator_interface.hpp>

// TODO:  Cleanup redundancy.

namespace ioa {

  class model;

  class action_executor_interface
  {
  public:
    virtual ~action_executor_interface () { }
    virtual bool fetch_instance (model&) = 0;
    virtual const action_interface& get_action () const = 0;
  };

  class input_executor_interface :
    public action_executor_interface
  {
  public:
    virtual ~input_executor_interface () { }
    virtual input_executor_interface* clone () const = 0;
    virtual void bound (model&, system_scheduler_interface&) const = 0;
    virtual void unbound (model&, system_scheduler_interface&) const = 0;
  };

  class unvalued_input_executor_interface :
    public input_executor_interface
  {
  public:
    virtual ~unvalued_input_executor_interface () { }
    virtual void operator() (system_scheduler_interface&) const = 0;
  };

  template <typename T>
  class valued_input_executor_interface :
    public input_executor_interface
  {
  public:
    virtual ~valued_input_executor_interface () { }
    virtual void operator() (system_scheduler_interface&, const T& t) const = 0;
  };

  class local_executor_interface :
    public action_executor_interface
  {
  public:
    virtual ~local_executor_interface () { }
    virtual void operator() (model&, system_scheduler_interface&) const = 0;
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
    virtual size_t size () const = 0;
    virtual void bind (system_scheduler_interface&,
		       const input_executor_interface&,
		       const aid_t,
		       void* const) = 0;
    virtual void unbind (const aid_t,
			 void* const) = 0;
    virtual void unbind_automaton (const aid_t) = 0;
    virtual void bound (model&,
			system_scheduler_interface&) const = 0;
    virtual void unbound (model&,
			  system_scheduler_interface&) const = 0;
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

  class system_input_executor_interface :
    public local_executor_interface
  {
  public:
    virtual ~system_input_executor_interface () { }
  };

  class bind_executor_interface
  {
  public:
    virtual ~bind_executor_interface () { }
    virtual output_executor_interface& get_output () = 0;
    virtual input_executor_interface& get_input () = 0;
  };

  /*
    Some actions are executed by the system and some are executed by bindings.
    Both need to advise the scheduler of the automaton that is currently executing.
  */

  class system_scheduler_interface
  {
  public:
    virtual ~system_scheduler_interface () { }
    virtual void set_current_aid (const aid_t aid) = 0;
    virtual void clear_current_aid () = 0;
    
    virtual void create (const aid_t automaton,
			 shared_ptr<generator_interface> generator,
			 void* const key) = 0;
    
    virtual void bind (const aid_t automaton,
		       shared_ptr<bind_executor_interface> exec,
		       void* const key) = 0;
    
    virtual void unbind (const aid_t automaton,
			 void* const key) = 0;
    
    virtual void destroy (const aid_t automaton,
			  void* const key) = 0;
    
    virtual void create_key_exists (const aid_t automaton,
				    void* const key) = 0;
    
    virtual void instance_exists (const aid_t automaton,
				  void* const key) = 0;
    
    virtual void automaton_created (const aid_t automaton,
				    void* const key,
				    const aid_t child) = 0;
    
    virtual void bind_key_exists (const aid_t automaton,
				  void* const key) = 0;
    
    virtual void output_automaton_dne (const aid_t automaton,
				       void* const key) = 0;
    
    virtual void input_automaton_dne (const aid_t automaton,
				      void* const key) = 0;
    
    virtual void binding_exists (const aid_t automaton,
				 void* const key) = 0;
    
    virtual void input_action_unavailable (const aid_t automaton,
					   void* const key) = 0;
    
    virtual void output_action_unavailable (const aid_t automaton,
					    void* const key) = 0;
    
    virtual void bound (const aid_t automaton,
			void* const key) = 0;
    
    virtual void output_bound (const output_executor_interface&) = 0;
    
    virtual void input_bound (const input_executor_interface&) = 0;
    
    virtual void bind_key_dne (const aid_t automaton,
			       void* const key) = 0;
    
    virtual void unbound (const aid_t automaton,
			  void* const key) = 0;
    
    virtual void output_unbound (const output_executor_interface&) = 0;
    
    virtual void input_unbound (const input_executor_interface&) = 0;
    
    virtual void create_key_dne (const aid_t automaton,
				 void* const key) = 0;
    
    virtual void automaton_destroyed (const aid_t automaton,
				      void* const key) = 0;
    
    virtual void recipient_dne (const aid_t automaton,
				void* const key) = 0;
    
    virtual void event_delivered (const aid_t automaton,
				  void* const key) = 0;
  };

  // TODO:  Memory allocation.

  class model
  {
  private:    
    
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

    system_scheduler_interface& m_system_scheduler;    
    shared_mutex m_mutex;
    sequential_set<aid_t> m_aids;
    std::set<automaton_interface*> m_instances;
    std::map<aid_t, automaton_record*> m_records;
    std::list<output_executor_interface*> m_bindings;

    void inner_destroy (automaton_record* automaton);
    
  public:
    model (system_scheduler_interface&);
    ~model ();

    void clear (void);
    aid_t create (shared_ptr<generator_interface> generator);
    aid_t create (const aid_t automaton,
			 shared_ptr<generator_interface> generator,
			 void* const key);
    int bind (const aid_t automaton,
		     shared_ptr<bind_executor_interface> exec,
		     void* const key);
    int unbind (const aid_t automaton,
		       void* const key);
    int destroy (const aid_t target);
    int destroy (const aid_t automaton,
			void* const key);
    int execute (output_executor_interface& exec);
    int execute (internal_executor_interface& exec);
    int execute (const aid_t from,
			event_executor_interface& exec,
			void* const key);
    int execute (system_input_executor_interface& exec);
    int execute_sys_create (const aid_t automaton);
    int execute_sys_bind (const aid_t automaton);
    int execute_sys_unbind (const aid_t automaton);
    int execute_sys_destroy (const aid_t automaton);
    int execute_output_bound (output_executor_interface& exec);
    int execute_input_bound (input_executor_interface& exec);
    int execute_output_unbound (output_executor_interface& exec);
    int execute_input_unbound (input_executor_interface& exec);

    size_t bind_count (const action_interface& action) const;

    template <class I>
    I* automaton_handle_to_instance (const automaton_handle<I>& handle) const {
      std::map<aid_t, automaton_record*>::const_iterator pos = m_records.find (handle);
      if (pos != m_records.end ()) {
	return dynamic_cast<I*> (pos->second->get_instance ());
      }
      else {
	return 0;
      }
    }

    void lock_automaton (const aid_t handle);
    void unlock_automaton (const aid_t handle);
  };

}

#endif
