#ifndef __automaton_hpp__
#define __automaton_hpp__

#include <boost/utility.hpp>
#include <boost/thread/mutex.hpp>
#include "locker.hpp"

namespace ioa {

  class generic_parameter_handle :
    public locker_key<void*>
  {
  public:
    generic_parameter_handle ()
      :
      locker_key<void*> ()
    { }

    generic_parameter_handle (serial_type const serial,
			      void* const value)
      :
      locker_key<void*> (serial, value)
    { }
  };

  template <class T>
  class parameter_handle :
    public locker_key<T*>
  {
  public:
    parameter_handle ()
    { }

    parameter_handle (const locker_key<T*>& key)
      :
      locker_key<T*> (key)
    { }

    operator generic_parameter_handle () const {
      return generic_parameter_handle (this->serial (), this->value ());
    }
  };

  class generic_automaton_handle :
    public locker_key<void*>
  {
  public:
    generic_automaton_handle ()
      :
      locker_key<void*> ()
    { }

    generic_automaton_handle (serial_type const serial,
			      void* const value)
      :
      locker_key<void*> (serial, value)
    { }
  };

  template <class T>
  class automaton_handle :
    public locker_key<T*>
  {
  public:
    automaton_handle ()
    { }

    automaton_handle (T* t) :
      locker_key<T*> (t)
    { }

    automaton_handle (const locker_key<T*>& key)
      :
      locker_key<T*> (key)
    { }

    operator generic_automaton_handle () const {
      return generic_automaton_handle (this->serial (), this->value ());
    }
  };

  class automaton_interface
    :
    public boost::mutex
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

    void rescind_parameter (const generic_parameter_handle& parameter) {
      m_parameters.erase (parameter);
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

}

#endif
