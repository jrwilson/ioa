#ifndef __system_scheduler_hpp__
#define __system_scheduler_hpp__

#include <ioa/aid.hpp>
#include <ioa/automaton_interface.hpp>

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
    static void set_current_aid (const aid_t aid,
				 const automaton_interface& current_this);
    static void clear_current_aid ();

    static void init (const aid_t automaton);

    static void instance_exists (const aid_t automaton,
				 void* aux);

    static void automaton_created (const aid_t automaton,
				   void* aux,
				   const aid_t child);

    static void output_automaton_dne (const aid_t binder,
				      void* aux);

    static void input_automaton_dne (const aid_t binder,
				      void* aux);

    static void binding_exists (const aid_t binder,
				void* aux);

    static void input_action_unavailable (const aid_t binder,
					  void* aux);

    static void output_action_unavailable (const aid_t binder,
					   void* aux);
    
    static void bound (const aid_t binder,
		       void* aux,
		       const bid_t bid);

    static void binding_dne (const aid_t binder,
			     void* aux);

    static void unbound (const aid_t binder,
			 const bid_t bid);

    static void target_automaton_dne (const aid_t automaton,
				      void* aux);

    static void destroyer_not_creator (const aid_t automaton,
				       void* aux);

    static void automaton_destroyed (const aid_t parent,
				     const aid_t child);

    static void recipient_dne (const aid_t from,
			       void* aux);
  };
  
}

#endif
