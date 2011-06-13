#ifndef __action_runnable_hpp__
#define __action_runnable_hpp__

#include <ioa/runnable_interface.hpp>
#include <ioa/model.hpp>

namespace ioa {

  class action_runnable_interface :
    public runnable_interface
  {
  public:
    virtual ~action_runnable_interface () { }
    
    virtual const action_interface& get_action () const = 0;
    
    bool operator== (const action_runnable_interface& x) const {
      return get_action () == x.get_action ();
    }
  };

  template <class I, class M>
  class action_runnable :
    public action_runnable_interface
  {
  private:
    model::action_executor<I, M> m_exec;
    
  public:
    action_runnable (const action<I, M> action) :
      m_exec (action)
    { }
    
    void operator() () {
      model::execute (m_exec);
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
