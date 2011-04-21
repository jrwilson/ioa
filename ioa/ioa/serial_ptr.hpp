#ifndef __serial_ptr_hpp__
#define __serial_ptr_hpp__

#include <boost/thread/mutex.hpp>

namespace ioa {

  template<class T>
  class counter
  {
  private:
    boost::mutex m_mutex;
    T m_count;

  public:
    counter (const T& start = 0)
      :
      m_count (start)
    { }

    T
    increment ()
    {
      boost::lock_guard<boost::mutex> lock (m_mutex);
      return m_count++;
    }
  };

  typedef unsigned long long counter_type;

  static counter<counter_type> serial_ptr_counter;

  template <class T>
  class serial_ptr
  {
  private:
    T* m_ptr;
    counter_type m_serial_number;

  public:
    serial_ptr () :
      m_ptr (0),
      m_serial_number (0)
    { }

    explicit serial_ptr (T* ptr) :
      m_ptr (ptr),
      m_serial_number (serial_ptr_counter.increment ())
    {
      if (m_serial_number == 0) {
	m_serial_number = serial_ptr_counter.increment ();
      }
    }

    serial_ptr (const serial_ptr& ts) :
      m_ptr (ts.m_ptr),
      m_serial_number (ts.m_serial_number)
    { }

    template <class U>
    serial_ptr (const serial_ptr<U>& ts) :
      m_ptr (ts.get ()),
      m_serial_number (ts.get_serial_number ())
    { }

    serial_ptr& operator= (const serial_ptr& ts) {
      if (this != &ts) {
	m_ptr = ts.m_ptr;
	m_serial_number = ts.m_serial_number;
      }
      return *this;
    }

    T* get_ptr () const {
      return m_ptr;
    }

    T* operator-> () const {
      return m_ptr;
    }

    // Should we have a T& operator* () ??

    counter_type get_serial_number () const {
      return m_serial_number;
    }

    bool valid () const {
      return m_serial_number != 0;
    }
  };

}

#endif
