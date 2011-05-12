#ifndef __simple_scheduler_hpp__
#define __simple_scheduler_hpp__

namespace ioa {

  // TODO:  DUPLICATES!!!
  // TODO:  EVENTS!!!
  // TODO:  Send event to destroyed automaton.

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

  template <class T>
  class runnable :
    public runnable_interface
  {
  private:
    T m_t;
    
  public:
    runnable (const T& t) :
      m_t (t)
    { }

    void operator() () {
      m_t ();
    }
  };

  template <class T>
  runnable<T>* make_runnable (const T& t) {
    return new runnable<T> (t);
  }

  template <class T, class EFL>
  class system_event :
    public runnable_interface
  {
  private:
    system& m_system;
    generic_automaton_handle m_automaton;
    runnable<T> m_t;
    internal_scheduler_interface& m_scheduler;
    EFL& m_efl;
    
  public:
    system_event (system& system,
  		  const generic_automaton_handle& automaton,
		  const T& t,
		  internal_scheduler_interface& scheduler,
		  EFL& efl) :
      m_system (system),
      m_automaton (automaton),
      m_t (t),
      m_scheduler (scheduler),
      m_efl (efl)
    { }
    
    void operator () () {
      m_system.execute (m_automaton, m_t, m_scheduler, m_efl);
    }
  };

  template <class T, class EFL>
  system_event<T, EFL>* make_system_event (system& system,
					   const generic_automaton_handle& automaton,
					   const T& t,
					   internal_scheduler_interface& scheduler,
					   EFL& efl) {
    return new system_event<T, EFL> (system, automaton, t, scheduler, efl);
  }

  class simple_scheduler :
    public internal_scheduler_interface
  {
  private:
    system m_system;
    blocking_queue<runnable_interface*> m_runq;
    generic_automaton_handle m_current_handle;

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

    void set_current_handle (const generic_automaton_handle& handle) {
      m_current_handle = handle;
    }

    void clear_current_handle () {
      m_current_handle = generic_automaton_handle ();
    }

    static bool keep_going () {
      return runnable_interface::count () != 0;
    }

    void schedule (runnable_interface* r) {
      m_runq.push (r);
    }

    struct create_proxy
    {
      simple_scheduler& m_scheduler;

      template <class I>
      void instance_exists (const I* instance) {
	// Create without a creator failed.  Something is wrong.
	BOOST_ASSERT (false);
      }

      template <class I>
      void automaton_created (const automaton_handle<I>& automaton) {
	// Initialize the new created root automaton.
	m_scheduler.schedule (make_system_event (m_scheduler.m_system,
						 automaton,
						 boost::bind (&I::init,
							      automaton.value ()),
						 m_scheduler,
						 m_scheduler.m_execute_proxy));
      }

      template <class P, class I>
      void automaton_dne (const automaton_handle<P>& automaton,
			  I* instance) {
	// An automaton attempting to create another automaton was destroyed.
	// Do nothing.
	// NB: The system called delete on instance.
      }

      template <class P, class I>
      void instance_exists (const automaton_handle<P>& automaton,
			    I* instance) {
	void (P::*ptr) (const I*) = &P::instance_exists;
	m_scheduler.schedule (make_system_event (m_scheduler.m_system,
						 automaton,
						 boost::bind (ptr,
							      automaton.value (),
							      instance),
						 m_scheduler,
						 m_scheduler.m_execute_proxy));
      }

      template <class P, class I>
      void automaton_created (const automaton_handle<P>& automaton,
			      const automaton_handle<I>& child) {
	// Tell the parent about its child.
	void (P::*ptr1) (const automaton_handle<I>&) = &P::automaton_created;
	m_scheduler.schedule (make_system_event (m_scheduler.m_system,
						 automaton,
						 boost::bind (ptr1,
							      automaton.value (),
							      child),
						 m_scheduler,
						 m_scheduler.m_execute_proxy));
	// Initialize the child.
	m_scheduler.schedule (make_system_event (m_scheduler.m_system,
						 child,
						 boost::bind (&I::init,
							      child.value ()),
						 m_scheduler,
						 m_scheduler.m_execute_proxy));
      }

      create_proxy (simple_scheduler& scheduler) :
	m_scheduler (scheduler)
      { }
    };
    create_proxy m_create_proxy;

    struct declare_proxy
    {
      simple_scheduler& m_scheduler;

      template <class I, class P>
      void automaton_dne (const automaton_handle<I>&,
			  P*) {
	// An automaton attempted to declare a parameter was destroyed.
	// Do nothing.
      }

      template <class I, class P>
      void parameter_exists (const automaton_handle<I>& automaton,
			     const parameter_handle<P>& parameter) {
	void (I::*ptr) (const parameter_handle<P>&) = &I::parameter_exists;
	m_scheduler.schedule (make_system_event (m_scheduler.m_system,
						 automaton,
						 boost::bind (ptr,
							      automaton.value (),
							      parameter),
						 m_scheduler,
						 m_scheduler.m_execute_proxy));
      }

      template <class I, class P>
      void parameter_declared (const automaton_handle<I>& automaton,
			       const parameter_handle<P>& parameter) {
	void (I::*ptr) (const parameter_handle<P>&) = &I::parameter_declared;
	m_scheduler.schedule (make_system_event (m_scheduler.m_system,
						 automaton,
						 boost::bind (ptr,
							      automaton.value (),
							      parameter),
						 m_scheduler,
						 m_scheduler.m_execute_proxy));
      }
      
      declare_proxy (simple_scheduler& scheduler) :
	m_scheduler (scheduler)
      { }
    };
    declare_proxy m_declare_proxy;

    // TODO:  Pass more information back to caller on failure.
    struct bind_proxy
    {
      simple_scheduler& m_scheduler;

      template <class OI, class OM, class II, class IM, class I>
      void automaton_dne (const action<OI, OM>& output_action,
			  const action<II, IM>& input_action,
			  const automaton_handle<I>& binder) {
	// An automaton attempting to bind was destroyed.
	// Do nothing.
      }

      template <class OI, class OM, class II, class IM, class I>
      void output_automaton_dne (const action<OI, OM>& output_action,
				 const action<II, IM>& input_action,
				 const automaton_handle<I>& binder) {
	m_scheduler.schedule (make_system_event (m_scheduler.m_system,
						 binder,
						 boost::bind (&I::bind_output_automaton_dne,
							      binder.value ()),
						 m_scheduler,
						 m_scheduler.m_execute_proxy));
      }

      template <class OI, class OM, class II, class IM, class I>
      void input_automaton_dne (const action<OI, OM>& output_action,
				const action<II, IM>& input_action,
				const automaton_handle<I>& binder) {
	m_scheduler.schedule (make_system_event (m_scheduler.m_system,
						 binder,
						 boost::bind (&I::bind_input_automaton_dne,
							      binder.value ()),
						 m_scheduler,
						 m_scheduler.m_execute_proxy));
      }

      template <class OI, class OM, class II, class IM, class I>
      void output_parameter_dne (const action<OI, OM>& output_action,
				 const action<II, IM>& input_action,
				 const automaton_handle<I>& binder) {
	m_scheduler.schedule (make_system_event (m_scheduler.m_system,
						 binder,
						 boost::bind (&I::bind_output_parameter_dne,
							      binder.value ()),
						 m_scheduler,
						 m_scheduler.m_execute_proxy));
      }

      template <class OI, class OM, class II, class IM, class I>
      void input_parameter_dne (const action<OI, OM>& output_action,
				const action<II, IM>& input_action,
				const automaton_handle<I>& binder) {
	m_scheduler.schedule (make_system_event (m_scheduler.m_system,
						 binder,
						 boost::bind (&I::bind_input_parameter_dne,
							      binder.value ()),
						 m_scheduler,
						 m_scheduler.m_execute_proxy));
      }

      template <class OI, class OM, class II, class IM, class I>
      void binding_exists (const action<OI, OM>& output_action,
			   const action<II, IM>& input_action,
			   const automaton_handle<I>& binder) {
	m_scheduler.schedule (make_system_event (m_scheduler.m_system,
						 binder,
						 boost::bind (&I::binding_exists,
							      binder.value ()),
						 m_scheduler,
						 m_scheduler.m_execute_proxy));
      }

      template <class OI, class OM, class II, class IM, class I>
      void input_action_unavailable (const action<OI, OM>& output_action,
				     const action<II, IM>& input_action,
				     const automaton_handle<I>& binder) {
	m_scheduler.schedule (make_system_event (m_scheduler.m_system,
						 binder,
						 boost::bind (&I::input_action_unavailable,
							      binder.value ()),
						 m_scheduler,
						 m_scheduler.m_execute_proxy));
      }

      template <class OI, class OM, class II, class IM, class I>
      void output_action_unavailable (const action<OI, OM>& output_action,
				      const action<II, IM>& input_action,
				      const automaton_handle<I>& binder) {
	m_scheduler.schedule (make_system_event (m_scheduler.m_system,
						 binder,
						 boost::bind (&I::output_action_unavailable,
							      binder.value ()),
						 m_scheduler,
						 m_scheduler.m_execute_proxy));
      }

      template <class OI, class OM, class II, class IM, class I>
      void bound (const action<OI, OM>& output_action,
		  const action<II, IM>& input_action,
		  const automaton_handle<I>& binder) {
	m_scheduler.schedule (make_system_event (m_scheduler.m_system,
						 binder,
						 boost::bind (&I::bound,
							      binder.value ()),
						 m_scheduler,
						 m_scheduler.m_execute_proxy));
	void (system::*ptr1) (const output_action_interface&,
			      scheduler_interface&,
			      execute_proxy&) = &system::bound;
	m_scheduler.schedule (make_runnable (boost::bind (ptr1,
							  boost::ref (m_scheduler.m_system),
							  output_action,
							  boost::ref (m_scheduler),
							  boost::ref (m_scheduler.m_execute_proxy))));
	void (system::*ptr2) (const input_action_interface&,
			      scheduler_interface&,
			      execute_proxy&) = &system::bound;
	m_scheduler.schedule (make_runnable (boost::bind (ptr2,
							  boost::ref (m_scheduler.m_system),
							  input_action,
							  boost::ref (m_scheduler),
							  boost::ref (m_scheduler.m_execute_proxy))));
      }

      bind_proxy (simple_scheduler& scheduler) :
	m_scheduler (scheduler)
      { }
    };
    bind_proxy m_bind_proxy;
    
    struct unbind_success_proxy
    {
      simple_scheduler& m_scheduler;
      
      template <class OI, class OM, class II, class IM, class I>
      void unbound (const action<OI, OM>& output_action,
		    const action<II, IM>& input_action,
		    const automaton_handle<I>& binder) {
	m_scheduler.schedule (make_system_event (m_scheduler.m_system,
						 binder,
						 boost::bind (&I::unbound,
							      binder.value ()),
						 m_scheduler,
						 m_scheduler.m_execute_proxy));
	void (system::*ptr1) (const output_action_interface&,
			      scheduler_interface&,
			      execute_proxy&) = &system::unbound;
	m_scheduler.schedule (make_runnable (boost::bind (ptr1,
							  boost::ref (m_scheduler.m_system),
							  output_action,
							  boost::ref (m_scheduler),
							  boost::ref (m_scheduler.m_execute_proxy))));
	void (system::*ptr2) (const input_action_interface&,
			      scheduler_interface&,
			      execute_proxy&) = &system::unbound;
	m_scheduler.schedule (make_runnable (boost::bind (ptr2,
							  boost::ref (m_scheduler.m_system),
							  input_action,
							  boost::ref (m_scheduler),
							  boost::ref (m_scheduler.m_execute_proxy))));
      }

      unbind_success_proxy (simple_scheduler& scheduler) :
	m_scheduler (scheduler)
      { }
    };
    unbind_success_proxy m_unbind_success_proxy;

    struct unbind_failure_proxy
    {
      simple_scheduler& m_scheduler;

      template <class OI, class OM, class II, class IM, class I>
      void automaton_dne (const action<OI, OM>& output_action,
			  const action<II, IM>& input_action,
			  const automaton_handle<I>& binder) {
	// An automaton attempted to unbind was destroyed.
	// Do nothing.
      }

      template <class OI, class OM, class II, class IM, class I>
      void output_automaton_dne (const action<OI, OM>& output_action,
				 const action<II, IM>& input_action,
				 const automaton_handle<I>& binder) {
	m_scheduler.schedule (make_system_event (m_scheduler.m_system,
						 binder,
						 boost::bind (&I::unbind_output_automaton_dne,
							      binder.value ()),
						 m_scheduler,
						 m_scheduler.m_execute_proxy));
      }

      template <class OI, class OM, class II, class IM, class I>
      void input_automaton_dne (const action<OI, OM>& output_action,
				const action<II, IM>& input_action,
				const automaton_handle<I>& binder) {
	m_scheduler.schedule (make_system_event (m_scheduler.m_system,
						 binder,
						 boost::bind (&I::unbind_input_automaton_dne,
							      binder.value ()),
						 m_scheduler,
						 m_scheduler.m_execute_proxy));
      }

      template <class OI, class OM, class II, class IM, class I>
      void output_parameter_dne (const action<OI, OM>& output_action,
				 const action<II, IM>& input_action,
				 const automaton_handle<I>& binder) {
	m_scheduler.schedule (make_system_event (m_scheduler.m_system,
						 binder,
						 boost::bind (&I::unbind_output_parameter_dne,
							      binder.value ()),
						 m_scheduler,
						 m_scheduler.m_execute_proxy));
      }

      template <class OI, class OM, class II, class IM, class I>
      void input_parameter_dne (const action<OI, OM>& output_action,
				const action<II, IM>& input_action,
				const automaton_handle<I>& binder) {
	m_scheduler.schedule (make_system_event (m_scheduler.m_system,
						 binder,
						 boost::bind (&I::unbind_input_parameter_dne,
							      binder.value ()),
						 m_scheduler,
						 m_scheduler.m_execute_proxy));
      }

      template <class OI, class OM, class II, class IM, class I>
      void binding_dne (const action<OI, OM>& output_action,
			const action<II, IM>& input_action,
			const automaton_handle<I>& binder) {
      	m_scheduler.schedule (make_system_event (m_scheduler.m_system,
      						 binder,
      						 boost::bind (&I::binding_dne,
      							      binder.value ()),
      						 m_scheduler,
      						 m_scheduler.m_execute_proxy));
      }
      
      unbind_failure_proxy (simple_scheduler& scheduler) :
	m_scheduler (scheduler)
      { }
    };
    unbind_failure_proxy m_unbind_failure_proxy;

    struct rescind_success_proxy
    {
      simple_scheduler& m_scheduler;

      template <class I, class P>
      void parameter_rescinded (const automaton_handle<I>& automaton,
				const parameter_handle<P>& parameter) {
	void (I::*ptr) (const parameter_handle<P>&) = &I::parameter_rescinded;
	m_scheduler.schedule (make_system_event (m_scheduler.m_system,
						 automaton,
						 boost::bind (ptr,
							      automaton.value (),
							      parameter),
						 m_scheduler,
						 m_scheduler.m_execute_proxy));
      }
      
      rescind_success_proxy (simple_scheduler& scheduler) :
	m_scheduler (scheduler)
      { }
    };
    rescind_success_proxy m_rescind_success_proxy;

    struct rescind_failure_proxy
    {
      simple_scheduler& m_scheduler;

      template <class I, class P>
      void automaton_dne (const automaton_handle<I>&,
			  const parameter_handle<P>&) {
	// An automaton attempting to rescind a parameter was destroyed.
	// Do nothing.
      }

      template <class I, class P>
      void parameter_dne (const automaton_handle<I>& automaton,
			  const parameter_handle<P>& parameter) {
	void (I::*ptr) (const parameter_handle<P>&) = &I::parameter_dne;
	m_scheduler.schedule (make_system_event (m_scheduler.m_system,
						 automaton,
						 boost::bind (ptr,
							      automaton.value (),
							      parameter),
						 m_scheduler,
						 m_scheduler.m_execute_proxy));
      }

      rescind_failure_proxy (simple_scheduler& scheduler) :
	m_scheduler (scheduler)
      { }
    };
    rescind_failure_proxy m_rescind_failure_proxy;

    struct destroy_success_proxy
    {
      simple_scheduler& m_scheduler;

      template <class I>
      void automaton_destroyed (const automaton_handle<I>& automaton) {
	// Root automaton destroyed.
      }

      template <class I, class T>
      void automaton_destroyed (const automaton_handle<I>& automaton,
				const automaton_handle<T>& child) {
	void (I::*ptr) (const automaton_handle<T>&) = &I::automaton_destroyed;
	m_scheduler.schedule (make_system_event (m_scheduler.m_system,
						 automaton,
						 boost::bind (ptr,
							      automaton.value (),
							      child),
						 m_scheduler,
						 m_scheduler.m_execute_proxy));
      }

      destroy_success_proxy (simple_scheduler& scheduler) :
	m_scheduler (scheduler)
      { }
    };
    destroy_success_proxy m_destroy_success_proxy;

    struct destroy_failure_proxy
    {
      simple_scheduler& m_scheduler;

      template <class P, class I>
      void automaton_dne (const automaton_handle<P>&,
			  const automaton_handle<I>&) {
	// An automaton attempting to destroy another automaton was itself destroyed.
	// Do nothing.
      }

      template <class P, class I>
      void target_automaton_dne (const automaton_handle<P>& automaton,
				 const automaton_handle<I>& target) {
	void (P::*ptr) (const automaton_handle<I>&) = &P::target_automaton_dne;
	m_scheduler.schedule (make_system_event (m_scheduler.m_system,
						 automaton,
						 boost::bind (ptr,
							      automaton.value (),
							      target),
						 m_scheduler,
						 m_scheduler.m_execute_proxy));
      }

      template <class P, class I>
      void destroyer_not_creator (const automaton_handle<P>& automaton,
				  const automaton_handle<I>& target) {
	void (P::*ptr) (const automaton_handle<I>&) = &P::destroyer_not_creator;
	m_scheduler.schedule (make_system_event (m_scheduler.m_system,
						 automaton,
						 boost::bind (ptr,
							      automaton.value (),
							      target),
						 m_scheduler,
						 m_scheduler.m_execute_proxy));
      }

      destroy_failure_proxy (simple_scheduler& scheduler) :
	m_scheduler (scheduler)
      { }
    };
    destroy_failure_proxy m_destroy_failure_proxy;

    struct execute_proxy
    {
      void automaton_dne () {
	// An automaton wishing to execute an action was destroyed.
	// Do nothing.
      }

      void parameter_dne () {
	// Attempting to execute an action with a non-existent parameter.
	// One of two things happened.
	// First, the user scheduled an action with a non-existent parameter.
	// If we so desire, we can catch this at schedule time.
	// Second, the parameter was rescinded before the action was executed.
	// This is acceptable behavior.
	// Do nothing.
      }
    };
    execute_proxy m_execute_proxy;

  public:
    simple_scheduler () :
      m_create_proxy (*this),
      m_declare_proxy (*this),
      m_bind_proxy (*this),
      m_unbind_success_proxy (*this),
      m_unbind_failure_proxy (*this),
      m_rescind_success_proxy (*this),
      m_rescind_failure_proxy (*this),
      m_destroy_success_proxy (*this),
      m_destroy_failure_proxy (*this)
    { }

    template <class C, class I>
    void create (const C* ptr,
		 I* instance) {
      void (system::*create_ptr) (const automaton_handle<C>&,
      				  I*,
      				  create_proxy&,
				  destroy_success_proxy&) = &system::create;
      schedule (make_runnable (boost::bind (create_ptr,
					    boost::ref (m_system),
					    get_current_handle (ptr),
					    instance,
					    boost::ref (m_create_proxy),
					    boost::ref (m_destroy_success_proxy))));
    }
    
    template <class C, class P>
    void declare (const C* ptr,
		  P* parameter) {
      void (system::*declare_ptr) (const automaton_handle<C>&,
				   P*,
				   declare_proxy&,
				   rescind_success_proxy&) = &system::declare;
      schedule (make_runnable (boost::bind (declare_ptr,
					    boost::ref (m_system),
					    get_current_handle (ptr),
					    parameter,
					    boost::ref (m_declare_proxy),
					    boost::ref (m_rescind_success_proxy))));
    }

    template <class C, class OI, class OM, class II, class IM>
    void bind (const C* ptr,
	       const automaton_handle<OI>& output_automaton,
	       OM OI::*output_member_ptr,
	       const automaton_handle<II>& input_automaton,
	       IM II::*input_member_ptr) {
      void (system::*bind_ptr) (const action<OI, OM>&,
      				const action<II, IM>&,
      				const automaton_handle<C>&,
      				bind_proxy&,
				unbind_success_proxy&) = &system::bind;
      schedule (make_runnable (boost::bind (bind_ptr,
					    boost::ref (m_system),
					    make_action (output_automaton, output_member_ptr),
					    make_action (input_automaton, input_member_ptr),
					    get_current_handle (ptr),
					    boost::ref (m_bind_proxy),
					    boost::ref (m_unbind_success_proxy))));
    }

    template <class OI, class OM, class OP, class II, class IM>
    void bind (const OI* ptr,
	       OM OI::*output_member_ptr,
	       const parameter_handle<OP>& output_parameter,
	       const automaton_handle<II>& input_automaton,
	       IM II::*input_member_ptr) {
      void (system::*bind_ptr) (const action<OI, OM>&,
      				const action<II, IM>&,
      				bind_proxy&,
				unbind_success_proxy&) = &system::bind;
      schedule (make_runnable (boost::bind (bind_ptr,
					    boost::ref (m_system),
					    make_action (get_current_handle (ptr), output_member_ptr, output_parameter),
					    make_action (input_automaton, input_member_ptr),
					    boost::ref (m_bind_proxy),
					    boost::ref (m_unbind_success_proxy))));
    }
    
    template <class OI, class OM, class II, class IM, class IP>
    void bind (const II* ptr,
	       const automaton_handle<OI>& output_automaton,
	       OM OI::*output_member_ptr,
	       IM II::*input_member_ptr,
	       const parameter_handle<IP>& input_parameter) {
      void (system::*bind_ptr) (const action<OI, OM>&,
      				const action<II, IM>&,
      				bind_proxy&,
				unbind_success_proxy&) = &system::bind;
      schedule (make_runnable (boost::bind (bind_ptr,
					    boost::ref (m_system),
					    make_action (output_automaton, output_member_ptr),
					    make_action (get_current_handle (ptr), input_member_ptr, input_parameter),
					    boost::ref (m_bind_proxy),
					    boost::ref (m_unbind_success_proxy))));
    }

    template <class C, class OI, class OM, class II, class IM>
    void unbind (const C* ptr,
		 const automaton_handle<OI>& output_automaton,
		 OM OI::*output_member_ptr,
		 const automaton_handle<II>& input_automaton,
		 IM II::*input_member_ptr) {
      void (system::*unbind_ptr) (const action<OI, OM>&,
				  const action<II, IM>&,
				  const automaton_handle<C>&,
				  unbind_failure_proxy&) = &system::unbind;
      schedule (make_runnable (boost::bind (unbind_ptr,
					    boost::ref (m_system),
					    make_action (output_automaton, output_member_ptr),
					    make_action (input_automaton, input_member_ptr),
					    get_current_handle (ptr),
					    boost::ref (m_unbind_failure_proxy))));
    }

    template <class OI, class OM, class OP, class II, class IM>
    void unbind (const OI* ptr,
	       OM OI::*output_member_ptr,
	       const parameter_handle<OP>& output_parameter,
	       const automaton_handle<II>& input_automaton,
	       IM II::*input_member_ptr) {
      void (system::*unbind_ptr) (const action<OI, OM>&,
				  const action<II, IM>&,
				  unbind_failure_proxy&) = &system::unbind;
      schedule (make_runnable (boost::bind (unbind_ptr,
					    boost::ref (m_system),
					    make_action (get_current_handle (ptr), output_member_ptr, output_parameter),
					    make_action (input_automaton, input_member_ptr),
					    boost::ref (m_unbind_failure_proxy))));
    }
    
    template <class OI, class OM, class II, class IM, class IP>
    void unbind (const II* ptr,
	       const automaton_handle<OI>& output_automaton,
	       OM OI::*output_member_ptr,
	       IM II::*input_member_ptr,
	       const parameter_handle<IP>& input_parameter) {
      void (system::*unbind_ptr) (const action<OI, OM>&,
				  const action<II, IM>&,
				  unbind_failure_proxy&) = &system::unbind;
      schedule (make_runnable (boost::bind (unbind_ptr,
					    boost::ref (m_system),
					    make_action (output_automaton, output_member_ptr),
					    make_action (get_current_handle (ptr), input_member_ptr, input_parameter),
					    boost::ref (m_unbind_failure_proxy))));
    }

    template <class C, class P>
    void rescind (const C* ptr,
		  const parameter_handle<P>& parameter) {
      void (system::*rescind_ptr) (const automaton_handle<C>&,
				   const parameter_handle<P>&,
				   rescind_failure_proxy&) = &system::rescind;
      schedule (make_runnable (boost::bind (rescind_ptr,
					    boost::ref (m_system),
					    get_current_handle (ptr),
					    parameter,
					    boost::ref (m_rescind_failure_proxy))));
    }

    template <class C, class I>
    void destroy (const C* ptr,
		  const automaton_handle<I>& automaton) {
      void (system::*destroy_ptr) (const automaton_handle<C>&,
				   const automaton_handle<I>&,
				   destroy_failure_proxy&) = &system::destroy;
      schedule (make_runnable (boost::bind (destroy_ptr,
					    boost::ref (m_system),
					    get_current_handle (ptr),
					    automaton,
					    boost::ref (m_destroy_failure_proxy))));
    }

  private:
    template <class I, class M>
    void schedule (const action<I, M>& ac,
  		   output_category /* */) {
      void (system::*execute_ptr) (const output_action_interface&,
      				   scheduler_interface&,
      				   execute_proxy&) = &system::execute;
      schedule (make_runnable (boost::bind (execute_ptr,
					    boost::ref (m_system),
					    ac,
					    boost::ref (*this),
					    boost::ref (m_execute_proxy))));
    }
    
    template <class I, class M>
    void schedule (const action<I, M>& ac,
  		   internal_category /* */) {
      void (system::*execute_ptr) (const independent_action_interface&,
      				   scheduler_interface&,
      				   execute_proxy&) = &system::execute;
      schedule (make_runnable (boost::bind (execute_ptr,
					    boost::ref (m_system),
					    ac,
					    boost::ref (*this),
					    boost::ref (m_execute_proxy))));
    }
    
  public:
    template <class I, class M>
    void schedule (const I* ptr,
		   M I::*member_ptr) {
      action<I, M> ac = make_action (get_current_handle (ptr), member_ptr);
      schedule (ac, typename action<I, M>::action_category ());
    }

    template <class T>
    void run (T* instance) {
      m_system.create (instance, m_create_proxy, m_destroy_success_proxy);

      for (;;) {
	runnable_interface* r = m_runq.pop ();
	(*r) ();
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

}

#endif
