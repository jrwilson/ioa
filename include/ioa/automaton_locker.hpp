#ifndef __automaton_locker_hpp__
#define __automaton_locker_hpp__

namespace ioa {

  class automaton_locker {
  public:
    static void lock_automaton (const aid_t handle);
    static void unlock_automaton (const aid_t handle);
  };

}

#endif
