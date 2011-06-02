#ifndef __thread_key_hpp__
#define __thread_key_hpp__

#include <pthread.h>
#include <cassert>

namespace ioa {

  template <typename T>
  class thread_key
  {
  private:
    pthread_key_t m_key;

  public:
    thread_key () {
      int r = pthread_key_create (&m_key, 0);
      assert (r == 0);
    }

    ~thread_key () {
      int r = pthread_key_delete (m_key);
      assert (r == 0);
    }

    T get () const {
      void* ptr = pthread_getspecific (m_key);
      return reinterpret_cast<T> (ptr);
    }

    void set (T t) {
      const void* ptr = reinterpret_cast<const void*> (t);
      int r = pthread_setspecific (m_key, ptr);
      assert (r == 0);
    }
  };

}

#endif
