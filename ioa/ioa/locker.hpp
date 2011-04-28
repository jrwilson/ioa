#ifndef __locker_hpp__
#define __locker_hpp__

#include <map>

namespace ioa {

  typedef size_t serial_type;

  template <class T>
  struct locker_key :
    public std::pair<serial_type const, T const>
  {
    locker_key (serial_type const serial_number,
		T const value)
      :
      std::pair<serial_type const, T const> (serial_number, value) { }
    
    template<class U>
    operator locker_key<U> () const {
      return locker_key<U> (this->first, this->second);
    }

    // bool operator== (const locker_key& k) const {
    //   return this->first == k.first && this->second == k.second;
    // }
  };

  template <class U, class V>
  bool operator== (const locker_key<U>& u,
		   const locker_key<V>& v) {
    return u.first == v.first && u.second == v.second;
  }

  template <class T>
  class locker
  {
  public:
    typedef size_t size_type;

  private:
    typedef locker_key<T> key_type;
    typedef std::map<serial_type, T> sv_type;
    typedef std::map<T, serial_type> vs_type;

    serial_type m_serial_number;
    sv_type m_serial_value;
    vs_type m_value_serial;
    
  public:
    // Capacity.
    bool empty () const {
      return m_serial_value.empty ();
    }

    size_type size () const {
      return m_serial_value.size ();
    }
  
    size_type max_size () const {
      return m_serial_value.max_size ();
    }

    // Modifiers.
    template <class U>
    locker_key<U> insert (const U& value) {
      // Do we already have the value?
      typename vs_type::iterator pos = m_value_serial.find (value);
      if (pos != m_value_serial.end ()) {

	m_serial_value.erase (pos->second);
	m_value_serial.erase (pos);
      }

      // Generate a new key.
      // We don't use 0 as a serial number so keys can have a default constructor.
      while (m_serial_value.find (m_serial_number) != m_serial_value.end () ||
	     m_serial_number == 0) {
	++m_serial_number;
      }

      // Insert.
      m_serial_value.insert (std::make_pair (m_serial_number, value));
      m_value_serial.insert (std::make_pair (value, m_serial_number));

      return locker_key<U> (m_serial_number, value);
    }

    void erase (const key_type& key) {
      m_serial_value.erase (key.first);
      m_value_serial.erase (key.second);
    }
    
    void clear () {
      m_serial_value.clear ();
      m_value_serial.clear ();
    }

    // Query.
    bool contains (const T& value) const {
      return m_value_serial.find (value) != m_value_serial.end ();
    }

    bool contains (const key_type& key) const {
      typename sv_type::const_iterator pos = m_serial_value.find (key.first);
      if (pos != m_serial_value.end ()) {
	return pos->second == key.second;
      }
      else {
	return false;
      }
    }

  };

}

#endif
