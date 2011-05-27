#ifndef __automaton_helper_hpp__
#define __automaton_helper_hpp__

#include "observer.hpp"
#include "automaton_handle.hpp"
#include <set>
#include <boost/foreach.hpp>
#include "scheduler.hpp"

namespace ioa {

  template <class T, class G>
  class automaton_helper :
    public observable
  {
  public:
    typedef G generator;
    typedef typename G::result_type instance;

  private:
    typedef enum {
      START,
      CREATE_SENT,
      CREATE_RECV1,
      CREATE_RECV2,
      DESTROY_SENT,
      STOP,
      ERROR
    } state_type;
    state_type m_state;
    const T* m_t;
    G m_generator;
    automaton_handle<instance> m_handle;
    std::set<observer*> m_observers;

  public:

    automaton_helper (const T* t,
		      G generator) :
      m_state (START),
      m_t (t),
      m_generator (generator)
    { }

    ~automaton_helper () {
      // Notify the observers.
      BOOST_FOREACH (observer* o, m_observers) {
	o->stop_observing ();
      }
    }

    void create () {
      switch (m_state) {
      case START:
	scheduler.create (m_t, m_generator, *this);
	m_state = CREATE_SENT;
	break;
      default:
	break;
      }
    }

    void destroy () {
      switch (m_state) {
      case START:
	m_state = STOP;
	break;
      case CREATE_SENT:
	m_state = CREATE_RECV2;
	break;
      case CREATE_RECV1:
	scheduler.destroy (m_t, m_handle, *this);
	// Reset the handle.
	m_handle = automaton_handle<instance> ();
	m_state = DESTROY_SENT;
	break;
      default:
	break;
      }
    }
  
    void automaton_created (const automaton_handle<instance>& automaton) {
      switch (m_state) {
      case CREATE_SENT:
	m_handle = automaton;
	m_state = CREATE_RECV1;
	// Notify the observers.
	BOOST_FOREACH (observer* o, m_observers) {
	  o->observe ();
	}
	break;
      case CREATE_RECV2:
	scheduler.destroy (m_t, automaton, *this);
	m_state = DESTROY_SENT;
	break;
      default:
	break;
      }
    }
  
    void instance_exists (const instance* /* */) {
      m_state = ERROR;
    }
  
    void automaton_destroyed () {
      m_handle = automaton_handle<instance> ();
      m_state = STOP;
    }

    void target_automaton_dne () {
      m_state = ERROR;
    }

    void destroyer_not_creator () {
      m_state = ERROR;
    }

    automaton_handle<instance> get_handle () const {
      return m_handle;
    }

    void add_observer (observer* o) {
      BOOST_ASSERT (o != 0);
      m_observers.insert (o);

      if (m_state == CREATE_RECV1) {
	o->observe ();
      }
    }

    void remove_observer (observer* o) {
      m_observers.erase (o);
    }

  };

  template <class T>
  class self_helper :
    public observable
  {
  private:
    const T* m_t;
    automaton_handle<T> m_handle;
    std::set<observer*> m_observers;

  public:
    typedef T instance;

    self_helper (const T* t) :
      m_t (t)
    { }

    void create () {
      m_handle = scheduler.get_current_aid (m_t);
      // Notify the observers.
      BOOST_FOREACH (observer* o, m_observers) {
	o->observe ();
      }
    }

    automaton_handle<instance> get_handle () const {
      return m_handle;
    }

    void add_observer (observer* o) {
      BOOST_ASSERT (o != 0);
      m_observers.insert (o);

      if (m_handle.aid () != -1) {
	o->observe ();
      }
    }

    void remove_observer (observer* o) {
      m_observers.erase (o);
    }

  };

}

#endif
