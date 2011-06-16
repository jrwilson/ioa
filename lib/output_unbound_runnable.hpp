#ifndef __output_unbound_runnable_hpp__
#define __output_unbound_runnable_hpp__

#include <ioa/runnable_interface.hpp>

namespace ioa {

  class output_unbound_runnable :
    public runnable_interface
  {
  private:
    std::auto_ptr<output_executor_interface> m_exec;
    
  public:
    output_unbound_runnable (const output_executor_interface& exec) :
      m_exec (exec.clone ())
    { }
    
    void operator() (model_interface& model) {
      model.execute_output_unbound (*m_exec);
    }
  };

}

#endif
