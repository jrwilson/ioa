#ifndef __runnable_hpp__
#define __runnable_hpp__

#include <boost/thread/shared_mutex.hpp>

#include "action.hpp"

namespace ioa {

  class runnable_interface
  {
  private:
    // We keep track of the number of runnables in existence.
    // When the count reaches 0, we can stop.
    static boost::shared_mutex m_mutex;
    static size_t m_count;

  public:
    runnable_interface ()
    {
      boost::unique_lock<boost::shared_mutex> lock (m_mutex);
      ++m_count;
    }
    virtual ~runnable_interface () {
      boost::unique_lock<boost::shared_mutex> lock (m_mutex);
      --m_count;
    }
    static size_t count () {
      boost::shared_lock<boost::shared_mutex> lock (m_mutex);
      return m_count;
    }

    virtual void operator() () = 0;
  };

  template <class T>
  class runnable :
    public runnable_interface
  {
  private:
    T m_t;
    
  public:
    runnable (const T& t) :
      m_t (t)
    { }

    void operator() () {
      m_t ();
    }
  };

  template <class T>
  runnable<T>* make_runnable (const T& t) {
    return new runnable<T> (t);
  }

  class action_runnable_interface :
    public runnable_interface
  {
  public:
    virtual ~action_runnable_interface () { }

    virtual const action_interface& get_action () const = 0;

    bool operator== (const action_runnable_interface& x) const {
      return get_action () == x.get_action ();
    }
  };

  template <class T, class I, class M>
  class action_runnable :
    public action_runnable_interface
  {
  private:
    T m_t;
    action<I, M> m_action;
    
  public:
    action_runnable (const T& t,
		     const action<I, M>& action) :
      m_t (t),
      m_action (action)
    { }
    
    void operator() () {
      m_t ();
    }

    const action_interface& get_action () const {
      return m_action;
    }
  };

  template <class T, class I, class M>
  action_runnable<T, I, M>* make_action_runnable (const T& t,
						  const action<I, M>& ac) {
    return new action_runnable<T, I, M> (t, ac);
  }

}

#endif
