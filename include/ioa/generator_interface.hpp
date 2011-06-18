#ifndef __generator_interface_hpp__
#define __generator_interface_hpp__

namespace ioa {

  /*
    A generator produces an automaton when invoked.
    The result should be delete'able.
  */

  class automaton;

  class generator_interface
  {
  public:
    virtual ~generator_interface () { }
    virtual automaton* operator() () = 0;
  };

}

#endif
