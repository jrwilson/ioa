#ifndef __automaton_helper_interface_hpp__
#define __automaton_helper_interface_hpp__

#include <ioa/automaton_handle.hpp>

namespace ioa {

  class automaton_helper_interface :
    public observable
  {
  public:
    virtual ~automaton_helper_interface () { }
    virtual void destroy () = 0;
  };
  
  template <class I>
  class automaton_handle_interface :
    public automaton_helper_interface
  {
  public:
    virtual ~automaton_handle_interface () { }
    virtual automaton_handle<I> get_handle () const = 0;
  };

}

#endif
