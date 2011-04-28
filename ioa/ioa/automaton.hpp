#ifndef __automaton_hpp__
#define __automaton_hpp__

#include <boost/utility.hpp>
#include "locker.hpp"

namespace ioa {

  typedef locker_key<void*> generic_parameter_handle;

  template <class T>
  class parameter_handle :
    public locker_key<T*>
  {
  public:
    parameter_handle (const locker_key<T*>& key)
      :
      locker_key<T*> (key)
    { }
  };

  class automaton_interface
    :
    private boost::noncopyable
  {
  private:
    locker<void*> m_parameters;
    
  public:
    virtual ~automaton_interface () { }
    
    virtual void* get_instance () const = 0;

    bool parameter_exists (void* parameter) const {
      return m_parameters.contains (parameter);
    }

    bool parameter_exists (const generic_parameter_handle& parameter) const {
      return m_parameters.contains (parameter);
    }
    
    template <class T>
    parameter_handle<T> declare_parameter (T* parameter) {
      return m_parameters.insert (parameter);
    }
  };

  template <class T>
  class automaton :
    public automaton_interface
  {
  private:
    std::auto_ptr<T> m_instance;
    
  public:
    automaton (T* instance) :
      m_instance (instance)
    { }
    
    void* get_instance () const {
      return m_instance.get ();
    }
    
    T* get_typed_instance () const {
      return m_instance.get ();
    }
  };

  typedef locker_key<automaton_interface*> generic_automaton_handle;

  template <class T>
  class automaton_handle :
    public locker_key<automaton<T>*>
  {
  public:
    automaton_handle (const locker_key<automaton<T>*>& key)
      :
      locker_key<automaton<T>*> (key)
    { }
    
  };

}

#endif
