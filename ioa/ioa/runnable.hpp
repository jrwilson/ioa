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
  private:
    const aid_t m_aid;
    const void* m_ptr;

  public:
    action_runnable_interface (const aid_t aid,
			       const void* ptr) :
      m_aid (aid),
      m_ptr (ptr)
    { }

    bool operator== (const action_runnable_interface& x) const {
      return m_aid == x.m_aid && m_ptr == x.m_ptr;
    }
  };

  template <class T>
  class action_runnable :
    public action_runnable_interface
  {
  private:
    T m_t;
    
  public:
    action_runnable (const T& t,
		     const aid_t aid,
		     const void* ptr) :
      action_runnable_interface (aid, ptr),
      m_t (t)
    { }
    
    void operator() () {
      m_t ();
    }
  };

  template <class T, class I, class M>
  action_runnable<T>* make_action_runnable (const T& t,
					    const action<I, M>& ac) {
    I* instance = 0;
    return new action_runnable<T> (t, ac.automaton.aid (), &((*instance).*(ac.member_ptr)));
  }

}

#endif
