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

      void instance_exists (const void* instance) {
	// Create without a creator failed.  Something is wrong.
	BOOST_ASSERT (false);
      }

      void automaton_created (const generic_automaton_handle& automaton) {
	// Initialize the new created root automaton.
	m_scheduler.schedule (make_system_event (m_scheduler.m_system,
						 automaton,
						 boost::bind (&automaton_interface::init,
							      automaton.value ()),
						 m_scheduler,
						 m_scheduler.m_execute_proxy));
      }

      void automaton_dne (const generic_automaton_handle& automaton,
			  automaton_interface* instance) {
	// An automaton attempting to create another automaton was destroyed.
	// Do nothing.
      }

      void instance_exists (const generic_automaton_handle& automaton,
			    void* instance) {
	m_scheduler.schedule (make_system_event (m_scheduler.m_system,
						 automaton,
						 boost::bind (&automaton_interface::instance_exists,
							      automaton.value (),
							      instance),
						 m_scheduler,
						 m_scheduler.m_execute_proxy));
      }

      void automaton_created (const generic_automaton_handle& automaton,
			      const generic_automaton_handle& child) {
	// Tell the parent about its child.
	m_scheduler.schedule (make_system_event (m_scheduler.m_system,
						 automaton,
						 boost::bind (&automaton_interface::automaton_created,
							      automaton.value (),
							      child),
						 m_scheduler,
						 m_scheduler.m_execute_proxy));
	// Initialize the child.
	m_scheduler.schedule (make_system_event (m_scheduler.m_system,
						 child,
						 boost::bind (&automaton_interface::init,
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

      void automaton_dne (const generic_automaton_handle&,
			  void*) {
	// An automaton attempted to declare a parameter was destroyed.
	// Do nothing.
      }

      void parameter_exists (const generic_automaton_handle& automaton,
			     const generic_parameter_handle& parameter) {
	m_scheduler.schedule (make_system_event (m_scheduler.m_system,
						 automaton,
						 boost::bind (&automaton_interface::parameter_exists,
							      automaton.value (),
							      parameter),
						 m_scheduler,
						 m_scheduler.m_execute_proxy));
      }

      void parameter_declared (const generic_automaton_handle& automaton,
			       const generic_parameter_handle& parameter) {
	m_scheduler.schedule (make_system_event (m_scheduler.m_system,
						 automaton,
						 boost::bind (&automaton_interface::parameter_declared,
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

      template <class OM, class IM>
      void automaton_dne (const action<OM>& output_action,
			  const action<IM>& input_action,
			  const generic_automaton_handle& binder) {
	// An automaton attempting to bind was destroyed.
	// Do nothing.
      }

      template <class OM, class IM>
      void output_automaton_dne (const action<OM>& output_action,
				 const action<IM>& input_action,
				 const generic_automaton_handle& binder) {
	m_scheduler.schedule (make_system_event (m_scheduler.m_system,
						 binder,
						 boost::bind (&automaton_interface::bind_output_automaton_dne,
							      binder.value ()),
						 m_scheduler,
						 m_scheduler.m_execute_proxy));
      }

      template <class OM, class IM>
      void input_automaton_dne (const action<OM>& output_action,
				const action<IM>& input_action,
				const generic_automaton_handle& binder) {
	m_scheduler.schedule (make_system_event (m_scheduler.m_system,
						 binder,
						 boost::bind (&automaton_interface::bind_input_automaton_dne,
							      binder.value ()),
						 m_scheduler,
						 m_scheduler.m_execute_proxy));
      }

      template <class OM, class IM>
      void output_parameter_dne (const action<OM>& output_action,
				 const action<IM>& input_action,
				 const generic_automaton_handle& binder) {
	m_scheduler.schedule (make_system_event (m_scheduler.m_system,
						 binder,
						 boost::bind (&automaton_interface::bind_output_parameter_dne,
							      binder.value ()),
						 m_scheduler,
						 m_scheduler.m_execute_proxy));
      }

      template <class OM, class IM>
      void input_parameter_dne (const action<OM>& output_action,
				const action<IM>& input_action,
				const generic_automaton_handle& binder) {
	m_scheduler.schedule (make_system_event (m_scheduler.m_system,
						 binder,
						 boost::bind (&automaton_interface::bind_input_parameter_dne,
							      binder.value ()),
						 m_scheduler,
						 m_scheduler.m_execute_proxy));
      }

      template <class OM, class IM>
      void binding_exists (const action<OM>& output_action,
			   const action<IM>& input_action,
			   const generic_automaton_handle& binder) {
	m_scheduler.schedule (make_system_event (m_scheduler.m_system,
						 binder,
						 boost::bind (&automaton_interface::binding_exists,
							      binder.value ()),
						 m_scheduler,
						 m_scheduler.m_execute_proxy));
      }

      template <class OM, class IM>
      void input_action_unavailable (const action<OM>& output_action,
				     const action<IM>& input_action,
				     const generic_automaton_handle& binder) {
	m_scheduler.schedule (make_system_event (m_scheduler.m_system,
						 binder,
						 boost::bind (&automaton_interface::input_action_unavailable,
							      binder.value ()),
						 m_scheduler,
						 m_scheduler.m_execute_proxy));
      }

      template <class OM, class IM>
      void output_action_unavailable (const action<OM>& output_action,
				      const action<IM>& input_action,
				      const generic_automaton_handle& binder) {
	m_scheduler.schedule (make_system_event (m_scheduler.m_system,
						 binder,
						 boost::bind (&automaton_interface::output_action_unavailable,
							      binder.value ()),
						 m_scheduler,
						 m_scheduler.m_execute_proxy));
      }

      template <class OM, class IM>
      void bound (const action<OM>& output_action,
		  const action<IM>& input_action,
		  const generic_automaton_handle& binder) {
	m_scheduler.schedule (make_system_event (m_scheduler.m_system,
						 binder,
						 boost::bind (&automaton_interface::bound,
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
      
      template <class OM, class IM>
      void unbound (const action<OM>& output_action,
		    const action<IM>& input_action,
		    const generic_automaton_handle& binder) {
	m_scheduler.schedule (make_system_event (m_scheduler.m_system,
						 binder,
						 boost::bind (&automaton_interface::unbound,
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

      template <class OM, class IM>
      void automaton_dne (const action<OM>& output_action,
			  const action<IM>& input_action,
			  const generic_automaton_handle& binder) {
	// An automaton attempted to unbind was destroyed.
	// Do nothing.
      }

      template <class OM, class IM>
      void output_automaton_dne (const action<OM>& output_action,
				 const action<IM>& input_action,
				 const generic_automaton_handle& binder) {
	m_scheduler.schedule (make_system_event (m_scheduler.m_system,
						 binder,
						 boost::bind (&automaton_interface::unbind_output_automaton_dne,
							      binder.value ()),
						 m_scheduler,
						 m_scheduler.m_execute_proxy));
      }

      template <class OM, class IM>
      void input_automaton_dne (const action<OM>& output_action,
				const action<IM>& input_action,
				const generic_automaton_handle& binder) {
	m_scheduler.schedule (make_system_event (m_scheduler.m_system,
						 binder,
						 boost::bind (&automaton_interface::unbind_input_automaton_dne,
							      binder.value ()),
						 m_scheduler,
						 m_scheduler.m_execute_proxy));
      }

      template <class OM, class IM>
      void output_parameter_dne (const action<OM>& output_action,
				 const action<IM>& input_action,
				 const generic_automaton_handle& binder) {
	m_scheduler.schedule (make_system_event (m_scheduler.m_system,
						 binder,
						 boost::bind (&automaton_interface::unbind_output_parameter_dne,
							      binder.value ()),
						 m_scheduler,
						 m_scheduler.m_execute_proxy));
      }

      template <class OM, class IM>
      void input_parameter_dne (const action<OM>& output_action,
				const action<IM>& input_action,
				const generic_automaton_handle& binder) {
	m_scheduler.schedule (make_system_event (m_scheduler.m_system,
						 binder,
						 boost::bind (&automaton_interface::unbind_input_parameter_dne,
							      binder.value ()),
						 m_scheduler,
						 m_scheduler.m_execute_proxy));
      }

      template <class OM, class IM>
      void binding_dne (const action<OM>& output_action,
			const action<IM>& input_action,
			const generic_automaton_handle& binder) {
      	m_scheduler.schedule (make_system_event (m_scheduler.m_system,
      						 binder,
      						 boost::bind (&automaton_interface::binding_dne,
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

      void parameter_rescinded (const generic_automaton_handle& automaton,
				const generic_parameter_handle& parameter) {
	m_scheduler.schedule (make_system_event (m_scheduler.m_system,
						 automaton,
						 boost::bind (&automaton_interface::parameter_rescinded,
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

      void automaton_dne (const generic_automaton_handle&,
			  const generic_parameter_handle&) {
	// An automaton attempting to rescind a parameter was destroyed.
	// Do nothing.
      }

      void parameter_dne (const generic_automaton_handle& automaton,
			  const generic_parameter_handle& parameter) {

	m_scheduler.schedule (make_system_event (m_scheduler.m_system,
						 automaton,
						 boost::bind (&automaton_interface::parameter_dne,
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

      void automaton_destroyed (const generic_automaton_handle& automaton) {
	// Root automaton destroyed.
      }

      void automaton_destroyed (const generic_automaton_handle& automaton,
				const generic_automaton_handle& child) {
	m_scheduler.schedule (make_system_event (m_scheduler.m_system,
						 automaton,
						 boost::bind (&automaton_interface::automaton_destroyed,
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

      void automaton_dne (const generic_automaton_handle&,
			  const generic_automaton_handle&) {
	// An automaton attempting to destroy another automaton was itself destroyed.
	// Do nothing.
      }

      void target_automaton_dne (const generic_automaton_handle& automaton,
				 const generic_automaton_handle& target) {
	m_scheduler.schedule (make_system_event (m_scheduler.m_system,
						 automaton,
						 boost::bind (&automaton_interface::target_automaton_dne,
							      automaton.value (),
							      target),
						 m_scheduler,
						 m_scheduler.m_execute_proxy));
      }

      void destroyer_not_creator (const generic_automaton_handle& automaton,
				  const generic_automaton_handle& target) {
	m_scheduler.schedule (make_system_event (m_scheduler.m_system,
						 automaton,
						 boost::bind (&automaton_interface::destroyer_not_creator,
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

    template <class C>
    void create (const C* ptr,
		 automaton_interface* instance) {
      void (system::*create_ptr) (const generic_automaton_handle&,
      				  automaton_interface*,
      				  create_proxy&,
				  destroy_success_proxy&) = &system::create;
      schedule (make_runnable (boost::bind (create_ptr,
					    boost::ref (m_system),
					    get_current_handle (ptr),
					    instance,
					    boost::ref (m_create_proxy),
					    boost::ref (m_destroy_success_proxy))));
    }
    
    template <class C>
    void declare (const C* ptr,
		  void* parameter) {
      schedule (make_runnable (boost::bind (&system::declare<declare_proxy, rescind_success_proxy>,
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
      void (system::*bind_ptr) (const action<OM>&,
      				const action<IM>&,
      				const generic_automaton_handle&,
      				bind_proxy&,
				unbind_success_proxy&) = &system::bind<OM, IM>;
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
      void (system::*bind_ptr) (const action<OM>&,
      				const action<IM>&,
      				bind_proxy&,
				unbind_success_proxy&) = &system::bind<OM, IM>;
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
      void (system::*bind_ptr) (const action<OM>&,
      				const action<IM>&,
      				bind_proxy&,
				unbind_success_proxy&) = &system::bind<OM, IM>;
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
      void (system::*unbind_ptr) (const action<OM>&,
				  const action<IM>&,
				  const generic_automaton_handle&,
				  unbind_failure_proxy&) = &system::unbind<OM, IM>;
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
      void (system::*unbind_ptr) (const action<OM>&,
				  const action<IM>&,
				  unbind_failure_proxy&) = &system::unbind<OM, IM>;
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
      void (system::*unbind_ptr) (const action<OM>&,
				  const action<IM>&,
				  unbind_failure_proxy&) = &system::unbind<OM, IM>;
      schedule (make_runnable (boost::bind (unbind_ptr,
					    boost::ref (m_system),
					    make_action (output_automaton, output_member_ptr),
					    make_action (get_current_handle (ptr), input_member_ptr, input_parameter),
					    boost::ref (m_unbind_failure_proxy))));
    }

    template <class C>
    void rescind (const C* ptr,
		  const generic_parameter_handle& parameter) {
      schedule (make_runnable (boost::bind (&system::rescind<rescind_failure_proxy>,
					    boost::ref (m_system),
					    get_current_handle (ptr),
					    parameter,
					    boost::ref (m_rescind_failure_proxy))));
    }

    template <class C>
    void destroy (const C* ptr,
		  const generic_automaton_handle& automaton) {
      schedule (make_runnable (boost::bind (&system::destroy<destroy_failure_proxy>,
					    boost::ref (m_system),
					    get_current_handle (ptr),
					    automaton,
					    boost::ref (m_destroy_failure_proxy))));
    }

  private:
    template <class M>
    void schedule (const action<M>& ac,
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
    
    template <class M>
    void schedule (const action<M>& ac,
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
      action<M> ac = make_action (get_current_handle (ptr), member_ptr);
      schedule (ac, typename action<M>::action_category ());
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
