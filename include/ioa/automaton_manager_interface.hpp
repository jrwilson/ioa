#ifndef __automaton_manager_interface_hpp__
#define __automaton_manager_interface_hpp__

#include <ioa/automaton_handle.hpp>

namespace ioa {

  class automaton_manager_interface
  {
  public:
    virtual ~automaton_manager_interface () { }
    virtual void destroy () = 0;
  };
  
  template <class I>
  class automaton_handle_interface :
    public observable
  {
  public:
    virtual ~automaton_handle_interface () { }
    virtual automaton_handle<I> get_handle () const = 0;
  };


}

#endif
