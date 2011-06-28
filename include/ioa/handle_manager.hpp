#ifndef __handle_manager_hpp__
#define __handle_manager_hpp__

#include <ioa/automaton_manager_interface.hpp>
#include <ioa/scheduler.hpp>

namespace ioa {
  
  template <class T>
  class handle_manager :
    public automaton_handle_interface<T>
  {
  private:
    automaton_handle<T> m_handle;

  public:
    typedef T instance;

    handle_manager () { }

    handle_manager (const automaton_handle<T>& handle) :
      m_handle (handle)
    { }
    
    automaton_handle<T> get_handle () const {
      return m_handle;
    }

    void destroy () {
      // This should never be called.
      assert (false);
    }
    
  };

}

#endif
