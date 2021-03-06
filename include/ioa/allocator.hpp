/*
   Copyright 2011 Justin R. Wilson

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#ifndef __allocator_hpp__
#define __allocator_hpp__

#include <ioa/allocator_interface.hpp>
#include <memory>

namespace ioa {

  /*
    Utility classes for allocators of 0 and 1 arguments.
    TODO:  Define allocators for N arguments.
  */

  template <class T>
  class allocator :
    public typed_allocator_interface<T>
  {
  private:
    bool m_flag;

  public:
    typedef T result_type;

    allocator () :
      m_flag (false)
    { }

    T* operator() () {
      assert (!m_flag);
      m_flag = true;
      return new T ();
    }
  };

  template <class T>
  std::auto_ptr<typed_allocator_interface<T> > make_allocator () {
    return std::auto_ptr<typed_allocator_interface<T> > (new allocator<T> ());
  }

  template <class T, typename A0>
  class allocator1 :
    public typed_allocator_interface<T>
  {
  private:
    bool m_flag;
    A0 m_a0;

  public:
    typedef T result_type;

    allocator1 (A0 a0) :
      m_flag (false),
      m_a0 (a0)
    { }

    T* operator() () {
      assert (!m_flag);
      m_flag = true;
      return new T (m_a0);
    }
  };

  template <class T, class A0>
  std::auto_ptr<typed_allocator_interface<T> > make_allocator (A0 a0) {
    return std::auto_ptr<typed_allocator_interface<T> > (new allocator1<T, A0> (a0));
  }

  template <class T, typename A0, typename A1>
  class allocator2 :
    public typed_allocator_interface<T>
  {
  private:
    bool m_flag;
    A0 m_a0;
    A1 m_a1;

  public:
    typedef T result_type;

    allocator2 (A0 a0, A1 a1) :
      m_flag (false),
      m_a0 (a0),
      m_a1 (a1)
    { }

    T* operator() () {
      assert (!m_flag);
      m_flag = true;
      return new T (m_a0, m_a1);
    }
  };

  template <class T, typename A0, typename A1>
  std::auto_ptr<typed_allocator_interface<T> > make_allocator (A0 a0, A1 a1) {
    return std::auto_ptr<typed_allocator_interface<T> > (new allocator2<T, A0, A1> (a0, a1));
  }

  template <class T, typename A0, typename A1, typename A2>
  class allocator3 :
    public typed_allocator_interface<T>
  {
  private:
    bool m_flag;
    A0 m_a0;
    A1 m_a1;
    A2 m_a2;

  public:
    typedef T result_type;

    allocator3 (A0 a0, A1 a1, A2 a2) :
      m_flag (false),
      m_a0 (a0),
      m_a1 (a1),
      m_a2 (a2)
    { }

    T* operator() () {
      assert (!m_flag);
      m_flag = true;
      return new T (m_a0, m_a1, m_a2);
    }
  };

  template <class T, typename A0, typename A1, typename A2>
  std::auto_ptr<typed_allocator_interface<T> > make_allocator (A0 a0, A1 a1, A2 a2) {
    return std::auto_ptr<typed_allocator_interface<T> > (new allocator3<T, A0, A1, A2> (a0, a1, a2));
  }

  template <class T, typename A0, typename A1, typename A2, typename A3>
  class allocator4 :
    public typed_allocator_interface<T>
  {
  private:
    bool m_flag;
    A0 m_a0;
    A1 m_a1;
    A2 m_a2;
    A3 m_a3;

  public:
    typedef T result_type;

    allocator4 (A0 a0, A1 a1, A2 a2, A3 a3) :
      m_flag (false),
      m_a0 (a0),
      m_a1 (a1),
      m_a2 (a2),
      m_a3 (a3)
    { }

    T* operator() () {
      assert (!m_flag);
      m_flag = true;
      return new T (m_a0, m_a1, m_a2, m_a3);
    }
  };

  template <class T, typename A0, typename A1, typename A2, typename A3>
  std::auto_ptr<typed_allocator_interface<T> > make_allocator (A0 a0, A1 a1, A2 a2, A3 a3) {
    return std::auto_ptr<typed_allocator_interface<T> > (new allocator4<T, A0, A1, A2, A3> (a0, a1, a2, a3));
  }

  template <class T, typename A0, typename A1, typename A2, typename A3, typename A4>
  class allocator5 :
    public typed_allocator_interface<T>
  {
  private:
    bool m_flag;
    A0 m_a0;
    A1 m_a1;
    A2 m_a2;
    A3 m_a3;
    A4 m_a4;

  public:
    typedef T result_type;

    allocator5 (A0 a0, A1 a1, A2 a2, A3 a3, A4 a4) :
      m_flag (false),
      m_a0 (a0),
      m_a1 (a1),
      m_a2 (a2),
      m_a3 (a3),
      m_a4 (a4)
    { }

    T* operator() () {
      assert (!m_flag);
      m_flag = true;
      return new T (m_a0, m_a1, m_a2, m_a3, m_a4);
    }
  };

  template <class T, typename A0, typename A1, typename A2, typename A3, typename A4>
  std::auto_ptr<typed_allocator_interface<T> > make_allocator (A0 a0, A1 a1, A2 a2, A3 a3, A4 a4) {
    return std::auto_ptr<typed_allocator_interface<T> > (new allocator5<T, A0, A1, A2, A3, A4> (a0, a1, a2, a3, a4));
  }


  template <class T, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5>
  class allocator6 :
    public typed_allocator_interface<T>
  {
  private:
    bool m_flag;
    A0 m_a0;
    A1 m_a1;
    A2 m_a2;
    A3 m_a3;
    A4 m_a4;
    A5 m_a5;

  public:
    typedef T result_type;

    allocator6 (A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) :
      m_flag (false),
      m_a0 (a0),
      m_a1 (a1),
      m_a2 (a2),
      m_a3 (a3),
      m_a4 (a4),
      m_a5 (a5)
    { }

    T* operator() () {
      assert (!m_flag);
      m_flag = true;
      return new T (m_a0, m_a1, m_a2, m_a3, m_a4, m_a5);
    }
  };

  template <class T, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5>
  std::auto_ptr<typed_allocator_interface<T> > make_allocator (A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) {
    return std::auto_ptr<typed_allocator_interface<T> > (new allocator6<T, A0, A1, A2, A3, A4, A5> (a0, a1, a2, a3, a4, a5));
  }
  
  template <class T, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
  class allocator7 :
    public typed_allocator_interface<T>
  {
  private:
    bool m_flag;
    A0 m_a0;
    A1 m_a1;
    A2 m_a2;
    A3 m_a3;
    A4 m_a4;
    A5 m_a5;
    A6 m_a6;

  public:
    typedef T result_type;

    allocator7 (A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6) :
      m_flag (false),
      m_a0 (a0),
      m_a1 (a1),
      m_a2 (a2),
      m_a3 (a3),
      m_a4 (a4),
      m_a5 (a5),
      m_a6 (a6)
    { }

    T* operator() () {
      assert (!m_flag);
      m_flag = true;
      return new T (m_a0, m_a1, m_a2, m_a3, m_a4, m_a5, m_a6);
    }
  };

  template <class T, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
  std::auto_ptr<typed_allocator_interface<T> > make_allocator (A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6) {
    return std::auto_ptr<typed_allocator_interface<T> > (new allocator7<T, A0, A1, A2, A3, A4, A5, A6> (a0, a1, a2, a3, a4, a5, a6));
  }

}

#endif
