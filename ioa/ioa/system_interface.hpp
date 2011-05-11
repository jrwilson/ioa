#ifndef __system_interface_hpp__
#define __system_interface_hpp__

#include "runnable.hpp"

namespace ioa {

  class system_interface
  {
  public:
    virtual ~system_interface () { }
    virtual void lock_automaton (const generic_automaton_handle& handle) = 0;
    virtual void unlock_automaton (const generic_automaton_handle& handle) = 0;
  };

  class automaton_dne_interface
  {
  public:
    virtual ~automaton_dne_interface () { }
    virtual void automaton_dne () = 0;    
  };

  class create_listener_interface :
    public automaton_dne_interface
  {
  public:
    virtual ~create_listener_interface () { }
    virtual void instance_exists (const void*) = 0;
    virtual void automaton_created (const generic_automaton_handle&) = 0;
    virtual void instance_exists (const generic_automaton_handle&,
				  const void*) = 0;
    virtual void automaton_created (const generic_automaton_handle&,
				    const generic_automaton_handle&) = 0;
  };
  
  class declare_listener_interface :
    public automaton_dne_interface
  {
  public:
    virtual ~declare_listener_interface () { }
    virtual void parameter_exists (const generic_automaton_handle&,
				   void*) = 0;
    virtual void parameter_declared (const generic_automaton_handle&,
				     const generic_parameter_handle&) = 0;
  };

  class bind_listener_interface :
    public automaton_dne_interface
  {
  public:
    virtual ~bind_listener_interface () { }
    virtual void bind_output_automaton_dne (const generic_automaton_handle& binder) = 0;
    virtual void bind_input_automaton_dne (const generic_automaton_handle& binder) = 0;
    virtual void bind_output_parameter_dne (const generic_automaton_handle& binder) = 0;
    virtual void bind_input_parameter_dne (const generic_automaton_handle& binder) = 0;
    virtual void binding_exists (const generic_automaton_handle& binder) = 0;
    virtual void input_action_unavailable (const generic_automaton_handle& binder) = 0;
    virtual void output_action_unavailable (const generic_automaton_handle& binder) = 0;
    virtual void bound (const generic_automaton_handle& binder) = 0;
  };

  class unbind_listener_interface :
    public automaton_dne_interface
  {
  public:
    virtual ~unbind_listener_interface () { }
    virtual void unbind_output_automaton_dne (const generic_automaton_handle& binder) = 0;
    virtual void unbind_input_automaton_dne (const generic_automaton_handle& binder) = 0;
    virtual void unbind_output_parameter_dne (const generic_automaton_handle& binder) = 0;
    virtual void unbind_input_parameter_dne (const generic_automaton_handle& binder) = 0;
    virtual void binding_dne (const generic_automaton_handle& binder) = 0;
    virtual void unbound (const generic_automaton_handle& binder) = 0;
  };

  class unbound_listener_interface
  {
  public:
    virtual ~unbound_listener_interface () { }
    virtual void unbound (const generic_automaton_handle& output_automaton,
			  const void* output_member_ptr,
			  const generic_automaton_handle& input_automaton,
			  const void* input_member_ptr,
			  const generic_automaton_handle& binder_automaton) = 0;
    virtual void unbound (const generic_automaton_handle& output_automaton,
			  const void* output_member_ptr,
			  const generic_parameter_handle& output_parameter,
			  const generic_automaton_handle& input_automaton,
			  const void* input_member_ptr) = 0;
    virtual void unbound (const generic_automaton_handle& output_automaton,
			  const void* output_member_ptr,
			  const generic_automaton_handle& input_automaton,
			  const void* input_member_ptr,
			  const generic_parameter_handle& input_parameter) = 0;
  };

  class rescind_listener_interface :
    public automaton_dne_interface,
    public unbound_listener_interface
  {
  public:
    virtual ~rescind_listener_interface () { }
    virtual void parameter_dne (const generic_automaton_handle&,
				const generic_parameter_handle&) = 0;
    virtual void parameter_rescinded (const generic_automaton_handle&,
				      void*) = 0;
  };
  
  class destroy_listener_interface :
    public automaton_dne_interface,
    public unbound_listener_interface
  {
  public:
    virtual ~destroy_listener_interface () { }
    virtual void target_automaton_dne (const generic_automaton_handle&) = 0;
    virtual void target_automaton_dne (const generic_automaton_handle&,
				       const generic_automaton_handle&) = 0;
    virtual void destroyer_not_creator (const generic_automaton_handle&,
					const generic_automaton_handle&) = 0;
    virtual void automaton_destroyed (const generic_automaton_handle&) = 0;
    virtual void automaton_destroyed (const generic_automaton_handle& parent,
				      const generic_automaton_handle& child) = 0;
  };

  class execute_listener_interface :
    public automaton_dne_interface
  {
  public:
    virtual ~execute_listener_interface () { }
    virtual void execute_parameter_dne () = 0;
  };

  class system_execute_listener_interface :
    public automaton_dne_interface
  {
  public:
    virtual ~system_execute_listener_interface () { }
  };

  class scheduler_interface
  {
  public:
    virtual ~scheduler_interface () { }
    virtual void set_current_handle (const generic_automaton_handle& handle) = 0;
    virtual void clear_current_handle () = 0;
  };

  class internal_scheduler_interface :
    public scheduler_interface,
    public create_listener_interface,
    public declare_listener_interface,
    public bind_listener_interface,
    public unbind_listener_interface,
    public rescind_listener_interface,
    public destroy_listener_interface,
    public execute_listener_interface,
    public system_execute_listener_interface
  {
  public:
    virtual void schedule (runnable_interface*) = 0;
  };

}

#endif
