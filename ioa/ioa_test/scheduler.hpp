#ifndef __scheduler_hpp__
#define __scheduler_hpp__

namespace ioa {

  struct null_scheduler : public ioa::scheduler_interface {
    void set_current_automaton (ioa::automaton*) { }
  };

  null_scheduler null_sched;
  
  ioa::scheduler_interface& get_scheduler () {
    return null_sched;
  }

}

#endif
