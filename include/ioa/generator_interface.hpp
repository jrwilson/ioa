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
    virtual automaton* operator() () const = 0;
  };

  template <class I>
  class typed_generator_interface :
    public generator_interface
  {
  public:
    virtual ~typed_generator_interface () { }
    virtual I* operator() () const = 0;
  };

}

#endif
