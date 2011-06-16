#ifndef __blocking_list_hpp__
#define __blocking_list_hpp__

#include "mutex.hpp"
#include "condition_variable.hpp"
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
    
    size_t push (const T& t) {
      size_t retval;
      {
	lock lock (list_mutex);
	list.push_back (t);
	retval = list.size ();
      }
      // Only one thread should be calling pop.
      m_condition.notify_one ();
      return retval;
    }
    
  };

}

#endif
