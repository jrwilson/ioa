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
    void stop_observing (observable* o);

  public:
    virtual ~observer ();
    virtual void observe () = 0;
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
