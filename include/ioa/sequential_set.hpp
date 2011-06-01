#ifndef __sequential_set_hpp__
#define __sequential_set_hpp__

#include <set>

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

#endif
