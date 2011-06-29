#ifndef __self_destruct_runnable_hpp__
#define __self_destruct_runnable_hpp__

#include <ioa/runnable_interface.hpp>

namespace ioa {
  
  class self_destruct_runnable :
    public runnable_interface
  {
  private:
    const aid_t m_automaton;
    
  public:
    self_destruct_runnable (const aid_t automaton) :
      m_automaton (automaton)
    { }
    
    void operator() (model_interface& model) {
      model.destroy (m_automaton);
    }
  };
  
}

#endif
