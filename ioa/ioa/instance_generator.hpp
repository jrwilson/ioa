#ifndef __instance_generator_hpp__
#define __instance_generator_hpp__

namespace ioa {
  
  template <class T>
  struct instance_generator
  {
    typedef T result_type;
    
    T* operator() () {
      return new T ();
    }
  };
  
  template <class T, class A0>
  struct instance_generator1
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

}

#endif
