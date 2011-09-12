#ifndef __instance_generator_hpp__
#define __instance_generator_hpp__

#include <ioa/generator_interface.hpp>
#include <memory>

namespace ioa {

  /*
    Utility classes for generators of 0 and 1 arguments.
    TODO:  Define generators for N arguments.
  */

  template <class T>
  class generator :
    public typed_generator_interface<T>
  {
  public:
    typedef T result_type;

    T* operator() () const {
      return new T ();
    }

    generator_interface* clone () const {
      return new generator (*this);
    }
  };

  template <class T>
  std::auto_ptr<typed_generator_interface<T> > make_generator () {
    return std::auto_ptr<typed_generator_interface<T> > (new generator<T> ());
  }

  template <class T, typename A0>
  class generator1 :
    public typed_generator_interface<T>
  {
  private:
    A0 m_a0;

  public:
    typedef T result_type;

    generator1 (A0 a0) :
      m_a0 (a0)
    { }

    T* operator() () const {
      return new T (m_a0);
    }
  };

  template <class T, class A0>
  std::auto_ptr<typed_generator_interface<T> > make_generator (A0 a0) {
    return std::auto_ptr<typed_generator_interface<T> > (new generator1<T, A0> (a0));
  }

  template <class T, typename A0, typename A1>
  class generator2 :
    public typed_generator_interface<T>
  {
  private:
    A0 m_a0;
    A1 m_a1;

  public:
    typedef T result_type;

    generator2 (A0 a0, A1 a1) :
      m_a0 (a0),
      m_a1 (a1)
    { }

    T* operator() () const {
      return new T (m_a0, m_a1);
    }

    generator_interface* clone () const {
      return new generator2 (*this);
    }
  };

  template <class T, typename A0, typename A1>
  std::auto_ptr<typed_generator_interface<T> > make_generator (A0 a0, A1 a1) {
    return std::auto_ptr<typed_generator_interface<T> > (new generator2<T, A0, A1> (a0, a1));
  }

  template <class T, typename A0, typename A1, typename A2>
  class generator3 :
    public typed_generator_interface<T>
  {
  private:
    A0 m_a0;
    A1 m_a1;
    A2 m_a2;

  public:
    typedef T result_type;

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
  std::auto_ptr<typed_generator_interface<T> > make_generator (A0 a0, A1 a1, A2 a2) {
    return std::auto_ptr<typed_generator_interface<T> > (new generator3<T, A0, A1, A2> (a0, a1, a2));
  }

  template <class T, typename A0, typename A1, typename A2, typename A3>
  class generator4 :
    public typed_generator_interface<T>
  {
  private:
    A0 m_a0;
    A1 m_a1;
    A2 m_a2;
    A3 m_a3;

  public:
    typedef T result_type;

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
  std::auto_ptr<typed_generator_interface<T> > make_generator (A0 a0, A1 a1, A2 a2, A3 a3) {
    return std::auto_ptr<typed_generator_interface<T> > (new generator4<T, A0, A1, A2, A3> (a0, a1, a2, a3));
  }

  template <class T, typename A0, typename A1, typename A2, typename A3, typename A4>
  class generator5 :
    public typed_generator_interface<T>
  {
  private:
    A0 m_a0;
    A1 m_a1;
    A2 m_a2;
    A3 m_a3;
    A4 m_a4;

  public:
    typedef T result_type;

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
  std::auto_ptr<typed_generator_interface<T> > make_generator (A0 a0, A1 a1, A2 a2, A3 a3, A4 a4) {
    return std::auto_ptr<typed_generator_interface<T> > (new generator5<T, A0, A1, A2, A3, A4> (a0, a1, a2, a3, a4));
  }


  template <class T, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5>
  class generator6 :
    public typed_generator_interface<T>
  {
  private:
    A0 m_a0;
    A1 m_a1;
    A2 m_a2;
    A3 m_a3;
    A4 m_a4;
    A5 m_a5;

  public:
    typedef T result_type;

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
  std::auto_ptr<typed_generator_interface<T> > make_generator (A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) {
    return std::auto_ptr<typed_generator_interface<T> > (new generator6<T, A0, A1, A2, A3, A4, A5> (a0, a1, a2, a3, a4, a5));
  }
  
  template <class T, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
  class generator7 :
    public typed_generator_interface<T>
  {
  private:
    A0 m_a0;
    A1 m_a1;
    A2 m_a2;
    A3 m_a3;
    A4 m_a4;
    A5 m_a5;
    A6 m_a6;

  public:
    typedef T result_type;

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
  std::auto_ptr<typed_generator_interface<T> > make_generator (A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6) {
    return std::auto_ptr<typed_generator_interface<T> > (new generator7<T, A0, A1, A2, A3, A4, A5, A6> (a0, a1, a2, a3, a4, a5, a6));
  }

}

#endif
