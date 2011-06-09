#ifndef __output_bound_runnable_hpp__
#define __output_bound_runnable_hpp__

#include <ioa/runnable_interface.hpp>
#include <ioa/system.hpp>

namespace ioa {

  class output_bound_runnable :
    public runnable_interface
  {
  private:
    std::auto_ptr<output_executor_interface> m_exec;
    
  public:
    output_bound_runnable (const output_executor_interface& exec) :
      m_exec (exec.clone ())
    { }
    
    void operator() () {
      system::execute_output_bound (*m_exec);
    }
  };

  class input_bound_runnable :
    public runnable_interface
  {
  private:
    std::auto_ptr<input_executor_interface> m_exec;
    
  public:
    input_bound_runnable (const input_executor_interface& exec) :
      m_exec (exec.clone ())
    { }
    
    void operator() () {
      system::execute_input_bound (*m_exec);
    }
  };

  class output_unbound_runnable :
    public runnable_interface
  {
  private:
    std::auto_ptr<output_executor_interface> m_exec;
    
  public:
    output_unbound_runnable (const output_executor_interface& exec) :
      m_exec (exec.clone ())
    { }
    
    void operator() () {
      system::execute_output_unbound (*m_exec);
    }
  };

  class input_unbound_runnable :
    public runnable_interface
  {
  private:
    std::auto_ptr<input_executor_interface> m_exec;
    
  public:
    input_unbound_runnable (const input_executor_interface& exec) :
      m_exec (exec.clone ())
    { }
    
    void operator() () {
      system::execute_input_unbound (*m_exec);
    }
  };
  
}

#endif
