#ifndef __blocking_list_hpp__
#define __blocking_list_hpp__

#include <ioa/condition_variable.hpp>
#include <list>

namespace ioa {
  
  template <class T>
  class blocking_list
  {
  private:
    condition_variable m_condition;
    
  public:
    std::list<T> list;
    mutex list_mutex;
    
    T pop () {
      lock lock (list_mutex);
      while (list.empty ()) {
	m_condition.wait (lock);
      }
      T retval = list.front ();
      list.pop_front ();
      return retval;
    }
    
    void push (const T& t) {
      {
	lock lock (list_mutex);
	list.push_back (t);
      }
      // Only one thread should be calling pop.
      m_condition.notify_one ();
    }
    
  };

}

#endif