#ifndef __locker_hpp__
#define __locker_hpp__

#include <map>

namespace ioa {

  typedef size_t serial_type;

  template <class T>
  class locker_key
  {
  private:
    serial_type m_serial;
    T m_value;

  public:
    locker_key ()
      :
      m_serial (0)
    { }

    locker_key (T const value)
    :
      m_serial (0),
      m_value (value)
    { }

    locker_key (serial_type const serial_number,
		T const value)
      :
      m_serial (serial_number),
      m_value (value)
    { }

    locker_key& operator= (const locker_key& key) {
      this->m_serial = key.m_serial;
      this->m_value = key.m_value;
      return *this;
    }

    template<class U>
    operator locker_key<U> () const {
      return locker_key<U> (this->m_serial, this->m_value);
    }

    const serial_type& serial () const {
      return m_serial;
    }
    
    const T& value () const {
      return m_value;
    }
  };

  template <class U, class V>
  bool operator== (const locker_key<U>& u,
		   const locker_key<V>& v) {
    return u.serial () == v.serial () && u.value () == v.value ();
  }

  template <class U, class V>
  bool operator< (const locker_key<U>& u,
		  const locker_key<V>& v) {
    if (u.serial () != v.serial ()) {
      return u.serial () < v.serial ();
    }
    return u.value () < v.value ();
  }

  template <class T>
  class locker
  {
  private:
    typedef locker_key<T> key_type;
    typedef std::map<serial_type, T> sv_type;
    typedef std::map<T, serial_type> vs_type;

    serial_type m_serial_number;
    sv_type m_serial_value;
    vs_type m_value_serial;

  public:
    typedef size_t size_type;
    typedef typename vs_type::iterator iterator;
    typedef typename vs_type::const_iterator const_iterator;

    locker ()
      :
      m_serial_number (0)
    { }

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

    iterator begin () {
      return iterator (m_value_serial.begin ());
    }

    const_iterator begin () const {
      return const_iterator (m_value_serial.begin ());
    }

    iterator end () {
      return iterator (m_value_serial.end ());
    }

    const_iterator end () const {
      return const_iterator (m_value_serial.end ());
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
      // We don't use the serial number from the default constructor locker keys constructed
      // outside of a locker are invalid.
      do {
	++m_serial_number;
      } while (m_serial_value.find (m_serial_number) != m_serial_value.end () ||
	       m_serial_number == 0);

      // Insert.
      m_serial_value.insert (std::make_pair (m_serial_number, value));
      m_value_serial.insert (std::make_pair (value, m_serial_number));

      return locker_key<U> (m_serial_number, value);
    }

    void erase (const key_type& key) {
      m_serial_value.erase (key.serial ());
      m_value_serial.erase (key.value ());
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
      typename sv_type::const_iterator pos = m_serial_value.find (key.serial ());
      if (pos != m_serial_value.end ()) {
	return pos->second == key.value ();
      }
      else {
	return false;
      }
    }

    locker_key<T> find (const T& value) const {
      typename vs_type::const_iterator pos = m_value_serial.find (value);
      return locker_key<T> (pos->second, pos->first);
    }

  };

}

#endif
