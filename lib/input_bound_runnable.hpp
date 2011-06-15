#ifndef __input_bound_runnable_hpp__
#define __input_bound_runnable_hpp__

#include <ioa/runnable_interface.hpp>

namespace ioa {

  class input_bound_runnable :
    public runnable_interface
  {
  private:
    std::auto_ptr<input_executor_interface> m_exec;
    
  public:
    input_bound_runnable (const input_executor_interface& exec) :
      m_exec (exec.clone ())
    { }
    
    void operator() (model_interface& model) {
      model.execute_input_bound (*m_exec);
    }
  };

}

#endif
