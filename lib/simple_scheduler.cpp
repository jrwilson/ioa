#include <ioa/simple_scheduler.hpp>

namespace ioa {

  blocking_list<std::pair<bool, runnable_interface*> > simple_scheduler::m_sysq;
  blocking_list<std::pair<bool, action_runnable_interface*> > simple_scheduler::m_execq;
  int simple_scheduler::m_wakeup_fd[2];
  blocking_list<action_time> simple_scheduler::m_timerq;
  thread_key<aid_t> simple_scheduler::m_current_aid;
  thread_key<const automaton_interface*> simple_scheduler::m_current_this;

}
