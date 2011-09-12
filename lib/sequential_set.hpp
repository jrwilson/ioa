#ifndef __sequential_set_hpp__
#define __sequential_set_hpp__

#include <set>

/*
  The Aliasing Problem

  Users manipulate resources in a system by using an ID of some kind.
  Assume a user has acquired the ID of a resource.
  That resource is then destroyed and the ID returned to the pool of available IDs.
  A new resource is allocated using the ID.
  The user then asks for an operating using the ID which now points to a different resource.

  A good, but not fool-proof method, of avoiding this problem is to make sure that IDs are not reused too quickly.

  The sequential_set class ensures that IDs are not reused too quickly by allocating IDs in ascending order and never allocating an ID that is already allocated.
 */

namespace ioa {
  
  template <class T>
  class sequential_set
  {
  private:
    T m_counter;
    std::set<T> m_used;
  public:
    sequential_set () :
      m_counter (0)
    { }
    
    T take () {
      do {
	++m_counter;
	if (m_counter < 0) {
	  m_counter = 0;
	}
      } while (m_used.count (m_counter) != 0);
      m_used.insert (m_counter);
      return m_counter;
    }

    void take (const T& t) {
      m_used.insert (t);
    }
    
    void replace (const T& t) {
      m_used.erase (t);
    }
    
    bool contains (const T& t) const {
      return m_used.count (t) != 0;
    }
    
    void clear () {
      m_used.clear ();
    }
    
    bool empty () const {
      return m_used.empty ();
    }
  };

}

#endif
