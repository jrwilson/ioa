#ifndef __instance_generator_hpp__
#define __instance_generator_hpp__

namespace ioa {

  template <class T>
  class generator_interface {
  public:
    typedef T result_type;
    virtual ~generator_interface () { }
    virtual T* operator() () = 0;
  };
  
  template <class T>
  struct instance_generator :
    public generator_interface<T>
  {
    typedef T result_type;
    
    T* operator() () {
      return new T ();
    }
  };

  template <class I>
  std::auto_ptr<ioa::generator_interface<I> > make_instance_generator () {
    return std::auto_ptr<ioa::generator_interface<I> > (new ioa::instance_generator<I> ());
  }
  
  template <class T, class A0>
  struct instance_generator1 :
    public generator_interface<T>
  {
    typedef T result_type;
    A0 m_a0;
    
    instance_generator1 (A0 a0) :
      m_a0 (a0)
    { }
    
    T* operator() () {
      return new T (m_a0);
    }
  };

  template <class I, class A0>
  std::auto_ptr<ioa::generator_interface<I> > make_instance_generator (A0 a0) {
    return std::auto_ptr<ioa::generator_interface<I> > (new ioa::instance_generator1<I, A0> (a0));
  }

}

#endif
