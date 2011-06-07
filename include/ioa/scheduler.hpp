#ifndef __scheduler_hpp__
#define __scheduler_hpp__

#include <ioa/aid.hpp>
#include <ioa/time.hpp>
#include <ioa/shared_ptr.hpp>
#include <ioa/generator_interface.hpp>
#include <ioa/action.hpp>

namespace ioa {

  class scheduler
  {
  public:
    static aid_t get_current_aid ();
    
    template <class I, class M>
    static void schedule (M I::*member_ptr);
    
    template <class I, class M>
    static void schedule (M I::*member_ptr,
			  time offset);
    
    static void run (shared_ptr<generator_interface> generator);
    
    static void clear ();
  };

}

#include <ioa/simple_scheduler.hpp>

#endif
