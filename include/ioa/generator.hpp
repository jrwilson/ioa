#ifndef __instance_generator_hpp__
#define __instance_generator_hpp__

#include <ioa/generator_interface.hpp>
#include <ioa/const_shared_ptr.hpp>

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

    T* operator() () const {
      return new T ();
    }
  };

  template <class I>
  const_shared_ptr<generator_interface> make_generator () {
    return const_shared_ptr<generator_interface> (new generator<I> ());
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

    T* operator() () const {
      return new T (m_a0);
    }
  };

  template <class I, class A0>
  const_shared_ptr<generator_interface> make_generator (A0 a0) {
    return const_shared_ptr<generator_interface> (new generator1<I, A0> (a0));
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

    T* operator() () const {
      return new T (m_a0, m_a1);
    }
  };

  template <class I, typename A0, typename A1>
  const_shared_ptr<generator_interface> make_generator (A0 a0, A1 a1) {
    return const_shared_ptr<generator_interface> (new generator2<I, A0, A1> (a0, a1));
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

    T* operator() () const {
      return new T (m_a0, m_a1, m_a2);
    }
  };

  template <class I, typename A0, typename A1, typename A2>
  const_shared_ptr<generator_interface> make_generator (A0 a0, A1 a1, A2 a2) {
    return const_shared_ptr<generator_interface> (new generator3<I, A0, A1, A2> (a0, a1, a2));
  }

  template <class T, typename A0, typename A1, typename A2, typename A3>
  struct generator4 :
    public generator_interface
  {
    typedef T result_type;
    A0 m_a0;
    A1 m_a1;
    A2 m_a2;
    A3 m_a3;

    generator4 (A0 a0, A1 a1, A2 a2, A3 a3) :
      m_a0 (a0),
      m_a1 (a1),
      m_a2 (a2),
      m_a3 (a3)
    { }

    T* operator() () const {
      return new T (m_a0, m_a1, m_a2, m_a3);
    }
  };

  template <class I, typename A0, typename A1, typename A2, typename A3>
  const_shared_ptr<generator_interface> make_generator (A0 a0, A1 a1, A2 a2, A3 a3) {
    return const_shared_ptr<generator_interface> (new generator4<I, A0, A1, A2, A3> (a0, a1, a2, a3));
  }

  template <class T, typename A0, typename A1, typename A2, typename A3, typename A4>
  struct generator5 :
    public generator_interface
  {
    typedef T result_type;
    A0 m_a0;
    A1 m_a1;
    A2 m_a2;
    A3 m_a3;
    A4 m_a4;

    generator5 (A0 a0, A1 a1, A2 a2, A3 a3, A4 a4) :
      m_a0 (a0),
      m_a1 (a1),
      m_a2 (a2),
      m_a3 (a3),
      m_a4 (a4)
    { }

    T* operator() () const {
      return new T (m_a0, m_a1, m_a2, m_a3, m_a4);
    }
  };

  template <class I, typename A0, typename A1, typename A2, typename A3, typename A4>
  const_shared_ptr<generator_interface> make_generator (A0 a0, A1 a1, A2 a2, A3 a3, A4 a4) {
    return const_shared_ptr<generator_interface> (new generator5<I, A0, A1, A2, A3, A4> (a0, a1, a2, a3, a4));
  }


  template <class T, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5>
  struct generator6 :
    public generator_interface
  {
    typedef T result_type;
    A0 m_a0;
    A1 m_a1;
    A2 m_a2;
    A3 m_a3;
    A4 m_a4;
    A5 m_a5;

    generator6 (A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) :
      m_a0 (a0),
      m_a1 (a1),
      m_a2 (a2),
      m_a3 (a3),
      m_a4 (a4),
      m_a5 (a5)
    { }

    T* operator() () const {
      return new T (m_a0, m_a1, m_a2, m_a3, m_a4, m_a5);
    }
  };

  template <class I, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5>
  const_shared_ptr<generator_interface> make_generator (A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) {
    return const_shared_ptr<generator_interface> (new generator6<I, A0, A1, A2, A3, A4, A5> (a0, a1, a2, a3, a4, a5));
  }



}

#endif
