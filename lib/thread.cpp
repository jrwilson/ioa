#include <ioa/thread.hpp>

namespace ioa {

  thread_arg::thread_arg (void (*func) ()) :
    m_func (func)
  { }
    
  void thread_arg::operator() () {
    (*m_func) ();
  }
  
  void* thread_func (void* a) {
    assert (a != 0);
    thread_arg* arg = static_cast<thread_arg*> (a);
    (*arg) ();
    pthread_exit (0);
  }

  thread::thread (void (*func) ()) :
    m_thread_arg (new thread_arg (func))
  {
    int r = pthread_create (&m_thread, 0, thread_func, m_thread_arg.get ());
    assert (r == 0);
  }
  
  void thread::join () {
    int r = pthread_join (m_thread, 0);
    assert (r == 0);
  }

}
