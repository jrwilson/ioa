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
    
    void operator() (model_interface& model) {
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
  
}

#endif
