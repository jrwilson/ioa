#ifndef __thread_hpp__
#define __thread_hpp__

#include <pthread.h>
#include <memory>
#include <cassert>

namespace ioa {

  class thread_arg_interface
  {
  public:
    virtual void operator() () = 0;
  };

  template <class I>
  class thread_arg :
    public thread_arg_interface
  {
  private:
    I& m_ref;
    void (I::*m_member_function) ();
    
  public:
    thread_arg (I& ref,
		void (I::*member_function) ()) :
      m_ref (ref),
      m_member_function (member_function)
    { }

    void operator() () {
      (m_ref.*m_member_function) ();
    }
  };

  void* thread_func (void* a);
    
  class thread
  {
  private:
    pthread_t m_thread;
    pthread_attr_t m_attr;
    std::auto_ptr<thread_arg_interface> m_thread_arg;
    
    // No copying.
    thread (const thread&) { }

  public:
    template <class I>
    thread (I& ref,
	    void (I::*func) ()) :
      m_thread_arg (new thread_arg<I> (ref, func))
    {
      int r;
      
      r = pthread_attr_init (&m_attr);
      assert (r == 0);
      r = pthread_create (&m_thread, &m_attr, thread_func, m_thread_arg.get ());
      assert (r == 0);
    }

    ~thread ();
    void join ();
  };
  
}

#endif
