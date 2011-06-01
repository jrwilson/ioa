#ifndef __blocking_queue_hpp__
#define __blocking_queue_hpp__

#include "condition_variable.hpp"
#include "mutex.hpp"
#include "lock.hpp"
#include <list>

template <class T>
class blocking_list
{
private:
  condition_variable m_condition;
  mutex m_mutex;

public:
  std::list<T> list;

  T pop () {
    lock lock (m_mutex);
    while (list.empty ()) {
      m_condition.wait (lock);
    }
    T retval = list.front ();
    list.pop_front ();
    return retval;
  }
  
  void push (const T& t) {
    {
      lock lock (m_mutex);
      list.push_back (t);
    }
    // Only one thread should be calling pop.
    m_condition.notify_one ();
  }

  mutex& get_mutex () {
    return m_mutex;
  }

};

#endif
