#include <ioa/system.hpp>

namespace ioa {

  shared_mutex system::m_mutex;
  sequential_set<aid_t> system::m_aids;
  std::set<automaton_interface*> system::m_instances;
  std::map<aid_t, automaton_record_interface*> system::m_records;
  std::list<binding_interface*> system::m_bindings;

  // Implement automaton_locker.

  void automaton_locker::lock_automaton (const aid_t handle) {
    system::lock_automaton (handle);
  }

  void automaton_locker::unlock_automaton (const aid_t handle) {
    system::unlock_automaton (handle);
  }

}
