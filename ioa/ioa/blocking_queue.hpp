#ifndef __blocking_queue_hpp__
#define __blocking_queue_hpp__

#include <boost/thread/condition_variable.hpp>
#include <list>

template <class T>
class blocking_queue
{
private:
  boost::condition_variable m_condition;
  boost::mutex m_mutex;
  std::list<T> m_queue;
  
public:
  typedef typename std::list<T>::iterator iterator;
  typedef typename std::list<T>::size_type size_type;

  T pop () {
    boost::unique_lock<boost::mutex> lock (m_mutex);
    while (m_queue.empty ()) {
      m_condition.wait (lock);
    }
    T retval = m_queue.front ();
    m_queue.pop_front ();
    return retval;
  }
  
  void push (const T& t) {
    boost::unique_lock<boost::mutex> lock (m_mutex);
    m_queue.push_back (t);
  }

  void clear () {
    m_queue.clear ();
  }

  size_type size () const {
    return m_queue.size ();
  }

  iterator begin () {
    return m_queue.begin ();
  }
  
  iterator end () {
    return m_queue.end ();
  }

};

#endif
