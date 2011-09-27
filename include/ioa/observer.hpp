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

#ifndef __observer_hpp__
#define __observer_hpp__

#include <set>

// An approach to the Observer Pattern.

namespace ioa {

  class observable;

  class observer
  {
  private:
    std::set<observable*> m_observables;

    friend class observable;
    void stop_observing_dispatcher (observable* o);

  public:
    virtual ~observer ();
    virtual void observe (observable* o) = 0;
    virtual void stop_observing (observable* o);
    void add_observable (observable* o);
    void remove_observable (observable* o);
  };

  class observable
  {
  private:
    std::set<observer*> m_observers;
    std::set<observer*> m_add;
    std::set<observer*> m_remove;
    bool m_notify;

    friend class observer;
    void add_observer (observer* o);
    void remove_observer (observer* o);

  protected:
    void notify_observers ();

  public:
    observable ();
    virtual ~observable ();
  };

}

#endif
