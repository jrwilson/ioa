#ifndef __self_manager_hpp__
#define __self_manager_hpp__

#include <ioa/automaton_manager_interface.hpp>
#include <ioa/scheduler.hpp>

namespace ioa {
  
  template <class T>
  class self_manager :
    public automaton_handle_interface<T>
  {
  private:
    const automaton_handle<T> m_handle;

  public:
    typedef T instance;
    
    self_manager () :
      m_handle (ioa::get_aid ())
    { }
    
    automaton_handle<T> get_handle () const {
      return m_handle;
    }

    void destroy () {
      // Do nothing.
    }
    
  };

}

#endif
