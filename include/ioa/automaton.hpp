#ifndef __automaton_hpp__
#define __automaton_hpp__

#include <ioa/shared_ptr.hpp>
#include <ioa/const_shared_ptr.hpp>
#include <ioa/action.hpp>
#include <set>
#include <ioa/generator_interface.hpp>
#include <ioa/executor_interface.hpp>
#include <ioa/action_wrapper.hpp>
#include <memory>

#define COMMA ,

namespace ioa {

  class system_automaton_manager_interface
  {
  public:
    virtual ~system_automaton_manager_interface () { }
    virtual std::auto_ptr<generator_interface> get_generator () = 0;
    virtual void instance_exists () = 0;
    virtual void automaton_created (const aid_t aid) = 0;
    virtual void automaton_destroyed () = 0;
  };

  class system_binding_manager_interface
  {
  public:
    virtual ~system_binding_manager_interface () { }
    virtual shared_ptr<bind_executor_interface> get_executor () const = 0;
    virtual void output_automaton_dne () = 0;
    virtual void input_automaton_dne () = 0;
    virtual void binding_exists () = 0;
    virtual void output_action_unavailable () = 0;
    virtual void input_action_unavailable () = 0;
    virtual void bound () = 0;
    virtual void unbound () = 0;
  };

  class automaton {
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

    bool m_self_destruct;

  public:
    automaton ();
    virtual ~automaton ();
    void create (system_automaton_manager_interface* helper);
    void bind (system_binding_manager_interface* helper);
    void unbind (system_binding_manager_interface* helper);
    void destroy (system_automaton_manager_interface* helper);
    void self_destruct ();

  private:
    void schedule () const;

  private:    
    bool sys_create_precondition () const;
    std::pair<generator_interface*, void*> sys_create_effect ();
  public:
    SYSTEM_OUTPUT (automaton, sys_create, std::pair<generator_interface* COMMA void*>);

  private:
    bool sys_bind_precondition () const;
    std::pair<shared_ptr<bind_executor_interface>, void*> sys_bind_effect ();
  public:
    SYSTEM_OUTPUT (automaton, sys_bind, std::pair<shared_ptr<bind_executor_interface> COMMA void*>);

  private:
    bool sys_unbind_precondition () const;
    void* sys_unbind_effect ();
  public:
    SYSTEM_OUTPUT (automaton, sys_unbind, void*);

  private:
    bool sys_destroy_precondition () const;
    void* sys_destroy_effect ();
  public:
    SYSTEM_OUTPUT (automaton, sys_destroy, void*);

  private:
    bool sys_self_destruct_precondition () const;
    void* sys_self_destruct_effect ();
  public:
    SYSTEM_OUTPUT (automaton, sys_self_destruct, void*);

  public:
    enum created_t {
      CREATE_KEY_EXISTS_RESULT,
      INSTANCE_EXISTS_RESULT,
      AUTOMATON_CREATED_RESULT,
    };
    struct created_arg_t {
      const created_t type;
      void* const key;
      const aid_t aid;

      created_arg_t (const created_t t,
		     void* const k,
		     const aid_t a) :
	type (t),
	key (k),
	aid (a) { }
    };
  private:
    void sys_created_effect (const created_arg_t &);
  public:
    SYSTEM_INPUT (automaton, sys_created, created_arg_t);

  public:
    enum bound_t {
      BIND_KEY_EXISTS_RESULT,
      OUTPUT_AUTOMATON_DNE_RESULT,
      INPUT_AUTOMATON_DNE_RESULT,
      BINDING_EXISTS_RESULT,
      OUTPUT_ACTION_UNAVAILABLE_RESULT,
      INPUT_ACTION_UNAVAILABLE_RESULT,
      BOUND_RESULT,
    };
  private:
    void sys_bound_effect (std::pair<bound_t COMMA void*> const &);
  public:
    SYSTEM_INPUT (automaton, sys_bound, std::pair<bound_t COMMA void*>);

  public:
    enum unbound_t {
      BIND_KEY_DNE_RESULT,
      UNBOUND_RESULT,
    };
  private:
    void sys_unbound_effect (std::pair<unbound_t COMMA void*> const &);
  public:
    SYSTEM_INPUT (automaton, sys_unbound, std::pair<unbound_t COMMA void*>);
    

  public:
    enum destroyed_t {
      CREATE_KEY_DNE_RESULT,
      AUTOMATON_DESTROYED_RESULT,
    };
  private:
    void sys_destroyed_effect (std::pair<destroyed_t COMMA void*> const &);
  public:
    SYSTEM_INPUT (automaton, sys_destroyed, std::pair<destroyed_t COMMA void*>);
  };

}

#endif
