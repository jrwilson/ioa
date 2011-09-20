#ifndef __thread_key_hpp__
#define __thread_key_hpp__

#include <pthread.h>
#include <cassert>

#include "profile.hpp"

namespace ioa {

  template <typename T>
  class thread_key
  {
  private:
    pthread_key_t m_key;

  public:
    thread_key () {
      BEGIN_SYS_CALL;
      int r = pthread_key_create (&m_key, 0);
      END_SYS_CALL;
      assert (r == 0);
    }

    ~thread_key () {
      BEGIN_SYS_CALL;
      int r = pthread_key_delete (m_key);
      END_SYS_CALL;
      assert (r == 0);
    }

    T get () const {
      // Using BEGIN_SYS_CALL/END_SYS_CALL results in infinite recursion.
      void* ptr = pthread_getspecific (m_key);
      return reinterpret_cast<T> (ptr);
    }

    void set (T t) {
      const void* ptr = reinterpret_cast<const void*> (t);
      BEGIN_SYS_CALL;
      int r = pthread_setspecific (m_key, ptr);
      END_SYS_CALL;
      assert (r == 0);
    }
  };

}

#endif
