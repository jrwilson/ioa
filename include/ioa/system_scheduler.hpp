#ifndef __system_scheduler_hpp__
#define __system_scheduler_hpp__

#include <ioa/aid.hpp>
#include <ioa/shared_ptr.hpp>
#include <ioa/generator_interface.hpp>

namespace ioa {

  /*
    Some actions are executed by the system and some are executed by bindings.
    Both need to advise the scheduler of the automaton that is currently executing.
    These functions must be defined by the scheduler implementation.
   */

  class system_scheduler
  {
  public:
    static void set_current_aid (const aid_t aid);
    static void clear_current_aid ();

    static void create (const aid_t automaton,
			shared_ptr<generator_interface> generator,
			void* const key);

    static void destroy (const aid_t automaton,
			 void* const key);

    static void create_key_exists (const aid_t automaton,
				   void* const key);

    static void instance_exists (const aid_t automaton,
				 void* const key);

    static void automaton_created (const aid_t automaton,
				   void* const key,
				   const aid_t child);

    static void bind_key_exists (const aid_t automaton,
				 void* const key);

    static void output_automaton_dne (const aid_t automaton,
				      void* const key);

    static void input_automaton_dne (const aid_t automaton,
				      void* const key);

    static void binding_exists (const aid_t automaton,
				void* const key);

    static void input_action_unavailable (const aid_t automaton,
					  void* const key);

    static void output_action_unavailable (const aid_t automaton,
					   void* const key);
    
    static void bound (const aid_t automaton,
		       void* const key);

    static void bind_key_dne (const aid_t automaton,
			      void* const key);

    static void unbound (const aid_t automaton,
			 void* const key);

    static void create_key_dne (const aid_t automaton,
				void* const key);

    static void automaton_destroyed (const aid_t automaton,
				     void* const key);
    
    static void recipient_dne (const aid_t automaton,
			       void* const key);

    static void event_delivered (const aid_t automaton,
				 void* const key);
  };
  
}

#endif
