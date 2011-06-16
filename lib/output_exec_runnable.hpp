#ifndef __output_exec_runnable_hpp__
#define __output_exec_runnable_hpp__

namespace ioa {

  class output_exec_runnable :
    public action_runnable_interface
  {
  private:
    std::auto_ptr<output_executor_interface> m_exec;
    
  public:
    output_exec_runnable (const output_executor_interface& exec) :
      m_exec (exec.clone ())
    { }
    
    void operator() (model_interface& model) {
      model.execute (*m_exec);
    }

    const action_interface& get_action () const {
      return m_exec->get_action ();
    }

  };

}

#endif
