#ifndef __automaton_hpp__
#define __automaton_hpp__

#include <ioa/shared_ptr.hpp>
#include <ioa/const_shared_ptr.hpp>
#include <ioa/action.hpp>
#include <set>
#include <ioa/generator_interface.hpp>
#include <ioa/executor_interface.hpp>
#include <ioa/action_wrapper.hpp>

#define COMMA ,

namespace ioa {

  class system_automaton_manager_interface
  {
  public:
    virtual ~system_automaton_manager_interface () { }
    virtual const_shared_ptr<generator_interface> get_generator () const = 0;
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

    bool sys_create_precondition () const;
    std::pair<const_shared_ptr<generator_interface>, void*> sys_create_effect ();

    bool sys_bind_precondition () const;
    std::pair<shared_ptr<bind_executor_interface>, void*> sys_bind_effect ();

    bool sys_unbind_precondition () const;
    void* sys_unbind_effect ();

    bool sys_destroy_precondition () const;
    void* sys_destroy_effect ();

    bool sys_self_destruct_precondition () const;
    void* sys_self_destruct_effect ();

    void sys_create_key_exists_effect (void* const &);
    void sys_instance_exists_effect (void* const &);
    void sys_automaton_created_effect (std::pair<void* COMMA aid_t> const &);
    void sys_bind_key_exists_effect (void* const &);
    void sys_output_automaton_dne_effect (void* const &);
    void sys_input_automaton_dne_effect (void* const &);
    void sys_binding_exists_effect (void* const &);
    void sys_output_action_unavailable_effect (void* const &);
    void sys_input_action_unavailable_effect (void* const &);
    void sys_bound_effect (void* const &);
    void sys_bind_key_dne_effect (void* const &);
    void sys_unbound_effect (void* const &);
    void sys_create_key_dne_effect (void* const &);
    void sys_automaton_destroyed_effect (void* const &);

  public:
    SYSTEM_OUTPUT (automaton, sys_create, std::pair<const_shared_ptr<generator_interface> COMMA void*>);
    SYSTEM_OUTPUT (automaton, sys_bind, std::pair<shared_ptr<bind_executor_interface> COMMA void*>);
    SYSTEM_OUTPUT (automaton, sys_unbind, void*);
    SYSTEM_OUTPUT (automaton, sys_destroy, void*);
    SYSTEM_OUTPUT (automaton, sys_self_destruct, void*);

    SYSTEM_INPUT (automaton, sys_create_key_exists, void*);
    SYSTEM_INPUT (automaton, sys_instance_exists, void*);
    SYSTEM_INPUT (automaton, sys_automaton_created, std::pair<void* COMMA aid_t>);
    SYSTEM_INPUT (automaton, sys_bind_key_exists, void*);
    SYSTEM_INPUT (automaton, sys_output_automaton_dne, void*);
    SYSTEM_INPUT (automaton, sys_input_automaton_dne, void*);
    SYSTEM_INPUT (automaton, sys_binding_exists, void*);
    SYSTEM_INPUT (automaton, sys_output_action_unavailable, void*);
    SYSTEM_INPUT (automaton, sys_input_action_unavailable, void*);
    SYSTEM_INPUT (automaton, sys_bound, void*);
    SYSTEM_INPUT (automaton, sys_bind_key_dne, void*);
    SYSTEM_INPUT (automaton, sys_unbound, void*);
    SYSTEM_INPUT (automaton, sys_create_key_dne, void*);
    SYSTEM_INPUT (automaton, sys_automaton_destroyed, void*);
  };

}

#endif
