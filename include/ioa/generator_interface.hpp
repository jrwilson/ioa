#ifndef __generator_interface_hpp__
#define __generator_interface_hpp__

namespace ioa {

  /*
    A generator produces an automaton when invoked.
    The result should be delete'able.
    Generators are owned by exactly one object at a time and will only be invoked once.  
  */

  class automaton;

  class generator_interface
  {
  public:
    virtual ~generator_interface () { }
    virtual generator_interface* clone () const = 0;
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
