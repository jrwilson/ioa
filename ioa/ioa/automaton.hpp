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
    virtual void parameter_exists (const generic_parameter_handle&) { }
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
    virtual void parameter_rescinded (const generic_parameter_handle&) { }
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

    generic_parameter_handle (const locker_key<void*>& key) :
      locker_key<void*> (key)
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

    generic_automaton_handle (const locker_key<automaton_interface*>& key) :
      locker_key<automaton_interface*> (key)
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

  template <class T>
  automaton_handle<T> cast_automaton (const generic_automaton_handle& handle)
  {
    locker_key<T*> key (handle.serial (), static_cast<T*> (handle.value ()));
    return automaton_handle<T> (key);
  }

  template <class T>
  parameter_handle<T> cast_parameter (const generic_parameter_handle& handle)
  {
    locker_key<T*> key (handle.serial (), static_cast<T*> (handle.value ()));
    return parameter_handle<T> (key);
  }

}

#endif
