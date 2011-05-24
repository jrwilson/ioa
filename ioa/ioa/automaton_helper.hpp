#ifndef __automaton_helper_hpp__
#define __automaton_helper_hpp__

#include "observer.hpp"

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
      CREATE_SENT,
      CREATE_RECV1,
      CREATE_RECV2,
      DESTROY_SENT,
    } state_type;
    state_type m_state;
    const T* m_t;
    G m_generator;
    automaton_handle<instance> m_handle;
    std::set<observer*> m_observers;

  public:

    automaton_helper (const T* t,
		      G generator) :
      m_state (CREATE_SENT),
      m_t (t),
      m_generator (generator)
    {
      scheduler.create (m_t, m_generator, *this);
    }

    ~automaton_helper () {
      // Notify the observers.
      BOOST_FOREACH (observer* o, m_observers) {
	o->stop_observing ();
      }
    }

    void destroy () {
      switch (m_state) {
      case CREATE_SENT:
	m_state = CREATE_RECV2;
	break;
      case CREATE_RECV1:
	scheduler.destroy (m_t, m_handle, *this);
	// Reset the handle.
	m_handle = automaton_handle<instance> ();
	m_state = DESTROY_SENT;
	break;
      case CREATE_RECV2:
	BOOST_ASSERT (false);
	break;
      case DESTROY_SENT:
	BOOST_ASSERT (false);
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
      case CREATE_RECV1:
	BOOST_ASSERT (false);
	break;
      case CREATE_RECV2:
	scheduler.destroy (m_t, automaton, *this);
	m_state = DESTROY_SENT;
	break;
      case DESTROY_SENT:
	BOOST_ASSERT (false);
	break;
      }
    }
  
    void instance_exists (const instance* /* */) {
      BOOST_ASSERT (false);
    }
  
    void automaton_destroyed () {
      delete this;
    }

    void target_automaton_dne () {
      BOOST_ASSERT (false);
    }

    void destroyer_not_creator () {
      BOOST_ASSERT (false);
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

}

#endif
