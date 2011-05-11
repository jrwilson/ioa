#ifndef __simple_scheduler_hpp__
#define __simple_scheduler_hpp__

namespace ioa {

  class simple_scheduler :
    public internal_scheduler_interface
  {
  private:
    system m_system;
    blocking_queue<runnable_interface*> m_runq;
    generic_automaton_handle m_current_handle;

    void set_current_handle (const generic_automaton_handle& handle) {
      m_current_handle = handle;
    }

    void clear_current_handle () {
      m_current_handle = generic_automaton_handle ();
    }

    static bool keep_going () {
      return runnable_interface::count () != 0;
    }

  public:
    simple_scheduler ()
    { }

    system& get_system () {
      return m_system;
    }

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

    void schedule (runnable_interface* r) {
      m_runq.push (r);
    }

    template <class T>
    void run (T* instance) {
      m_system.create (instance, *this);

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

  private:

    void automaton_dne () {
      // Do nothing.
    }

    // create_listener_interface

    void instance_exists (const void* instance) {
      // Create without a creator failed so do nothing.
    }

    void automaton_created (const generic_automaton_handle& automaton) {
      m_runq.push (make_system_event (*this, m_system, automaton, boost::bind (&automaton_interface::init, automaton.value ())));
    }

    void instance_exists (const generic_automaton_handle& creator,
    			  const void* instance) {
      m_runq.push (make_system_event (*this, m_system, creator, boost::bind (&automaton_interface::instance_exists, creator.value (), instance)));
    }
    
    void automaton_created (const generic_automaton_handle& creator,
    			    const generic_automaton_handle& automaton) {
      m_runq.push (make_system_event (*this, m_system, creator, boost::bind (&automaton_interface::automaton_created, creator.value (), automaton)));
      m_runq.push (make_system_event (*this, m_system, automaton, boost::bind (&automaton_interface::init, automaton.value ())));
    }
    
    // declare_listener_interface

    void parameter_exists (const generic_automaton_handle& automaton,
    			   void* parameter) {
      m_runq.push (make_system_event (*this, m_system, automaton, boost::bind (&automaton_interface::parameter_exists, automaton.value (), parameter)));
    }

    void parameter_declared (const generic_automaton_handle& automaton,
    			     const generic_parameter_handle& parameter) {
      m_runq.push (make_system_event (*this, m_system, automaton, boost::bind (&automaton_interface::parameter_declared, automaton.value (), parameter)));
    }

    // bind_listener_interface

    void bind_output_automaton_dne (const generic_automaton_handle& binder) {
      m_runq.push (make_system_event (*this, m_system, binder, boost::bind (&automaton_interface::bind_output_automaton_dne, binder.value ())));
    }

    void bind_input_automaton_dne (const generic_automaton_handle& binder) {
      m_runq.push (make_system_event (*this, m_system, binder, boost::bind (&automaton_interface::bind_input_automaton_dne, binder.value ())));
    }

    void bind_output_parameter_dne (const generic_automaton_handle& binder) {
      m_runq.push (make_system_event (*this, m_system, binder, boost::bind (&automaton_interface::bind_output_parameter_dne, binder.value ())));
    }

    void bind_input_parameter_dne (const generic_automaton_handle& binder) {
      m_runq.push (make_system_event (*this, m_system, binder, boost::bind (&automaton_interface::bind_input_parameter_dne, binder.value ())));
    }

    void binding_exists (const generic_automaton_handle& binder) {
      m_runq.push (make_system_event (*this, m_system, binder, boost::bind (&automaton_interface::binding_exists, binder.value ())));
    }

    void input_action_unavailable (const generic_automaton_handle& binder) {
      m_runq.push (make_system_event (*this, m_system, binder, boost::bind (&automaton_interface::input_action_unavailable, binder.value ())));
    }

    void output_action_unavailable (const generic_automaton_handle& binder) {
      m_runq.push (make_system_event (*this, m_system, binder, boost::bind (&automaton_interface::output_action_unavailable, binder.value ())));
    }

    void bound (const generic_automaton_handle& binder) {
      m_runq.push (make_system_event (*this, m_system, binder, boost::bind (&automaton_interface::bound, binder.value ())));
      // TODO:  Send bound signals to input and output.
      // 	m_scheduler.schedule (make_bound (m_scheduler, m_output_action));
      // 	m_scheduler.schedule (make_bound (m_scheduler, m_input_action));
    }

    // unbind_listener_interface

    void unbind_output_automaton_dne (const generic_automaton_handle& binder) {
      m_runq.push (make_system_event (*this, m_system, binder, boost::bind (&automaton_interface::unbind_output_automaton_dne, binder.value ())));
    }

    void unbind_input_automaton_dne (const generic_automaton_handle& binder) {
      m_runq.push (make_system_event (*this, m_system, binder, boost::bind (&automaton_interface::unbind_input_automaton_dne, binder.value ())));
    }

    void unbind_output_parameter_dne (const generic_automaton_handle& binder) {
      m_runq.push (make_system_event (*this, m_system, binder, boost::bind (&automaton_interface::unbind_output_parameter_dne, binder.value ())));
    }

    void unbind_input_parameter_dne (const generic_automaton_handle& binder) {
      m_runq.push (make_system_event (*this, m_system, binder, boost::bind (&automaton_interface::unbind_input_parameter_dne, binder.value ())));
    }

    void binding_dne (const generic_automaton_handle& binder) {
      m_runq.push (make_system_event (*this, m_system, binder, boost::bind (&automaton_interface::binding_dne, binder.value ())));
    }

    void unbound (const generic_automaton_handle& binder) {
      m_runq.push (make_system_event (*this, m_system, binder, boost::bind (&automaton_interface::unbound, binder.value ())));
      // TODO:  Send bound signals to input and output.
      // 	m_scheduler.schedule (make_bound (m_scheduler, m_output_action));
      // 	m_scheduler.schedule (make_bound (m_scheduler, m_input_action));
    }

    // TODO:  Eliminate previous.

    void unbound (const generic_automaton_handle& output_automaton,
		  const void* output_member_ptr,
		  const generic_automaton_handle& input_automaton,
		  const void* input_member_ptr,
		  const generic_automaton_handle& binder_automaton) {
      // TODO
      BOOST_ASSERT (false);

    }

    void unbound (const generic_automaton_handle& output_automaton,
		  const void* output_member_ptr,
		  const generic_parameter_handle& output_parameter,
		  const generic_automaton_handle& input_automaton,
		  const void* input_member_ptr) {
      // TODO
      BOOST_ASSERT (false);

    }

    void unbound (const generic_automaton_handle& output_automaton,
		  const void* output_member_ptr,
		  const generic_automaton_handle& input_automaton,
		  const void* input_member_ptr,
		  const generic_parameter_handle& input_parameter) {
      // TODO
      BOOST_ASSERT (false);

    }

    void parameter_dne (const generic_automaton_handle& automaton,
			const generic_parameter_handle& parameter) {
      m_runq.push (make_system_event (*this, m_system, automaton, boost::bind (&automaton_interface::parameter_dne, automaton.value (), parameter)));
    }
    
    void parameter_rescinded (const generic_automaton_handle& automaton,
			      void* parameter) {
      m_runq.push (make_system_event (*this, m_system, automaton, boost::bind (&automaton_interface::parameter_rescinded, automaton.value (), parameter)));
    }

    void target_automaton_dne (const generic_automaton_handle& automaton) {
      // Top-level destroy didn't succeed.
    }

    void target_automaton_dne (const generic_automaton_handle& automaton,
			       const generic_automaton_handle& target) {
      m_runq.push (make_system_event (*this, m_system, automaton, boost::bind (&automaton_interface::target_automaton_dne, automaton.value (), target)));
    }

    void destroyer_not_creator (const generic_automaton_handle& automaton,
				const generic_automaton_handle& target) {
 m_runq.push (make_system_event (*this, m_system, automaton, boost::bind (&automaton_interface::destroyer_not_creator, automaton.value (), target)));
    }

    void automaton_destroyed (const generic_automaton_handle& automaton) {
      // Do nothing.
    }

    void automaton_destroyed (const generic_automaton_handle& parent,
			      const generic_automaton_handle& child) {
      m_runq.push (make_system_event (*this, m_system, parent, boost::bind (&automaton_interface::automaton_destroyed, parent.value (), child)));
    }

    // execute_listener_interface

    void execute_parameter_dne () {
      // TODO
      BOOST_ASSERT (false);
    }

  };

}

#endif
