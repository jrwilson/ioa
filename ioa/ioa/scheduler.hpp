#ifndef __scheduler_hpp__
#define __scheduler_hpp__

#include <boost/thread.hpp>
#include <queue>
#include "system.hpp"

namespace ioa {

  template <class T>
  class blocking_queue
  {
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

    template <class C, class I, class M>
    void create (const C* ptr,
		 I* instance,
		 M C::*member_ptr) {
      m_scheduler.create (ptr, instance, member_ptr);
    }

    template <class C, class OI, class OM, class II, class IM, class M>
    void bind (const C* ptr,
	       const automaton_handle<OI>& output_automaton,
	       OM OI::*output_member_ptr,
	       const automaton_handle<II>& input_automaton,
	       IM II::*input_member_ptr,
	       M C::*member_ptr) {
      m_scheduler.bind (ptr, output_automaton, output_member_ptr, input_automaton, input_member_ptr, member_ptr);
    }

    template <class I, class M>
    void schedule (const I* ptr,
		   M I::*member_ptr) {
      m_scheduler.schedule (ptr, member_ptr);
    }

    template <class T>
    void run (T* instance) {
      m_scheduler.run (instance);
    }
  };

  class runnable;

  class internal_scheduler_interface :
    public scheduler_interface
  {
  public:
    virtual void schedule (runnable*) = 0;
  };

  class runnable
  {
  protected:
    internal_scheduler_interface& m_scheduler;
  public:
    runnable (internal_scheduler_interface& scheduler) :
      m_scheduler (scheduler)
    { }
    virtual ~runnable () { }
    virtual void operator() (system&) = 0;
  };

  template <class T>
  class executable :
    public runnable
  {
  private:
    T m_t;

  public:
    executable (internal_scheduler_interface& scheduler,
		const T& t) :
      runnable (scheduler),
      m_t (t)
    { }

    void operator () (system& system) {
      // TODO:  Do something with the result.
      system.execute (m_t, m_scheduler);
    }
  };

  template <class T>
  executable<T>* make_executable (internal_scheduler_interface& scheduler,
				  const T& t)
  {
    return new executable<T> (scheduler, t);
  }

  template <class C, class I, class M>
  class create :
    public runnable
  {
  private:
    automaton_handle<C> m_creator;
    I* m_instance;
    M C::*m_member_ptr;
  public:
    create (internal_scheduler_interface& scheduler,
	    const automaton_handle<C>& creator,
	    I* instance,
	    M C::*member_ptr) :
      runnable (scheduler),
      m_creator (creator),
      m_instance (instance),
      m_member_ptr (member_ptr)
    { }

    void operator () (system& system) {
      system::create_result<I> r = system.create (m_creator, m_instance);
      switch (r.type) {
      case system::CREATE_CREATOR_DNE:
	// Do nothing.
	break;
      case system::CREATE_SUCCESS:
	m_scheduler.schedule (make_executable (m_scheduler, make_action (r.automaton, &I::init)));
	// Fall through.
      case system::CREATE_EXISTS:
	// Tell the creator.
	m_scheduler.schedule (make_executable (m_scheduler, make_action (m_creator, m_member_ptr, r)));
	break;
      }
    }
  };

  template <class C, class I, class M>
  create<C, I, M>* make_create (internal_scheduler_interface& scheduler,
				const automaton_handle<C>& creator,
				I* instance,
				M C::*member_ptr)
  {
    return new create<C, I, M> (scheduler, creator, instance, member_ptr);
  }

  template <class C, class OM, class IM, class M>
  class bind :
    public runnable
  {
  private:
    automaton_handle<C> m_binder;
    action<OM> m_output_action;
    action<IM> m_input_action;
    M C::*m_member_ptr;
  public:
    bind (internal_scheduler_interface& scheduler,
	  const automaton_handle<C>& binder,
	  const action<OM>& output_action,
	  const action<IM>& input_action,
	  M C::*member_ptr) :
      runnable (scheduler),
      m_binder (binder),
      m_output_action (output_action),
      m_input_action (input_action),
      m_member_ptr (member_ptr)
    { }

    void operator() (system& system) {
      system::bind_result r = system.bind (m_output_action, m_input_action, m_binder);
      switch (r.type) {
      case system::BIND_BINDER_AUTOMATON_DNE:
	// Do nothing.
	break;
      case system::BIND_SUCCESS:
	// TODO:  Tell the output and input that they are composed.
	// Fall through.
      case system::BIND_OUTPUT_AUTOMATON_DNE:
      case system::BIND_INPUT_AUTOMATON_DNE:
      case system::BIND_OUTPUT_PARAMETER_DNE:
      case system::BIND_INPUT_PARAMETER_DNE:
      case system::BIND_EXISTS:
      case system::BIND_OUTPUT_ACTION_UNAVAILABLE:
      case system::BIND_INPUT_ACTION_UNAVAILABLE:
	m_scheduler.schedule (make_executable (m_scheduler, make_action (m_binder, m_member_ptr, r)));
	break;
      }
    }
  };

  template <class C, class OM, class IM, class M>
  bind<C, OM, IM, M>* make_bind (internal_scheduler_interface& scheduler,
				 const automaton_handle<C>& binder,
				 const action<OM>& output_action, 
				 const action<IM>& input_action,
				 M C::*member_ptr)
  {
    return new bind<C, OM, IM, M> (scheduler, binder, output_action, input_action, member_ptr);
  }

  class simple_scheduler :
    public internal_scheduler_interface
  {
  private:
    system m_system;
    blocking_queue<runnable*> m_runq;
    generic_automaton_handle m_current_handle;

    void set_current_handle (const generic_automaton_handle& handle) {
      m_current_handle = handle;
    }

    void clear_current_handle () {
      m_current_handle = generic_automaton_handle ();
    }

    void schedule (runnable* r) {
      m_runq.push (r);
    }

    template <class I>
    automaton_handle<I> get_current_handle (const I* ptr) const {
      // An automaton is executing an action.
      // Someone forgot to set the current handle.
      BOOST_ASSERT (m_current_handle.serial () != 0);
      // We require the user to pass a pointer to an instance which should always be "this."
      // We then check if the pointer in the handle matches the supplied pointer.
      BOOST_ASSERT (m_current_handle.value ()->get_instance () == ptr);
      // Unless the user has casted "this" to a different type, we can convert the generic handle to a typed handle.
      locker_key<automaton<I>*> key (m_current_handle.serial (), static_cast<automaton<I>*> (m_current_handle.value ()));
      automaton_handle<I> handle (key);
      return handle;
    }

  public:
    simple_scheduler ()
    { }

    template <class C, class I, class M>
    void create (const C* ptr,
		 I* instance,
		 M C::*member_ptr) {
      m_runq.push (make_create (*this, get_current_handle (ptr), instance, member_ptr));
    }
    
    void schedule_declare () {
      BOOST_ASSERT (false);
    }

    template <class C, class OI, class OM, class II, class IM, class M>
    void bind (const C* ptr,
	       const automaton_handle<OI>& output_automaton,
	       OM OI::*output_member_ptr,
	       const automaton_handle<II>& input_automaton,
	       IM II::*input_member_ptr,
	       M C::*member_ptr) {
      m_runq.push (make_bind (*this, get_current_handle (ptr),
			      make_action (output_automaton, output_member_ptr),
			      make_action (input_automaton, input_member_ptr),
			      member_ptr));
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

  private:
    template <class M>
    void schedule (const action<M>& ac,
		   schedulable_category /* */) {
      m_runq.push (make_executable (*this, ac));
    }

  public:
    template <class I, class M>
    void schedule (const I* ptr,
		   M I::*member_ptr) {
      action<M> ac = make_action (get_current_handle (ptr), member_ptr);
      schedule (ac, typename action<M>::action_category ());
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
      }
    }

  };

  simple_scheduler ss;
  scheduler_wrapper<simple_scheduler> scheduler (ss);

}

#endif
