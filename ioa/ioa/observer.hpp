#ifndef __observer_hpp__
#define __observer_hpp__

#include <set>
#include <boost/foreach.hpp>

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
    void notify_observers () {
      // Notify the observers.
      BOOST_FOREACH (observer* o, m_observers) {
	o->observe ();
      }
    }

  public:

    ~observable () {
      // Notify the observers.
      BOOST_FOREACH (observer* o, m_observers) {
	o->stop_observing ();
      }
    }

    void add_observer (observer* o) {
      BOOST_ASSERT (o != 0);
      m_observers.insert (o);
    }
    
    void remove_observer (observer* o) {
      m_observers.erase (o);
    }

  };

}

#endif
