#ifndef __automaton_hpp__
#define __automaton_hpp__

#include <boost/utility.hpp>
#include <boost/thread/mutex.hpp>
#include "locker.hpp"

namespace ioa {

  class generic_automaton_handle;
  class generic_parameter_handle;

  class automaton_interface
  {
  public:
    virtual ~automaton_interface () { }
    virtual void init () { }
    virtual void instance_exists (const void*) { }
    virtual void automaton_created (const generic_automaton_handle&) { }
    virtual void parameter_exists (void*) { }
    virtual void parameter_declared (const generic_parameter_handle&) { }
    virtual void bind_output_automaton_dne () { }
    virtual void bind_input_automaton_dne () { }
    virtual void bind_output_parameter_dne () { }
    virtual void bind_input_parameter_dne () { }
    virtual void binding_exists () { }
    virtual void input_action_unavailable () { }
    virtual void output_action_unavailable () { }
    virtual void bound () { }
    virtual void unbind_output_automaton_dne () { }
    virtual void unbind_input_automaton_dne () { }
    virtual void unbind_output_parameter_dne () { }
    virtual void unbind_input_parameter_dne () { }
    virtual void binding_dne () { }
    virtual void unbound () { }
    virtual void parameter_dne (const generic_parameter_handle&) { }
    virtual void parameter_rescinded (void*) { }
    virtual void target_automaton_dne (const generic_automaton_handle&) { }
    virtual void destroyer_not_creator (const generic_automaton_handle&) { }
    virtual void automaton_destroyed (const generic_automaton_handle&) { }
  };

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
    public locker_key<automaton_interface*>
  {
  public:
    generic_automaton_handle ()
      :
      locker_key<automaton_interface*> ()
    { }

    generic_automaton_handle (serial_type const serial,
			      automaton_interface* const value)
      :
      locker_key<automaton_interface*> (serial, value)
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

  class generic_automaton_record
    :
    public boost::mutex
  {
  private:
    locker<void*> m_parameters;
    
  public:
    virtual ~generic_automaton_record () { }
    
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
  class automaton_record :
    public generic_automaton_record
  {
  private:
    std::auto_ptr<T> m_instance;
    
  public:
    automaton_record (T* instance) :
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
