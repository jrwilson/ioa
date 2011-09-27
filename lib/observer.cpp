/*
   Copyright 2011 Justin R. Wilson

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

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

  void observer::stop_observing_dispatcher (observable* o) {
    assert (o != 0);
    // Observable is going away.
    if (m_observables.count (o) != 0) {
      m_observables.erase (o);
      stop_observing (o);
    }
  }

  void observer::stop_observing (observable*) { }

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
    m_notify = true;
    // Notify the observers.
    for (std::set<observer*>::const_iterator pos = m_observers.begin ();
	 pos != m_observers.end ();
	 ++pos) {
      (*pos)->observe (this);
    }
    m_notify = false;

    m_observers.insert (m_add.begin (), m_add.end ());
    for (std::set<observer*>::const_iterator pos = m_remove.begin ();
	 pos != m_remove.end ();
	 ++pos) {
      m_observers.erase (*pos);
    }
  }

  observable::observable () :
    m_notify (false)
  { }

  observable::~observable () {
    // Notify the observers.
    for (std::set<observer*>::const_iterator pos = m_observers.begin ();
	 pos != m_observers.end ();
	 ++pos) {
      (*pos)->stop_observing_dispatcher (this);
    }
  }
  
  void observable::add_observer (observer* o) {
    assert (o != 0);
    if (m_notify) {
      m_add.insert (o);
      m_remove.erase (o);
    }
    else {
      m_observers.insert (o);
    }
  }
  
  void observable::remove_observer (observer* o) {
    assert (o != 0);
    if (m_notify) {
      m_remove.insert (o);
      m_add.erase (o);
    }
    else {
      m_observers.erase (o);      
    }
  }

}
