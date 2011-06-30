#ifndef __const_shared_ptr_hpp__
#define __const_shared_ptr_hpp__

#include <cstddef>
#include <cassert>
#include <ioa/mutex.hpp>

namespace ioa {
  
  template <typename T>
  class const_shared_ptr
  {
  private:
    mutex* m_mutex;
    const T* m_ptr;
    size_t* m_ref_count;

    void incref () {
      m_mutex->lock ();
      ++(*m_ref_count);
      m_mutex->unlock ();
    }

    void decref () {
      m_mutex->lock ();
      --(*m_ref_count);
      if (*m_ref_count == 0) {
	m_mutex->unlock ();
	delete m_mutex;
	delete m_ptr;
	delete m_ref_count;
      }
      else {
	m_mutex->unlock ();
      }
    }

  public:
    explicit const_shared_ptr (T* ptr = 0) :
      m_mutex (new mutex),
      m_ptr (ptr),
      m_ref_count (new size_t)
    {
      *m_ref_count = 1;
    }
    
    const_shared_ptr (const const_shared_ptr& s) :
      m_mutex (s.m_mutex),
      m_ptr (s.m_ptr),
      m_ref_count (s.m_ref_count)
    {
      incref ();
    }
    
    template <class Y>
    const_shared_ptr (const const_shared_ptr<Y>& s) :
      m_mutex (s.get_mutex ()),
      m_ptr (s.get ()),
      m_ref_count (s.get_ref_count ())
    {
      incref ();
    }

    ~const_shared_ptr () {
      decref ();
    }
    
    const_shared_ptr& operator= (const const_shared_ptr& s) {
      if (this != &s) {
	decref ();
	m_mutex = s.m_mutex;
	m_ptr = s.m_ptr;
	m_ref_count = s.m_ref_count;
	incref ();
      }
      return *this;
    }

    const T* get () const {
      return m_ptr;
    }

    mutex* get_mutex () const {
      return m_mutex;
    }

    size_t* get_ref_count () const {
      return m_ref_count;
    }

    const T* operator-> () const {
      return m_ptr;
    }

    const T& operator* () const {
      return *m_ptr;
    }

    void reset (T* ptr = 0) {
      decref ();
      m_mutex = new mutex;
      m_ptr = ptr;
      m_ref_count = new size_t;
      *m_ref_count = 1;
    }
  };
  
}

#endif
