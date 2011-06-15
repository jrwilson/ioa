#include <ioa/thread.hpp>

#include <cassert>

namespace ioa {

  void* thread_func (void* a) {
    assert (a != 0);
    thread_arg_interface* arg = static_cast<thread_arg_interface*> (a);
    (*arg) ();
    pthread_exit (0);
  }

  thread::~thread () {
    int r = pthread_attr_destroy (&m_attr);
    assert (r == 0);
  }
  
  void thread::join () {
    int r = pthread_join (m_thread, 0);
    assert (r == 0);
  }

}
