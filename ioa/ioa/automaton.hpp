#ifndef __automaton_hpp__
#define __automaton_hpp__

#include <boost/utility.hpp>
#include <boost/thread/mutex.hpp>
#include "locker.hpp"

namespace ioa {

  class generic_automaton_handle;
  class generic_parameter_handle;

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
    public locker_key<void*>
  {
  public:
    generic_automaton_handle ()
      :
      locker_key<void*> ()
    { }

    generic_automaton_handle (const locker_key<void*>& key) :
      locker_key<void*> (key)
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

}

#endif
