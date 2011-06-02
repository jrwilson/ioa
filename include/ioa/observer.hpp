#ifndef __observer_hpp__
#define __observer_hpp__

#include <set>

// An approach to the Observer Pattern.

namespace ioa {

  class observer
  {
  public:
    virtual ~observer () { }
    virtual void observe () = 0;
    virtual void stop_observing () = 0;
  };

  class observable
  {
  private:
    std::set<observer*> m_observers;

  protected:
    void notify_observers ();

  public:
    ~observable ();
    void add_observer (observer* o);
    void remove_observer (observer* o);
  };

}

#endif
