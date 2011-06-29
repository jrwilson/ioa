#ifndef __system_scheduler_interface_hpp__
#define __system_scheduler_interface_hpp__

namespace ioa {

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
			 const_shared_ptr<generator_interface> generator,
			 void* const key) = 0;
    
    virtual void bind (const aid_t automaton,
		       shared_ptr<bind_executor_interface> exec,
		       void* const key) = 0;
    
    virtual void unbind (const aid_t automaton,
			 void* const key) = 0;
    
    virtual void destroy (const aid_t automaton,
			  void* const key) = 0;

    virtual void self_destruct (const aid_t automaton) = 0;
    
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
    
  };

}

#endif
