#ifndef __ioa_hpp__
#define __ioa_hpp__

#include <memory>
#include <set>
#include <queue>
#include <boost/foreach.hpp>
#include <boost/thread.hpp>

#include "scheduler.hpp"
#include "simple_scheduler.hpp"

namespace ioa {

  template <class C, class T, void (C::*member)(const T)>
  class input_wrapper :
    public input,
    public value<T>,
    public no_parameter
 {
  private:
    C& m_c;
    
  public:
    input_wrapper (C& c)
      : m_c (c) { }
    
    void operator() (const T t) {
      (m_c.*member) (t);
    }

   void bound () { }
   void unbound () { }
  };

  // TODO: Combine with previous.
  template <class C, void (C::*member)()>
  class void_input_wrapper :
    public input,
    public no_value,
    public no_parameter
  {
  private:
    C& m_c;
    
  public:
    void_input_wrapper (C& c)
      : m_c (c) { }
    
    void operator() () {
      (m_c.*member) ();
    }


    void bound () {
    }
    void unbound () {
    }
  };

  template <class C, class P, void (C::*member)(P*)>
  class void_parameter_input_wrapper :
    public input,
    public no_value,
    public parameter<P>
  {
  private:
    C& m_c;
    
  public:
    void_parameter_input_wrapper (C& c)
      : m_c (c) { }
    
    void operator() (P* p) {
      (m_c.*member) (p);
    }

    void bound (P*) {
    }

    void unbound (P*) {
    }
  };
  
  template <class C, class T, std::pair<bool, T> (C::*member)(void)>
  class output_wrapper :
    public output,
    public value<T>,
    public no_parameter
 {
  private:
    C& m_c;
    
  public:
    output_wrapper (C& c) :
      m_c (c) { }
    
    std::pair<bool, T> operator() () {
      return (m_c.*member) ();
    }

    void bound () {
    }

    void unbound () {
    }
  };

  template <class C, bool (C::*member)(void)>
  class void_output_wrapper :
    public output,
    public no_value,
    public no_parameter
  {
  private:
    C& m_c;
    
  public:
    void_output_wrapper (C& c) :
      m_c (c) { }
    
    bool operator() () {
      return (m_c.*member) ();
    }

    void bound () {
    }
    void unbound () {
    }
  };

  template <class C, class P, bool (C::*member)(P*)>
  class void_parameter_output_wrapper :
    public output,
    public no_value,
    public parameter<P>
  {
  private:
    C& m_c;
    
  public:
    void_parameter_output_wrapper (C& c) :
      m_c (c) { }
    
    bool operator() (P* p) {
      return (m_c.*member) (p);
    }

    void bound (P*) {
    }

    void unbound (P*) {
    }
  };

  template <class C, void (C::*member)()>
  class internal_wrapper :
    public internal,
    public no_parameter
 {
  private:
    C& m_c;
    
  public:
    internal_wrapper (C& c)
      : m_c (c) { }
    
    void operator() () {
      (m_c.*member) ();
    }
  };

  simple_scheduler ss;
  scheduler_wrapper<simple_scheduler> scheduler (ss);
}

#endif
