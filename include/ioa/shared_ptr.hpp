#ifndef __shared_ptr_hpp__
#define __shared_ptr_hpp__

#include <cstddef>
#include <cassert>

namespace ioa {
  
  template <typename T>
  class shared_ptr
  {
  private:
    T* m_ptr;
    size_t* m_ref_count;

    void incref () {
      ++(*m_ref_count);
    }

    void decref () {
      --(*m_ref_count);
      if (*m_ref_count == 0) {
	delete m_ptr;
	delete m_ref_count;
      }
    }

  public:
    explicit shared_ptr (T* ptr = 0) :
      m_ptr (ptr),
      m_ref_count (new size_t)
    {
      *m_ref_count = 1;
    }
    
    shared_ptr (const shared_ptr& s) :
      m_ptr (s.m_ptr),
      m_ref_count (s.m_ref_count)
    {
      incref ();
    }
    
    template <class Y>
    shared_ptr (shared_ptr<Y>& s) :
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
	m_ptr = s.m_ptr;
	m_ref_count = s.m_ref_count;
	incref ();
      }
      return *this;
    }

    T* operator-> () {
      return m_ptr;
    }

    T& operator* () {
      return *m_ptr;
    }
  };
  
}

#endif
