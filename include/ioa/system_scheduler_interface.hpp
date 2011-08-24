#ifndef __system_scheduler_interface_hpp__
#define __system_scheduler_interface_hpp__

#include <ioa/automaton.hpp>
#include <memory>

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
			 std::auto_ptr<generator_interface> generator,
			 void* const key) = 0;
    
    virtual void bind (const aid_t automaton,
		       shared_ptr<bind_executor_interface> exec,
		       void* const key) = 0;
    
    virtual void unbind (const aid_t automaton,
			 void* const key) = 0;
    
    virtual void destroy (const aid_t automaton,
			  void* const key) = 0;

    virtual void created (const aid_t automaton,
			  const created_t,
			  void* const key,
			  const aid_t child) = 0;
    
    virtual void bound (const aid_t automaton,
			const bound_t,
			void* const key) = 0;
    
    virtual void output_bound (const output_executor_interface&) = 0;
    
    virtual void input_bound (const input_executor_interface&) = 0;
    
    virtual void unbound (const aid_t automaton,
			  const unbound_t,
			  void* const key) = 0;
    
    virtual void output_unbound (const output_executor_interface&) = 0;
    
    virtual void input_unbound (const input_executor_interface&) = 0;
    
    virtual void destroyed (const aid_t automaton,
			    const destroyed_t,
			    void* const key) = 0;
  };

}

#endif
