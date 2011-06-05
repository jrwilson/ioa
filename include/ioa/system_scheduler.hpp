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

    template <class M>
    static void schedule (const aid_t aid,
			  M automaton_interface::*member_ptr);

    template <class M, class T>
    static void schedule (const aid_t aid,
			  M automaton_interface::*member_ptr,
			  const T& t);
  };
  
}

#endif
