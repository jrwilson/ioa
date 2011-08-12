#ifndef __automaton_manager_interface_hpp__
#define __automaton_manager_interface_hpp__

#include <ioa/automaton_handle.hpp>

namespace ioa {

  class automaton_manager_interface
  {
  public:
    enum state_t {
      START,
      INSTANCE_EXISTS,
      CREATED,
      DESTROYED
    };
    virtual ~automaton_manager_interface () { }
    virtual void destroy () = 0;
    virtual state_t get_state () const = 0;
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
