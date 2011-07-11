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
    public typed_generator_interface<T>
  {
    typedef T result_type;

    T* operator() () const {
      return new T ();
    }
  };

  template <class T>
  const_shared_ptr<typed_generator_interface<T> > make_generator () {
    return const_shared_ptr<typed_generator_interface<T> > (new generator<T> ());
  }

  template <class T, typename A0>
  struct generator1 :
    public typed_generator_interface<T>
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

  template <class T, class A0>
  const_shared_ptr<typed_generator_interface<T> > make_generator (A0 a0) {
    return const_shared_ptr<typed_generator_interface<T> > (new generator1<T, A0> (a0));
  }

  template <class T, typename A0, typename A1>
  struct generator2 :
    public typed_generator_interface<T>
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

  template <class T, typename A0, typename A1>
  const_shared_ptr<typed_generator_interface<T> > make_generator (A0 a0, A1 a1) {
    return const_shared_ptr<typed_generator_interface<T> > (new generator2<T, A0, A1> (a0, a1));
  }

  template <class T, typename A0, typename A1, typename A2>
  struct generator3 :
    public typed_generator_interface<T>
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

  template <class T, typename A0, typename A1, typename A2>
  const_shared_ptr<typed_generator_interface<T> > make_generator (A0 a0, A1 a1, A2 a2) {
    return const_shared_ptr<typed_generator_interface<T> > (new generator3<T, A0, A1, A2> (a0, a1, a2));
  }

  template <class T, typename A0, typename A1, typename A2, typename A3>
  struct generator4 :
    public typed_generator_interface<T>
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

  template <class T, typename A0, typename A1, typename A2, typename A3>
  const_shared_ptr<typed_generator_interface<T> > make_generator (A0 a0, A1 a1, A2 a2, A3 a3) {
    return const_shared_ptr<typed_generator_interface<T> > (new generator4<T, A0, A1, A2, A3> (a0, a1, a2, a3));
  }

  template <class T, typename A0, typename A1, typename A2, typename A3, typename A4>
  struct generator5 :
    public typed_generator_interface<T>
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

  template <class T, typename A0, typename A1, typename A2, typename A3, typename A4>
  const_shared_ptr<typed_generator_interface<T> > make_generator (A0 a0, A1 a1, A2 a2, A3 a3, A4 a4) {
    return const_shared_ptr<typed_generator_interface<T> > (new generator5<T, A0, A1, A2, A3, A4> (a0, a1, a2, a3, a4));
  }


  template <class T, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5>
  struct generator6 :
    public typed_generator_interface<T>
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

  template <class T, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5>
  const_shared_ptr<typed_generator_interface<T> > make_generator (A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) {
    return const_shared_ptr<typed_generator_interface<T> > (new generator6<T, A0, A1, A2, A3, A4, A5> (a0, a1, a2, a3, a4, a5));
  }
  
  template <class T, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
  struct generator7 :
    public typed_generator_interface<T>
  {
    typedef T result_type;
    A0 m_a0;
    A1 m_a1;
    A2 m_a2;
    A3 m_a3;
    A4 m_a4;
    A5 m_a5;
    A6 m_a6;

    generator7 (A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6) :
      m_a0 (a0),
      m_a1 (a1),
      m_a2 (a2),
      m_a3 (a3),
      m_a4 (a4),
      m_a5 (a5),
      m_a6 (a6)
    { }

    T* operator() () const {
      return new T (m_a0, m_a1, m_a2, m_a3, m_a4, m_a5, m_a6);
    }
  };

  template <class T, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
  const_shared_ptr<typed_generator_interface<T> > make_generator (A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6) {
    return const_shared_ptr<typed_generator_interface<T> > (new generator7<T, A0, A1, A2, A3, A4, A5, A6> (a0, a1, a2, a3, a4, a5, a6));
  }

}

#endif
