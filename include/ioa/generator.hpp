#ifndef __instance_generator_hpp__
#define __instance_generator_hpp__

#include <memory>
#include <ioa/generator_interface.hpp>

namespace ioa {
  
  /*
    Utility classes for generators of 0 and 1 arguments.
    TODO:  Define generators for N arguments.
  */
  
  template <class T>
  struct generator :
    public generator_interface<T>
  {
    typedef T result_type;
    
    T* operator() () {
      return new T ();
    }
  };

  template <class I>
  std::auto_ptr<ioa::generator_interface<I> > make_generator () {
    return std::auto_ptr<ioa::generator_interface<I> > (new ioa::generator<I> ());
  }
  
  template <class T, class A0>
  struct generator1 :
    public generator_interface<T>
  {
    typedef T result_type;
    A0 m_a0;
    
    generator1 (A0 a0) :
      m_a0 (a0)
    { }
    
    T* operator() () {
      return new T (m_a0);
    }
  };

  template <class I, class A0>
  std::auto_ptr<ioa::generator_interface<I> > make_generator (A0 a0) {
    return std::auto_ptr<ioa::generator_interface<I> > (new ioa::generator1<I, A0> (a0));
  }

}

#endif
