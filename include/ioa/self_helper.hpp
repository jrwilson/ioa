#ifndef __self_helper_hpp__
#define __self_helper_hpp__

#include <ioa/automaton_helper_interface.hpp>
#include <ioa/scheduler.hpp>

namespace ioa {
  
  template <class T>
  class self_helper :
    public automaton_handle_interface<T>
  {
  private:
    automaton_handle<T> m_handle;

  public:
    typedef T instance;
    
    self_helper ()
    {
      m_handle = scheduler::get_current_aid ();
    }
    
    automaton_handle<T> get_handle () const {
      return m_handle;
    }

    void destroy () {
      // Delete to be consistent.
      delete this;
    }
    
  };

}

#endif
