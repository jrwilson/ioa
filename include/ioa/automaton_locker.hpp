#ifndef __automaton_locker_hpp__
#define __automaton_locker_hpp__

namespace ioa {

  /*
    Some actions are executed by the system and some are executed by bindings.
    The system depends on the bindings for their implementation and the bindings depend on the system since it can lock automata.
    To break the circular dependency, we introduce a class for locking and unlocking automata.
    These functions are implemented by the system.
  */

  class automaton_locker {
  public:
    static void lock_automaton (const aid_t handle);
    static void unlock_automaton (const aid_t handle);
  };

}

#endif
