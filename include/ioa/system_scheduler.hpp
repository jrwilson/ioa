#ifndef __system_scheduler_hpp__
#define __system_scheduler_hpp__

namespace ioa {

  class system_scheduler
  {
  public:
    static void set_current_aid (const aid_t aid);
    static void set_current_aid (const aid_t aid,
				 const automaton_interface& current_this);
    static void clear_current_aid ();
  };
  
}

#endif
