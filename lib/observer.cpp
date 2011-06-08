#include <ioa/observer.hpp>

#include <cassert>

namespace ioa {

  observer::~observer () {
    // Tell the observables we are no longer interested.
    for (std::set<observable*>::const_iterator pos = m_observables.begin ();
	 pos != m_observables.end ();
	 ++pos) {
      (*pos)->remove_observer (this);
    }
  }

  void observer::stop_observing (observable* o) {
    // Observable is going away.
    m_observables.erase (o);
  }

  void observer::add_observable (observable* o) {
    assert (o != 0);
    if (m_observables.count (o) == 0) {
      m_observables.insert (o);
      o->add_observer (this);
    }
  }

  void observer::remove_observable (observable* o) {
    assert (o != 0);
    if (m_observables.count (o) != 0) {
      m_observables.erase (o);
      o->remove_observer (this);
    }
  }

  void observable::notify_observers () {
    // Notify the observers.
    for (std::set<observer*>::const_iterator pos = m_observers.begin ();
	 pos != m_observers.end ();
	 ++pos) {
      (*pos)->observe ();
    }
  }

  observable::~observable () {
    // Notify the observers.
    for (std::set<observer*>::const_iterator pos = m_observers.begin ();
	 pos != m_observers.end ();
	 ++pos) {
      (*pos)->stop_observing (this);
    }
  }
  
  void observable::add_observer (observer* o) {
    assert (o != 0);
    m_observers.insert (o);
  }
  
  void observable::remove_observer (observer* o) {
    m_observers.erase (o);
  }

}
