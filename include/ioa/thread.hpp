#ifndef __thread_hpp__
#define __thread_hpp__

#include <pthread.h>
#include <memory>

namespace ioa {

  class thread_arg
  {
  private:
    void (*m_func) ();
    
  public:
    thread_arg (void (*func) ());
    void operator() ();
  };
    
  class thread
  {
  private:
    pthread_t m_thread;
    pthread_attr_t m_attr;
    std::auto_ptr<thread_arg> m_thread_arg;
    
  public:
    thread (void (*func) ());
    ~thread ();
    void join ();
  };
  
}

#endif
