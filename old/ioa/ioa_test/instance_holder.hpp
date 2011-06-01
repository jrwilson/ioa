#ifndef __instance_holder_hpp__
#define __instance_holder_hpp__

template <class T>
class instance_holder :
  public ioa::generator_interface<T>
{
private:
  T* m_instance;

public:

  typedef T result_type;

  instance_holder (T* instance) :
    m_instance (instance)
  { }

  T* operator() () {
    return m_instance;
  }
  
};

#endif
