#ifndef __unbind_runnable_hpp__
#define __unbind_runnable_hpp__

#include <ioa/runnable_interface.hpp>
#include <ioa/model.hpp>

namespace ioa {
  
  class unbind_runnable :
    public runnable_interface
  {
  private:
    const aid_t m_automaton;
    void* const m_key;
    
  public:
    unbind_runnable (const aid_t automaton,
		     void* const key) :
      m_automaton (automaton),
      m_key (key)
    { }
    
    void operator() (model& model) {
      model.unbind (m_automaton, m_key);
    }
  };
  
}

#endif
