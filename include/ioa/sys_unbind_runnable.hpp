#ifndef __sys_unbind_runnable_hpp__
#define __sys_unbind_runnable_hpp__

#include <ioa/action_runnable.hpp>
#include <ioa/system.hpp>
#include <ioa/automaton_interface.hpp>

namespace ioa {
  
  class sys_unbind_runnable :
    public action_runnable_interface
  {
  private:
    const aid_t m_automaton;
    const action<automaton_interface, automaton_interface::sys_unbind_type> m_action;

  public:
    sys_unbind_runnable (const aid_t automaton) :
      m_automaton (automaton),
      m_action (automaton, &automaton_interface::sys_unbind)
    { }
    
    void operator() () {
      system::execute_sys_unbind (m_automaton);
    }

    const action_interface& get_action () const {
      return m_action;
    }
  };
  
}

#endif
