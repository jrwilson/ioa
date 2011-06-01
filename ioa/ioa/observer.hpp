#ifndef __observer_hpp__
#define __observer_hpp__

#include <set>

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
      for (std::set<observer*>::const_iterator pos = m_observers.begin ();
	   pos != m_observers.end ();
	   ++pos) {
	(*pos)->observe ();
      }
    }

  public:

    ~observable () {
      // Notify the observers.
      for (std::set<observer*>::const_iterator pos = m_observers.begin ();
	   pos != m_observers.end ();
	   ++pos) {
	(*pos)->stop_observing ();
      }
    }

    void add_observer (observer* o) {
      assert (o != 0);
      m_observers.insert (o);
    }
    
    void remove_observer (observer* o) {
      m_observers.erase (o);
    }

  };

}

#endif
