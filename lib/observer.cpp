#include <ioa/observer.hpp>

namespace ioa {

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
      (*pos)->stop_observing ();
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
