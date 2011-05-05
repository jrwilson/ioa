#ifndef __scheduler_hpp__
#define __scheduler_hpp__

#include <boost/thread.hpp>
#include <queue>
#include "system.hpp"

namespace ioa {

  template <class T>
  class blocking_queue {
  private:
    boost::condition_variable m_condition;
    boost::mutex m_mutex;
    std::queue<T> m_queue;

  public:
    T pop () {
      boost::unique_lock<boost::mutex> lock (m_mutex);
      while (m_queue.empty ()) {
	m_condition.wait (lock);
      }
      T retval = m_queue.front ();
      m_queue.pop ();
      return retval;
    }

    void push (const T& t) {
      boost::unique_lock<boost::mutex> lock (m_mutex);
      m_queue.push (t);
    }

  };

  template <class S>
  class scheduler_wrapper
  {
  private:
    S& m_scheduler;
  public:
    scheduler_wrapper (S& scheduler) :
      m_scheduler (scheduler)
    { }

    template <class I, class M>
    void schedule (I* ptr,
		   M I::*member_ptr) {
      m_scheduler.schedule (ptr, member_ptr);
    }

    template <class T>
    void run (T* instance) {
      m_scheduler.run (instance);
    }
  };

  class internal_scheduler_interface
  {
  public:
    virtual ~internal_scheduler_interface () { }
    virtual void set_current_handle (const generic_automaton_handle& handle) = 0;
    virtual void clear_current_handle () = 0;
  };

  class runnable {
  public:
    virtual ~runnable () { }
    virtual void operator() (system&) = 0;
  };

  template <class T>
  class executable :
    public runnable {
  private:
    internal_scheduler_interface& m_scheduler;
    T m_t;

  public:
    executable (internal_scheduler_interface& scheduler,
		const T& t) :
      m_scheduler (scheduler),
      m_t (t)
    { }

    void operator () (system& system) {
      m_scheduler.set_current_handle (m_t.get_automaton_handle ());
      system.execute (m_t);
      m_scheduler.clear_current_handle ();
    }
  };

  template <class T>
  executable<T>* make_executable (internal_scheduler_interface& scheduler, const T& t)
  {
    return new executable<T> (scheduler, t);
  }

  class simple_scheduler :
    public internal_scheduler_interface
  {
  private:
    system m_system;
    blocking_queue<runnable*> m_runq;
    bool m_flag;
    generic_automaton_handle m_current_handle;

    generic_automaton_handle get_current_handle () const {
      BOOST_ASSERT (m_current_handle.serial () != 0);
      return m_current_handle;
    }

    void set_current_handle (const generic_automaton_handle& handle) {
      m_current_handle = handle;
    }

    void clear_current_handle () {
      m_current_handle = generic_automaton_handle ();
    }

  public:
    simple_scheduler () :
      m_flag (false)
    { }

    void schedule_create () {
      BOOST_ASSERT (false);
    }
    
    void schedule_declare () {
      BOOST_ASSERT (false);
    }

    void schedule_bind () {
      BOOST_ASSERT (false);
    }
    
    void schedule_unbind () {
      BOOST_ASSERT (false);
    }

    void schedule_rescind () {
      BOOST_ASSERT (false);
    }
    
    void schedule_destroy () {
      BOOST_ASSERT (false);
    }

    template <class I, class M>
    void schedule (I* ptr,
		   M I::*member_ptr) {
      // This deserves some explaining.
      // An automaton is executing an action so we get the handle for that automaton.
      generic_automaton_handle generic_handle = get_current_handle ();
      // We require the user to pass a pointer to an instance which should always be "this."
      // We then check if the pointer in the handle matches the supplied pointer.
      BOOST_ASSERT (generic_handle.value ()->get_instance () == ptr);
      // Unless the user has casted "this" to a different type, we can convert the generic handle to a typed handle.
      locker_key<automaton<I>*> key (generic_handle.serial (), static_cast<automaton<I>*> (generic_handle.value ()));
      automaton_handle<I> handle (key);
      
      m_runq.push (make_executable (*this, make_action (handle, member_ptr)));
    }

    template <class T>
    void run (T* instance) {
      system::create_result<T> r = m_system.create (instance);
      BOOST_ASSERT (r.type == system::CREATE_SUCCESS);

      m_runq.push (make_executable (*this, make_action (r.automaton, &T::init)));

      for (;;) {
	runnable* r = m_runq.pop ();
	(*r) (m_system);
	delete r;
	if (m_flag) {
	  break;
	}
      }
    }

    void exit (void) {
      m_flag = true;
    }

  };

  simple_scheduler ss;
  scheduler_wrapper<simple_scheduler> scheduler (ss);

}

#endif
