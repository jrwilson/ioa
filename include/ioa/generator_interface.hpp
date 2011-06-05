#ifndef __generator_interface_hpp__
#define __generator_interface_hpp__

#include <ioa/automaton_interface.hpp>

namespace ioa {

  /*
    A generator produces an automaton when invoked.
    The result should be delete'able.
  */

  class generator_interface
  {
  public:
    virtual ~generator_interface () { }
    virtual automaton_interface* operator() () = 0;
  };

}

#endif
