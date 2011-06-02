#ifndef __automaton_helper_hpp__
#define __automaton_helper_hpp__

#include "observer.hpp"
#include "automaton_handle.hpp"
#include "scheduler.hpp"

namespace ioa {

  template <class T>
  class self_helper :
    public observable
  {
  private:
    automaton_handle<T> m_handle;

  public:
    typedef T instance;

    self_helper (const T* t)
    {
      m_handle = scheduler::get_current_aid (t);
    }

  private:
    ~self_helper () { }

  public:
    void destroy () {
      delete this;
    }

    automaton_handle<instance> get_handle () const {
      return m_handle;
    }

  };

  template <class T, class I>
  class automaton_helper :
    public observable
  {
  public:
    typedef I instance;

  private:
    typedef enum {
      CREATE_SENT,
      CREATE_RECV1,
      CREATE_RECV2,
      DESTROY_SENT,
    } state_type;
    state_type m_state;
    const T* m_t;
    std::auto_ptr<generator_interface<I> > m_generator;
    automaton_handle<I> m_handle;

  public:
    automaton_helper (const T* t,
		      std::auto_ptr<generator_interface<I> > generator) :
      m_t (t),
      m_generator (generator)
    {
      scheduler::create (m_t, m_generator, *this);
      m_state = CREATE_SENT;
    }

  private:
    ~automaton_helper () { }

  public:
    void destroy () {
      switch (m_state) {
      case CREATE_SENT:
	m_state = CREATE_RECV2;
	break;
      case CREATE_RECV1:
	scheduler::destroy (m_t, m_handle, *this);
	// Reset the handle.
	m_handle = automaton_handle<I> ();
	m_state = DESTROY_SENT;
	break;
      default:
	break;
      }
    }
  
    void automaton_created (const automaton_handle<I>& automaton) {
      switch (m_state) {
      case CREATE_SENT:
	m_handle = automaton;
	m_state = CREATE_RECV1;
	// Notify the observers.
	notify_observers ();
	break;
      case CREATE_RECV2:
	scheduler::destroy (m_t, automaton, *this);
	m_state = DESTROY_SENT;
	break;
      default:
	break;
      }
    }
  
    void instance_exists (const I* /* */) {
      delete this;
    }
  
    void automaton_destroyed () {
      m_handle = automaton_handle<I> ();
      delete this;
    }

    void target_automaton_dne () {
      delete this;
    }

    void destroyer_not_creator () {
      delete this;
    }

    automaton_handle<I> get_handle () const {
      return m_handle;
    }

  };

}

#endif
