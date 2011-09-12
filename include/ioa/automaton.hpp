#ifndef __automaton_hpp__
#define __automaton_hpp__

#include <set>
#include <memory>

#include <ioa/action.hpp>
#include <ioa/automaton_base.hpp>
#include <ioa/generator_interface.hpp>
#include <ioa/executor_interface.hpp>
#include <ioa/action_wrapper.hpp>

namespace ioa {

  class action_interface
  {
  public:
    virtual ~action_interface () { }
    virtual aid_t get_aid () const = 0;
    virtual void* get_member_ptr () const = 0;
    virtual void* get_pid () const = 0;

    virtual bool operator== (const action_interface& x) const {
      return
    	get_aid () == x.get_aid () &&
    	get_member_ptr () == x.get_member_ptr () &&
    	get_pid () == x.get_pid ();
    }
    
    bool operator!= (const action_interface& x) const {
      return !(*this == x);
    }
    
    bool operator< (const action_interface& x) const {
      if (get_aid () != x.get_aid ()) {
    	return get_aid () < x.get_aid ();
      }
      if (get_member_ptr () != x.get_member_ptr ()) {
    	return get_member_ptr () < x.get_member_ptr ();
      }
      return get_pid () < x.get_pid ();
    }
  };

  class output_action_interface :
    public action_interface
  {
  public:
    virtual ~output_action_interface () { }
    virtual output_action_interface* clone () const = 0;
    virtual void set_auto_parameter (aid_t) = 0;
  };

  class input_action_interface :
    public action_interface
  {
  public:
    virtual ~input_action_interface () { }
    virtual input_action_interface* clone () const = 0;
    virtual void set_auto_parameter (aid_t) = 0;
  };

  class create_request_t
  {
  public:
    aid_t parent;
  private:
    generator_interface* m_generator;
  public:
    generator_interface& generator;
    void* key;
    
    create_request_t (const aid_t p,
		      const generator_interface& g,
		      void* const k) :
      parent (p),
      m_generator (g.clone ()),
      generator (*m_generator),
      key (k)
    { }

    ~create_request_t () {
      delete m_generator;
    }
  };
  
  enum create_result_t {
    CREATE_KEY_EXISTS_RESULT,
    INSTANCE_EXISTS_RESULT,
    AUTOMATON_CREATED_RESULT
  };
  
  struct create_response_t
  {
    aid_t const parent;
    create_result_t const result;
    void* const key;
    aid_t const child;
    
    create_response_t (aid_t const p,
		       create_result_t const r,
		       void* const k,
		       aid_t const c) :
      parent (p),
      result (r),
      key (k),
      child (c)
    { }
  };

  class bind_request_t
  {
  public:
    aid_t const owner;
  private:
    output_action_interface* m_output;
    input_action_interface* m_input;
  public:
    output_action_interface& output;
    input_action_interface& input;
    void* const key;

    bind_request_t (aid_t const ow,
		    const output_action_interface& o,
		    const input_action_interface& i,
		    void* const k) :
      owner (ow),
      m_output (o.clone ()),
      m_input (i.clone ()),
      output (*m_output),
      input (*m_input),
      key (k)
    { }

    ~bind_request_t () {
      delete m_output;
      delete m_input;
    }
  };
  
  enum bind_result_t {
    BIND_KEY_EXISTS_RESULT,
    OUTPUT_AUTOMATON_DNE_RESULT,
    INPUT_AUTOMATON_DNE_RESULT,
    INPUT_ACTION_UNAVAILABLE_RESULT,
    OUTPUT_ACTION_UNAVAILABLE_RESULT,
    BOUND_RESULT
  };
  
  struct bind_response_t
  {
    aid_t const owner;
    bind_result_t const result;
    void* const key;
    
    bind_response_t (aid_t const o,
		     bind_result_t const r,
		     void* const k) :
      owner (o),
      result (r),
      key (k)
    { }
  };

  struct unbind_request_t
  {
    aid_t const owner;
    void* const key;

    unbind_request_t (aid_t const o,
		      void* const k) :
      owner (o),
      key (k)
    { }
  };

  enum unbind_result_t {
    BIND_KEY_DNE_RESULT,
    UNBOUND_RESULT,
  };

  struct unbind_response_t
  {
    aid_t const owner;
    unbind_result_t const result;
    void* const key;

    unbind_response_t (aid_t const o,
		       unbind_result_t const r,
		       void* const k) :
      owner (o),
      result (r),
      key (k)
    { }
  };

  struct destroy_request_t
  {
    aid_t const owner;
    void* const key;

    destroy_request_t (aid_t const o,
		       void* const k) :
      owner (o),
      key (k)
    { }
  };

  enum destroy_result_t {
    CREATE_KEY_DNE_RESULT,
    AUTOMATON_DESTROYED_RESULT,
  };

  struct destroy_response_t
  {
    aid_t const owner;
    destroy_result_t result;
    void* const key;

    destroy_response_t (aid_t const o,
			destroy_result_t const r,
			void* const k) :
      owner (o),
      result (r),
      key (k)
    { }
  };

  class system_automaton_manager_interface
  {
  public:
    virtual ~system_automaton_manager_interface () { }
    virtual generator_interface& get_generator () const = 0;
    virtual void created (const create_result_t result, const aid_t aid) = 0;
    virtual void destroyed (const destroy_result_t result) = 0;
  };

  class system_binding_manager_interface
  {
  public:
    virtual ~system_binding_manager_interface () { }
    // TODO:  Axe this.
    virtual std::auto_ptr<bind_executor_interface> get_executor () const = 0;
    virtual output_action_interface& get_output () const = 0;
    virtual input_action_interface& get_input () const = 0;
    virtual void bound (const bind_result_t result) = 0;
    virtual void unbound (const unbind_result_t result) = 0;
  };

  class automaton :
    public automaton_base
  {
  private:
    // Invariant:  m_create_send INTERSECT m_create_recv INTERSECT m_destroy_send = empty set
    // Basically, a helper is in one place or another.
    // The send sets are waiting to send the command.
    // The receive set is waiting for a response.
    std::set<system_automaton_manager_interface*> m_create_send;
    std::set<system_automaton_manager_interface*> m_create_recv;
    std::set<system_automaton_manager_interface*> m_create_done;
    std::set<system_automaton_manager_interface*> m_destroy_send;
    std::set<system_automaton_manager_interface*> m_destroy_recv;
    // Same thing for binding.
    std::set<system_binding_manager_interface*> m_bind_send;
    std::set<system_binding_manager_interface*> m_bind_recv;
    std::set<system_binding_manager_interface*> m_bind_done;
    std::set<system_binding_manager_interface*> m_unbind_send;
    std::set<system_binding_manager_interface*> m_unbind_recv;

    // Automata can't be copied.
    automaton (const automaton&);
    automaton& operator= (const automaton&);

  public:
    automaton ();
    virtual ~automaton ();
    void create (system_automaton_manager_interface* helper);
    void bind (system_binding_manager_interface* helper);
    void unbind (system_binding_manager_interface* helper);
    void destroy (system_automaton_manager_interface* helper);

  private:
    void schedule () const;

  private:    
    bool create_request_precondition () const;
    create_request_t create_request_effect ();
    void create_request_schedule () const { schedule (); }
  public:
    V_UP_OUTPUT (automaton, create_request, create_request_t);

  private:
    bool bind_request_precondition () const;
    bind_request_t bind_request_effect ();
    void bind_request_schedule () const { schedule (); }
  public:
    V_UP_OUTPUT (automaton, bind_request, bind_request_t);

  private:
    bool unbind_request_precondition () const;
    unbind_request_t unbind_request_effect ();
    void unbind_request_schedule () const { schedule (); }
  public:
    V_UP_OUTPUT (automaton, unbind_request, unbind_request_t);

  private:
    bool destroy_request_precondition () const;
    destroy_request_t destroy_request_effect ();
    void destroy_request_schedule () const { schedule (); }
  public:
    V_UP_OUTPUT (automaton, destroy_request, destroy_request_t);
  private:
    void create_respond_effect (const create_response_t&);
    void create_respond_schedule () const { schedule (); }
  public:
    V_UP_INPUT (automaton, create_respond, create_response_t);

  private:
    void bind_respond_effect (const bind_response_t&);
    void bind_respond_schedule () const { schedule (); }
  public:
    V_UP_INPUT (automaton, bind_respond, bind_response_t);

  private:
    void unbind_respond_effect (const unbind_response_t&);
    void unbind_respond_schedule () const { schedule (); }
  public:
    V_UP_INPUT (automaton, unbind_respond, unbind_response_t);
    
  private:
    void destroy_respond_effect (const destroy_response_t&);
    void destroy_respond_schedule () const { schedule (); }
  public:
    V_UP_INPUT (automaton, destroy_respond, destroy_response_t);
  };

}

#endif
