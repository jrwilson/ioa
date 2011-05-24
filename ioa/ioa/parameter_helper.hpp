#ifndef __parameter_helper_hpp__
#define __parameter_helper_hpp__

#include "observer.hpp"

namespace ioa {

  template <class T, class P>
  class parameter_helper :
    public observable
  {
  private:
    typedef enum {
      DECLARE_SENT,
      DECLARE_RECV1,
      DECLARE_RECV2,
      RESCIND_SENT,
    } state_type;
    state_type m_state;
    const T* m_t;
    P* m_parameter;
    parameter_handle<P> m_handle;
    std::set<observer*> m_observers;

  public:

    parameter_helper (const T* t,
		      P* parameter) :
      m_state (DECLARE_SENT),
      m_t (t),
      m_parameter (parameter)
    {
      scheduler.declare (m_t, m_parameter, *this);
    }

    ~parameter_helper () {
      // Notify the observers.
      BOOST_FOREACH (observer* o, m_observers) {
	o->stop_observing ();
      }
    }

    void rescind () {
      switch (m_state) {
      case DECLARE_SENT:
	m_state = DECLARE_RECV2;
	break;
      case DECLARE_RECV1:
	scheduler.rescind (m_t, m_handle, *this);
	// Reset the handle.
	m_handle = parameter_handle<P> ();
	m_state = RESCIND_SENT;
	break;
      case DECLARE_RECV2:
	BOOST_ASSERT (false);
	break;
      case RESCIND_SENT:
	BOOST_ASSERT (false);
	break;
      }
    }
  
    void parameter_declared (const parameter_handle<P>& parameter) {
      switch (m_state) {
      case DECLARE_SENT:
	m_handle = parameter;
	m_state = DECLARE_RECV1;
	// Notify the observers.
	BOOST_FOREACH (observer* o, m_observers) {
	  o->observe ();
	}
	break;
      case DECLARE_RECV1:
	BOOST_ASSERT (false);
	break;
      case DECLARE_RECV2:
	scheduler.rescind (m_t, parameter, *this);
	m_state = RESCIND_SENT;
	break;
      case RESCIND_SENT:
	BOOST_ASSERT (false);
	break;
      }
    }
  
    void parameter_rescinded () {
      delete this;
    }

    void parameter_exists () {
      BOOST_ASSERT (false);
    }

    void parameter_dne () {
      BOOST_ASSERT (false);
    }

    parameter_handle<P> get_handle () const {
      return m_handle;
    }

    void add_observer (observer* o) {
      BOOST_ASSERT (o != 0);
      m_observers.insert (o);

      if (m_state == DECLARE_RECV1) {
	o->observe ();
      }
    }

    void remove_observer (observer* o) {
      m_observers.erase (o);
    }

  };

}

#endif
