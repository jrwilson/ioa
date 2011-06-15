#ifndef __action_runnable_hpp__
#define __action_runnable_hpp__

#include <ioa/runnable_interface.hpp>
#include <ioa/action_executor.hpp>

namespace ioa {

  template <class I, class M>
  class action_runnable :
    public action_runnable_interface
  {
  private:
    action_executor<I, M> m_exec;
    
  public:
    action_runnable (const action<I, M> action) :
      m_exec (action)
    { }
    
    void operator() (model& model) {
      model.execute (m_exec);
    }
    
    const action_interface& get_action () const {
      return m_exec.get_action ();
    }
  };
  
  template <class I, class M>
  action_runnable<I, M>* make_action_runnable (const action<I, M> action) {
    return new action_runnable<I, M> (action);
  }

  class output_exec_runnable :
    public action_runnable_interface
  {
  private:
    std::auto_ptr<output_executor_interface> m_exec;
    
  public:
    output_exec_runnable (const output_executor_interface& exec) :
      m_exec (exec.clone ())
    { }
    
    void operator() (model& model) {
      model.execute (*m_exec);
    }

    const action_interface& get_action () const {
      return m_exec->get_action ();
    }

  };
  
}

#endif
