#ifndef __generator_interface_hpp__
#define __generator_interface_hpp__

namespace ioa {

  /*
    A generator produces an automaton when invoked.
    The result should be delete'able.
  */

  template <class T>
  class generator_interface {
  public:
    typedef T result_type;
    virtual ~generator_interface () { }
    virtual T* operator() () = 0;
  };

}

#endif
