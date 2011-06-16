#ifndef __bind_runnable_hpp__
#define __bind_runnable_hpp__

#include <ioa/runnable_interface.hpp>

namespace ioa {

  class bind_runnable :
    public runnable_interface
  {
  private:
    const aid_t m_automaton;
    shared_ptr<bind_executor_interface> m_exec;
    void* const m_key;
    
  public:
    bind_runnable (const aid_t automaton,
		   shared_ptr<bind_executor_interface> exec,
		   void* const key) :
      m_automaton (automaton),
      m_exec (exec),
      m_key (key)
    { }
    
    void operator() (model_interface& model) {
      model.bind (m_automaton, m_exec, m_key);
    }
  };
  
}

#endif
