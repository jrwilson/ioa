#ifndef __blocking_queue_hpp__
#define __blocking_queue_hpp__

#include <boost/thread/condition_variable.hpp>
#include <queue>

template <class T>
class blocking_queue
{
private:
  boost::condition_variable m_condition;
  boost::mutex m_mutex;
  std::queue<T> m_queue;
  
public:
  T pop () {
    boost::unique_lock<boost::mutex> lock (m_mutex);
    while (m_queue.empty ()) {
      m_condition.wait (lock);
    }
    T retval = m_queue.front ();
    m_queue.pop ();
    return retval;
  }
  
  void push (const T& t) {
    boost::unique_lock<boost::mutex> lock (m_mutex);
    m_queue.push (t);
  }
  
};

#endif
