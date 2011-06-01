#ifndef __thread_hpp__
#define __thread_hpp__

class thread_arg_interface
{
public:
  virtual ~thread_arg_interface () { }
  virtual void operator() () = 0;
};

template <class I>
class thread_arg :
  public thread_arg_interface
{
private:
  void (I::*m_member_function_ptr) ();
  I& m_i;

public:
  thread_arg (void (I::*member_function_ptr) (),
	      I& i) :
    m_member_function_ptr (member_function_ptr),
    m_i (i)
  { }

  void operator() () {
    (m_i.*m_member_function_ptr) ();
  }
};

void* thread_func (void* a) {
  assert (a != 0);
  thread_arg_interface* arg = static_cast<thread_arg_interface*> (a);
  (*arg) ();
  pthread_exit (0);
}

class thread
{
private:
  pthread_t m_thread;
  std::auto_ptr<thread_arg_interface> m_thread_arg;

public:
  template <class I>
  thread (void (I::*member_function_ptr) (),
	  I& i) :
    m_thread_arg (new thread_arg<I> (member_function_ptr, i))
  {
    int r = pthread_create (&m_thread, 0, thread_func, m_thread_arg.get ());
    assert (r == 0);
  }

  void join () {
    int r = pthread_join (m_thread, 0);
    assert (r == 0);
  }
};

#endif
