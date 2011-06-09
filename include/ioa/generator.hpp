#ifndef __instance_generator_hpp__
#define __instance_generator_hpp__

#include <ioa/generator_interface.hpp>
#include <ioa/shared_ptr.hpp>

namespace ioa {

  /*
    Utility classes for generators of 0 and 1 arguments.
    TODO:  Define generators for N arguments.
  */

  template <class T>
  struct generator :
    public generator_interface
  {
    typedef T result_type;

    T* operator() () {
      return new T ();
    }
  };

  template <class I>
  shared_ptr<generator_interface> make_generator () {
    return shared_ptr<generator_interface> (new generator<I> ());
  }

  template <class T, typename A0>
  struct generator1 :
    public generator_interface
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
  shared_ptr<generator_interface> make_generator (A0 a0) {
    return shared_ptr<generator_interface> (new generator1<I, A0> (a0));
  }

  template <class T, typename A0, typename A1>
  struct generator2 :
    public generator_interface
  {
    typedef T result_type;
    A0 m_a0;
    A1 m_a1;

    generator2 (A0 a0, A1 a1) :
      m_a0 (a0),
      m_a1 (a1)
    { }

    T* operator() () {
      return new T (m_a0, m_a1);
    }
  };

  template <class I, typename A0, typename A1>
  shared_ptr<generator_interface> make_generator (A0 a0, A1 a1) {
    return shared_ptr<generator_interface> (new generator2<I, A0, A1> (a0, a1));
  }

  template <class T, typename A0, typename A1, typename A2>
  struct generator3 :
    public generator_interface
  {
    typedef T result_type;
    A0 m_a0;
    A1 m_a1;
    A2 m_a2;

    generator3 (A0 a0, A1 a1, A2 a2) :
      m_a0 (a0),
      m_a1 (a1),
      m_a2 (a2)
    { }

    T* operator() () {
      return new T (m_a0, m_a1, m_a2);
    }
  };

  template <class I, typename A0, typename A1, typename A2>
  shared_ptr<generator_interface> make_generator (A0 a0, A1 a1, A2 a2) {
    return shared_ptr<generator_interface> (new generator3<I, A0, A1, A2> (a0, a1, a2));
  }

}

#endif
