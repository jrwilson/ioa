#ifndef __runnable_hpp__
#define __runnable_hpp__

#include "action.hpp"

#include "shared_mutex.hpp"
#include "unique_lock.hpp"
#include "shared_lock.hpp"

namespace ioa {

  class runnable_interface
  {
  private:
    // We keep track of the number of runnables in existence.
    // When the count reaches 0, we can stop.
    static shared_mutex m_mutex;
    static size_t m_count;

  public:
    runnable_interface ()
    {
      unique_lock lock (m_mutex);
      ++m_count;
    }
    virtual ~runnable_interface () {
      unique_lock lock (m_mutex);
      --m_count;
    }
    static size_t count () {
      shared_lock lock (m_mutex);
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

}

#endif