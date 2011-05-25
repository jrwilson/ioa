#ifndef __blocking_queue_hpp__
#define __blocking_queue_hpp__

#include <boost/thread/condition_variable.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <list>

template <class T>
class blocking_list
{
private:
  boost::condition_variable_any m_condition;

public:
  std::list<T> list;
  boost::shared_mutex mutex;

  T pop () {
    boost::unique_lock<boost::shared_mutex> lock (mutex);
    while (list.empty ()) {
      m_condition.wait (lock);
    }
    T retval = list.front ();
    list.pop_front ();
    return retval;
  }
  
  void push (const T& t) {
    {
      boost::unique_lock<boost::shared_mutex> lock (mutex);
      list.push_back (t);
    }
    // Only one thread should be calling pop.
    m_condition.notify_one ();
  }

};

#endif
