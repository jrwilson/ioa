#ifndef __shared_ptr_hpp__
#define __shared_ptr_hpp__

#include <cstddef>
#include <cassert>
#include <ioa/mutex.hpp>

namespace ioa {
  
  template <typename T>
  class shared_ptr
  {
  private:
    mutex* m_mutex;
    T* m_ptr;
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
    explicit shared_ptr (T* ptr = 0) :
      m_mutex (new mutex),
      m_ptr (ptr),
      m_ref_count (new size_t)
    {
      *m_ref_count = 1;
    }
    
    shared_ptr (const shared_ptr& s) :
      m_mutex (s.m_mutex),
      m_ptr (s.m_ptr),
      m_ref_count (s.m_ref_count)
    {
      incref ();
    }
    
    ~shared_ptr () {
      decref ();
    }
    
    shared_ptr& operator= (const shared_ptr& s) {
      if (this != &s) {
	decref ();
	m_mutex = s.m_mutex;
	m_ptr = s.m_ptr;
	m_ref_count = s.m_ref_count;
	incref ();
      }
      return *this;
    }

    T* operator-> () const {
      return m_ptr;
    }

    T& operator* () const {
      return *m_ptr;
    }
  };
  
}

#endif
