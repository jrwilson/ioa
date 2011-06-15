#ifndef __destroy_runnable_hpp__
#define __destroy_runnable_hpp__

#include <ioa/runnable_interface.hpp>
#include <ioa/model.hpp>

namespace ioa {
  
  class destroy_runnable :
    public runnable_interface
  {
  private:
    const aid_t m_automaton;
    void* const m_key;
    
  public:
    destroy_runnable (const aid_t automaton,
		      void* const key) :
      m_automaton (automaton),
      m_key (key)
    { }
    
    void operator() (model& model) {
      model.destroy (m_automaton, m_key);
    }
  };
  
}

#endif
