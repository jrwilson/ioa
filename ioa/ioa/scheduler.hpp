#ifndef __scheduler_hpp__
#define __scheduler_hpp__

#include <boost/thread.hpp>
#include <queue>
#include "system.hpp"
#include "runnable.hpp"

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

  template <class C, class I>
  class create :
    public runnable
  {
  private:
    automaton_handle<C> m_creator;
    I* m_instance;
  public:
    create (internal_scheduler_interface& scheduler,
	    const automaton_handle<C>& creator,
	    I* instance) :
      runnable (scheduler),
      m_creator (creator),
      m_instance (instance)
    { }

    void operator () (system& system) {
      system::create_result r = system.create (m_creator, m_instance);
      switch (r.type) {
      case system::CREATE_CREATOR_DNE:
	// Do nothing.
	break;
      case system::CREATE_SUCCESS:
	m_scheduler.schedule (make_executable (m_scheduler, make_action (cast_automaton<I> (r.automaton), &I::init)));
	// Fall through.
      case system::CREATE_EXISTS:
	// Tell the creator.
	m_scheduler.schedule (make_executable (m_scheduler, make_action (m_creator, &C::created, r)));
	break;
      }
    }
  };

  template <class C, class I>
  create<C, I>* make_create (internal_scheduler_interface& scheduler,
			     const automaton_handle<C>& creator,
			     I* instance)
  {
    return new create<C, I> (scheduler, creator, instance);
  }

  template <class C, class P>
  class declare :
    public runnable
  {
  private:
    automaton_handle<C> m_automaton;
    P* m_parameter;
  public:
    declare (internal_scheduler_interface& scheduler,
	     const automaton_handle<C>& automaton,
	     P* parameter) :
      runnable (scheduler),
      m_automaton (automaton),
      m_parameter (parameter)
    { }

    void operator () (system& system) {
      system::declare_result r = system.declare (m_automaton, m_parameter);
      switch (r.type) {
      case system::DECLARE_AUTOMATON_DNE:
      	// Do nothing.
      	break;
      case system::DECLARE_SUCCESS:
      case system::DECLARE_EXISTS:
  	m_scheduler.schedule (make_executable (m_scheduler, make_action (m_automaton, &C::declared, r)));
      	break;
      }
    }
  };

  template <class C, class P>
  declare<C, P>* make_declare (internal_scheduler_interface& scheduler,
			       const automaton_handle<C>& automaton,
			       P* parameter) {
    return new declare<C, P> (scheduler, automaton, parameter);
  }

  template <class C, class OM, class IM>
  class bind :
    public runnable
  {
  private:
    automaton_handle<C> m_binder;
    action<OM> m_output_action;
    action<IM> m_input_action;
  public:
    bind (internal_scheduler_interface& scheduler,
	  const automaton_handle<C>& binder,
	  const action<OM>& output_action,
	  const action<IM>& input_action) :
      runnable (scheduler),
      m_binder (binder),
      m_output_action (output_action),
      m_input_action (input_action)
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
	m_scheduler.schedule (make_executable (m_scheduler, make_action (m_binder, &C::bound, r)));
	break;
      }
    }
  };

  template <class C, class OM, class IM>
  bind<C, OM, IM>* make_bind (internal_scheduler_interface& scheduler,
			      const automaton_handle<C>& binder,
			      const action<OM>& output_action, 
			      const action<IM>& input_action)
  {
    return new bind<C, OM, IM> (scheduler, binder, output_action, input_action);
  }

  template <class C, class I, class M>
  class destroy :
    public runnable
  {
  private:
    automaton_handle<C> m_destroyer;
    automaton_handle<I> m_automaton;
    M C::*m_member_ptr;
  public:
    destroy (internal_scheduler_interface& scheduler,
	     const automaton_handle<C>& destroyer,
	     const automaton_handle<I>& automaton,
	     M C::*member_ptr) :
      runnable (scheduler),
      m_destroyer (destroyer),
      m_automaton (automaton),
      m_member_ptr (member_ptr)
    { }

    void operator () (system& system) {
      BOOST_ASSERT (false);
      // system::destroy_result r = system.destroy (m_destroyer, m_automaton);
      // switch (r.type) {
      // case system::DESTROY_DESTROYER_DNE:
      // 	// Do nothing.
      // 	break;
      // case system::DESTROY_DESTROYER_NOT_CREATOR:
      // case system::DESTROY_SUCCESS:
      // 	// m_scheduler.schedule (make_executable (m_scheduler, make_action (r.automaton, &I::init)));
      // 	// Fall through.
      // case system::DESTROY_EXISTS:
      // 	// Tell the creator.
      // 	// m_scheduler.schedule (make_executable (m_scheduler, make_action (m_creator, m_member_ptr, r)));
      // 	break;
      // }
    }
  };

  template <class C, class I, class M>
  destroy<C, I, M>* make_destroy (internal_scheduler_interface& scheduler,
				  const automaton_handle<C>& creator,
				  const automaton_handle<I>& automaton,
				  M C::*member_ptr)
  {
    return new destroy<C, I, M> (scheduler, creator, automaton, member_ptr);
  }

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

  template <class S>
  class scheduler_wrapper
  {
  private:
    S& m_scheduler;
  public:
    scheduler_wrapper (S& scheduler) :
      m_scheduler (scheduler)
    { }

    template <class C, class I>
    void create (const C* ptr,
		 I* instance) {
      m_scheduler.schedule (make_create (m_scheduler, m_scheduler.get_current_handle (ptr), instance));
    }

    template <class C, class P>
    void declare (const C* ptr,
		  P* parameter) {
      m_scheduler.schedule (make_declare (m_scheduler, m_scheduler.get_current_handle (ptr), parameter));
    }

    template <class C, class OI, class OM, class II, class IM>
    void bind (const C* ptr,
	       const automaton_handle<OI>& output_automaton,
	       OM OI::*output_member_ptr,
	       const automaton_handle<II>& input_automaton,
	       IM II::*input_member_ptr) {
      m_scheduler.schedule (make_bind (m_scheduler, m_scheduler.get_current_handle (ptr),
      				       make_action (output_automaton, output_member_ptr),
      				       make_action (input_automaton, input_member_ptr)));
    }

    template <class C, class I, class M>
    void destroy (const C* ptr,
		  const automaton_handle<I>& automaton,
		  M C::*member_ptr) {
      m_scheduler.destroy (ptr, automaton, member_ptr);
    }

  private:
    template <class M>
    void schedule (const action<M>& ac,
  		   output_category /* */) {
      m_scheduler.schedule (make_executable (m_scheduler, ac));
    }
    
    template <class M>
    void schedule (const action<M>& ac,
  		   internal_category /* */) {
      m_scheduler.schedule (make_executable (m_scheduler, ac));
    }

  public:
    template <class I, class M>
    void schedule (const I* ptr,
		   M I::*member_ptr) {
      action<M> ac = make_action (m_scheduler.get_current_handle (ptr), member_ptr);
      schedule (ac, typename action<M>::action_category ());
    }

    template <class T>
    void run (T* instance) {
      m_scheduler.run (instance);
    }

    void clear (void) {
      m_scheduler.clear ();
    }
  };

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

    static bool keep_going () {
      return runnable::count () != 0;
    }

  public:
    simple_scheduler ()
    { }

    template <class I>
    automaton_handle<I> get_current_handle (const I* ptr) const {
      // An automaton is executing an action.
      // Someone forgot to set the current handle.
      BOOST_ASSERT (m_current_handle.serial () != 0);
      // We require the user to pass a pointer to an instance which should always be "this."
      // We then check if the pointer in the handle matches the supplied pointer.
      BOOST_ASSERT (m_current_handle.value () == ptr);
      // Unless the user has casted "this" to a different type, we can convert the generic handle to a typed handle.
      locker_key<I*> key (m_current_handle.serial (), static_cast<I*> (m_current_handle.value ()));
      automaton_handle<I> handle (key);
      return handle;
    }

    void schedule (runnable* r) {
      m_runq.push (r);
    }

    template <class T>
    void run (T* instance) {
      system::create_result r = m_system.create (instance);
      BOOST_ASSERT (r.type == system::CREATE_SUCCESS);

      m_runq.push (make_executable (*this, make_action (cast_automaton<T> (r.automaton), &T::init)));

      for (;;) {
	runnable* r = m_runq.pop ();
	(*r) (m_system);
	delete r;
	if (!keep_going ()) {
	  break;
	}
      }
    }

    void clear (void) {
      m_system.clear ();
    }
  };

  simple_scheduler ss;
  scheduler_wrapper<simple_scheduler> scheduler (ss);

}

#endif
