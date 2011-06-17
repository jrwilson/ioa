#ifndef __sys_create_runnable_hpp__
#define __sys_create_runnable_hpp__

#include <ioa/action_runnable.hpp>
#include <ioa/automaton_interface.hpp>

namespace ioa {
  
  class sys_create_runnable :
    public action_runnable_interface
  {
  private:
    const aid_t m_automaton;
    const action_executor<automaton_interface, automaton_interface::sys_create_type> m_action;
    
  public:
    sys_create_runnable (const aid_t automaton) :
      m_automaton (automaton),
      m_action (automaton, &automaton_interface::sys_create)
    { }
    
    void operator() (model_interface& model) {
      model.execute_sys_create (m_automaton);
    }

    const action_executor_interface& get_action () const {
      return m_action;
    }
  };
  
}

#endif
