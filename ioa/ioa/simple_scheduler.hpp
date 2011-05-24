#ifndef __simple_scheduler_hpp__
#define __simple_scheduler_hpp__

#include "blocking_queue.hpp"
#include "runnable.hpp"
#include "system.hpp"
#include <boost/bind.hpp>

namespace ioa {

  // TODO:  DUPLICATE ACTIONS!!!
  // TODO:  SYSTEM CALL FAIRNESS!!!
  // TODO:  EVENTS!!!
  // TODO:  automaton_dne consistency.
  // TODO:  What happens when we send an event to a destroyed automaton?

  class simple_scheduler :
    public scheduler_interface
  {
  private:
    bool m_running;
    system m_system;
    blocking_queue<runnable_interface*> m_runq;
    aid_t m_current_aid;
    const automaton_interface* m_current_this;
    
    template <class I>
    automaton_handle<I> get_current_aid (const I* ptr) const {
      // The system uses set_current_aid to alert the scheduler that the code that is executing belongs to the given automaton.
      // When the automaton invokes the scheduler, this ID is used in the production of the corresponding runnable.
      // To be type-safe, we require an automaton_handle<T> instead of an aid_t.
      // This function (get_current_aid) is responsible for producing the handle.

      // First, we check that set_current_aid was called.
      BOOST_ASSERT (m_current_aid != -1);
      BOOST_ASSERT (m_current_this != 0);

      // Second, we need to make sure that the user didn't inappropriately cast "this."
      const I* tmp = dynamic_cast<const I*> (m_current_this);
      BOOST_ASSERT (tmp == ptr);

      return m_system.cast_aid (ptr, m_current_aid);
    }

    void set_current_aid (const aid_t aid) {
      // This is to be used during generation so that any allocated memory can be associated with the automaton.
      BOOST_ASSERT (aid != -1);
      m_current_aid = aid;
      m_current_this = 0;
    }

    void set_current_aid (const aid_t aid,
			  const automaton_interface* current_this) {
      // This is for all cases except generation.
      BOOST_ASSERT (aid != -1);
      BOOST_ASSERT (current_this != 0);
      m_current_aid = aid;
      m_current_this = current_this;
    }

    void clear_current_aid () {
      m_current_aid = -1;
      m_current_this = 0;
    }

    static bool keep_going () {
      return runnable_interface::count () != 0;
    }

    void schedule (runnable_interface* r) {
      if (m_running) {
	m_runq.push (r);
      }
      else {
	delete r;
      }
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
	// Init the new automaton.
	void (system::*ptr2) (const automaton_handle<I>&,
			      scheduler_interface&,
			      execute_proxy&) = &system::init;
	m_scheduler.schedule (make_runnable (boost::bind (ptr2,
							  boost::ref (m_scheduler.m_system),
							  automaton,
							  boost::ref (m_scheduler),
							  boost::ref (m_scheduler.m_execute_proxy))));
      }

      template <class P, class D>
      void automaton_dne (const automaton_handle<P>& automaton,
			  D&) {
	// An automaton attempting to create another automaton was destroyed.
	// Do nothing.
      }

      template <class P, class I, class D>
      void instance_exists (const automaton_handle<P>& automaton,
			    I* instance,
			    D& d) {
	void (system::*ptr) (const automaton_handle<P>&,
			     const I*,
			     scheduler_interface&,
			     execute_proxy&,
			     D&) = &system::instance_exists;
	m_scheduler.schedule (make_runnable (boost::bind (ptr,
							  boost::ref (m_scheduler.m_system),
							  automaton,
							  instance,
							  boost::ref (m_scheduler),
							  boost::ref (m_scheduler.m_execute_proxy),
							  boost::ref (d))));
      }

      template <class P, class I, class D>
      void automaton_created (const automaton_handle<P>& automaton,
			      const automaton_handle<I>& child,
			      D& d) {
	// Tell the parent.
	void (system::*ptr1) (const automaton_handle<P>&,
			      const automaton_handle<I>&,
			      scheduler_interface&,
			      execute_proxy&,
			      D&) = &system::automaton_created;
	m_scheduler.schedule (make_runnable (boost::bind (ptr1,
							  boost::ref (m_scheduler.m_system),
							  automaton,
							  child,
							  boost::ref (m_scheduler),
							  boost::ref (m_scheduler.m_execute_proxy),
							  boost::ref (d))));
	// Init the child.
	void (system::*ptr2) (const automaton_handle<I>&,
			      scheduler_interface&,
			      execute_proxy&) = &system::init;
	m_scheduler.schedule (make_runnable (boost::bind (ptr2,
							  boost::ref (m_scheduler.m_system),
							  child,
							  boost::ref (m_scheduler),
							  boost::ref (m_scheduler.m_execute_proxy))));
      }

      create_proxy (simple_scheduler& scheduler) :
	m_scheduler (scheduler)
      { }
    };
    create_proxy m_create_proxy;

    struct declare_proxy
    {
      simple_scheduler& m_scheduler;

      template <class I, class P, class D>
      void automaton_dne (const automaton_handle<I>&,
			  P*,
			  D&) {
	// An automaton attempted to declare a parameter was destroyed.
	// Do nothing.
      }

      template <class I, class D>
      void parameter_exists (const automaton_handle<I>& automaton,
			     D& d) {
	void (system::*ptr) (const automaton_handle<I>&,
			     scheduler_interface&,
			     execute_proxy&,
			     D&) = &system::parameter_exists;
	m_scheduler.schedule (make_runnable (boost::bind (ptr,
							  boost::ref (m_scheduler.m_system),
							  automaton,
							  boost::ref (m_scheduler),
							  boost::ref (m_scheduler.m_execute_proxy),
							  boost::ref (d))));
      }

      template <class I, class P, class D>
      void parameter_declared (const automaton_handle<I>& automaton,
			       const parameter_handle<P>& parameter,
			       D& d) {
	void (system::*ptr) (const automaton_handle<I>&,
			     const parameter_handle<P>&,
			     scheduler_interface&,
			     execute_proxy&,
			     D&) = &system::parameter_declared;
	m_scheduler.schedule (make_runnable (boost::bind (ptr,
							  boost::ref (m_scheduler.m_system),
							  automaton,
							  parameter,
							  boost::ref (m_scheduler),
							  boost::ref (m_scheduler.m_execute_proxy),
							  boost::ref (d))));
      }
      
      declare_proxy (simple_scheduler& scheduler) :
	m_scheduler (scheduler)
      { }
    };
    declare_proxy m_declare_proxy;

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

      template <class OI, class OM, class II, class IM, class I, class D>
      void output_automaton_dne (const action<OI, OM>& output_action,
				 const action<II, IM>& input_action,
				 const automaton_handle<I>& binder,
				 D& d) {
	void (system::*ptr) (const automaton_handle<I>&,
			     scheduler_interface&,
			     execute_proxy&,
			     D&) = &system::bind_output_automaton_dne;
	m_scheduler.schedule (make_runnable (boost::bind (ptr,
							  boost::ref (m_scheduler.m_system),
							  binder,
							  boost::ref (m_scheduler),
							  boost::ref (m_scheduler.m_execute_proxy),
							  boost::ref (d))));
      }

      template <class OI, class OM, class II, class IM, class I, class D>
      void input_automaton_dne (const action<OI, OM>& output_action,
				const action<II, IM>& input_action,
				const automaton_handle<I>& binder,
				D& d) {
	void (system::*ptr) (const automaton_handle<I>&,
			     scheduler_interface&,
			     execute_proxy&,
			     D&) = &system::bind_input_automaton_dne;
	m_scheduler.schedule (make_runnable (boost::bind (ptr,
							  boost::ref (m_scheduler.m_system),
							  binder,
							  boost::ref (m_scheduler),
							  boost::ref (m_scheduler.m_execute_proxy),
							  boost::ref (d))));
      }

      template <class OI, class OM, class II, class IM, class I, class D>
      void output_parameter_dne (const action<OI, OM>& output_action,
				 const action<II, IM>& input_action,
				 const automaton_handle<I>& binder,
				 D& d) {
	void (system::*ptr) (const automaton_handle<I>&,
			     scheduler_interface&,
			     execute_proxy&,
			     D&) = &system::bind_output_parameter_dne;
	m_scheduler.schedule (make_runnable (boost::bind (ptr,
							  boost::ref (m_scheduler.m_system),
							  binder,
							  boost::ref (m_scheduler),
							  boost::ref (m_scheduler.m_execute_proxy),
							  boost::ref (d))));
      }

      template <class OI, class OM, class II, class IM, class I, class D>
      void input_parameter_dne (const action<OI, OM>& output_action,
				const action<II, IM>& input_action,
				const automaton_handle<I>& binder,
				D& d) {
	void (system::*ptr) (const automaton_handle<I>&,
			     scheduler_interface&,
			     execute_proxy&,
			     D&) = &system::bind_input_parameter_dne;
	m_scheduler.schedule (make_runnable (boost::bind (ptr,
							  boost::ref (m_scheduler.m_system),
							  binder,
							  boost::ref (m_scheduler),
							  boost::ref (m_scheduler.m_execute_proxy),
							  boost::ref (d))));
      }

      template <class OI, class OM, class II, class IM, class I, class D>
      void binding_exists (const action<OI, OM>& output_action,
			   const action<II, IM>& input_action,
			   const automaton_handle<I>& binder,
			   D& d) {
	void (system::*ptr) (const automaton_handle<I>&,
			     scheduler_interface&,
			     execute_proxy&,
			     D&) = &system::binding_exists;
	m_scheduler.schedule (make_runnable (boost::bind (ptr,
							  boost::ref (m_scheduler.m_system),
							  binder,
							  boost::ref (m_scheduler),
							  boost::ref (m_scheduler.m_execute_proxy),
							  boost::ref (d))));
      }

      template <class OI, class OM, class II, class IM, class I, class D>
      void input_action_unavailable (const action<OI, OM>& output_action,
				     const action<II, IM>& input_action,
				     const automaton_handle<I>& binder,
				     D& d) {
	void (system::*ptr) (const automaton_handle<I>&,
			     scheduler_interface&,
			     execute_proxy&,
			     D&) = &system::input_action_unavailable;
	m_scheduler.schedule (make_runnable (boost::bind (ptr,
							  boost::ref (m_scheduler.m_system),
							  binder,
							  boost::ref (m_scheduler),
							  boost::ref (m_scheduler.m_execute_proxy),
							  boost::ref (d))));
      }

      template <class OI, class OM, class II, class IM, class I, class D>
      void output_action_unavailable (const action<OI, OM>& output_action,
				      const action<II, IM>& input_action,
				      const automaton_handle<I>& binder,
				      D& d) {
	void (system::*ptr) (const automaton_handle<I>&,
			     scheduler_interface&,
			     execute_proxy&,
			     D&) = &system::output_action_unavailable;
	m_scheduler.schedule (make_runnable (boost::bind (ptr,
							  boost::ref (m_scheduler.m_system),
							  binder,
							  boost::ref (m_scheduler),
							  boost::ref (m_scheduler.m_execute_proxy),
							  boost::ref (d))));
      }

      template <class OI, class OM, class II, class IM, class I, class D>
      void bound (const action<OI, OM>& output_action,
		  const action<II, IM>& input_action,
		  const automaton_handle<I>& binder,
		  D& d) {
	void (system::*ptr) (const automaton_handle<I>&,
			     scheduler_interface&,
			     execute_proxy&,
			     D&) = &system::bound;
	m_scheduler.schedule (make_runnable (boost::bind (ptr,
							  boost::ref (m_scheduler.m_system),
							  binder,
							  boost::ref (m_scheduler),
							  boost::ref (m_scheduler.m_execute_proxy),
							  boost::ref (d))));

	void (system::*ptr1) (const action<OI, OM>&,
			      scheduler_interface&,
			      execute_proxy&) = &system::action_bound;
	m_scheduler.schedule (make_runnable (boost::bind (ptr1,
							  boost::ref (m_scheduler.m_system),
							  output_action,
							  boost::ref (m_scheduler),
							  boost::ref (m_scheduler.m_execute_proxy))));
	void (system::*ptr2) (const action<II, IM>&,
			      scheduler_interface&,
			      execute_proxy&) = &system::action_bound;
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
      
      template <class OI, class OM, class II, class IM, class I, class D>
      void unbound (const action<OI, OM>& output_action,
		    const action<II, IM>& input_action,
		    const automaton_handle<I>& binder,
		    D& d) {
	void (system::*ptr) (const automaton_handle<I>&,
			     scheduler_interface&,
			     execute_proxy&,
			     D&) = &system::unbound;
	m_scheduler.schedule (make_runnable (boost::bind (ptr,
							  boost::ref (m_scheduler.m_system),
							  binder,
							  boost::ref (m_scheduler),
							  boost::ref (m_scheduler.m_execute_proxy),
							  boost::ref (d))));

	void (system::*ptr1) (const action<OI, OM>&,
			      scheduler_interface&,
			      execute_proxy&) = &system::action_unbound;
	m_scheduler.schedule (make_runnable (boost::bind (ptr1,
							  boost::ref (m_scheduler.m_system),
							  output_action,
							  boost::ref (m_scheduler),
							  boost::ref (m_scheduler.m_execute_proxy))));
	void (system::*ptr2) (const action<II, IM>&,
			      scheduler_interface&,
			      execute_proxy&) = &system::action_unbound;
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

      template <class OI, class OM, class II, class IM, class I, class D>
      void output_automaton_dne (const action<OI, OM>& output_action,
				 const action<II, IM>& input_action,
				 const automaton_handle<I>& binder,
				 D& d) {
	void (system::*ptr) (const automaton_handle<I>&,
			     scheduler_interface&,
			     execute_proxy&,
			     D&) = &system::unbind_output_automaton_dne;
	m_scheduler.schedule (make_runnable (boost::bind (ptr,
							  boost::ref (m_scheduler.m_system),
							  binder,
							  boost::ref (m_scheduler),
							  boost::ref (m_scheduler.m_execute_proxy),
							  boost::ref (d))));
      }

      template <class OI, class OM, class II, class IM, class I, class D>
      void input_automaton_dne (const action<OI, OM>& output_action,
				const action<II, IM>& input_action,
				const automaton_handle<I>& binder,
				D& d) {
	void (system::*ptr) (const automaton_handle<I>&,
			     scheduler_interface&,
			     execute_proxy&,
			     D&) = &system::unbind_input_automaton_dne;
	m_scheduler.schedule (make_runnable (boost::bind (ptr,
							  boost::ref (m_scheduler.m_system),
							  binder,
							  boost::ref (m_scheduler),
							  boost::ref (m_scheduler.m_execute_proxy),
							  boost::ref (d))));
      }

      template <class OI, class OM, class II, class IM, class I, class D>
      void output_parameter_dne (const action<OI, OM>& output_action,
				 const action<II, IM>& input_action,
				 const automaton_handle<I>& binder,
				 D& d) {
	void (system::*ptr) (const automaton_handle<I>&,
			     scheduler_interface&,
			     execute_proxy&,
			     D&) = &system::unbind_output_parameter_dne;
	m_scheduler.schedule (make_runnable (boost::bind (ptr,
							  boost::ref (m_scheduler.m_system),
							  binder,
							  boost::ref (m_scheduler),
							  boost::ref (m_scheduler.m_execute_proxy),
							  boost::ref (d))));
      }

      template <class OI, class OM, class II, class IM, class I, class D>
      void input_parameter_dne (const action<OI, OM>& output_action,
				const action<II, IM>& input_action,
				const automaton_handle<I>& binder,
				D& d) {
	void (system::*ptr) (const automaton_handle<I>&,
			     scheduler_interface&,
			     execute_proxy&,
			     D&) = &system::unbind_input_parameter_dne;
	m_scheduler.schedule (make_runnable (boost::bind (ptr,
							  boost::ref (m_scheduler.m_system),
							  binder,
							  boost::ref (m_scheduler),
							  boost::ref (m_scheduler.m_execute_proxy),
							  boost::ref (d))));
      }

      template <class OI, class OM, class II, class IM, class I, class D>
      void binding_dne (const action<OI, OM>& output_action,
			const action<II, IM>& input_action,
			const automaton_handle<I>& binder,
			D& d) {
	void (system::*ptr) (const automaton_handle<I>&,
			     scheduler_interface&,
			     execute_proxy&,
			     D&) = &system::binding_dne;
	m_scheduler.schedule (make_runnable (boost::bind (ptr,
							  boost::ref (m_scheduler.m_system),
							  binder,
							  boost::ref (m_scheduler),
							  boost::ref (m_scheduler.m_execute_proxy),
							  boost::ref (d))));
      }
      
      unbind_failure_proxy (simple_scheduler& scheduler) :
	m_scheduler (scheduler)
      { }
    };
    unbind_failure_proxy m_unbind_failure_proxy;

    struct rescind_success_proxy
    {
      simple_scheduler& m_scheduler;

      template <class I, class D>
      void parameter_rescinded (const automaton_handle<I>& automaton,
				D& d) {
	void (system::*ptr) (const automaton_handle<I>&,
			     scheduler_interface&,
			     execute_proxy&,
			     D&) = &system::parameter_rescinded;
	m_scheduler.schedule (make_runnable (boost::bind (ptr,
							  boost::ref (m_scheduler.m_system),
							  automaton,
							  boost::ref (m_scheduler),
							  boost::ref (m_scheduler.m_execute_proxy),
							  boost::ref (d))));
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

      template <class I, class D>
      void parameter_dne (const automaton_handle<I>& automaton,
			  D& d) {
	void (system::*ptr) (const automaton_handle<I>&,
			     scheduler_interface&,
			     execute_proxy&,
			     D&) = &system::parameter_dne;
	m_scheduler.schedule (make_runnable (boost::bind (ptr,
							  boost::ref (m_scheduler.m_system),
							  automaton,
							  boost::ref (m_scheduler),
							  boost::ref (m_scheduler.m_execute_proxy),
							  boost::ref (d))));
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

      template <class I, class D>
      void automaton_destroyed (const automaton_handle<I>& automaton,
				D& d) {
	void (system::*ptr) (const automaton_handle<I>&,
			     scheduler_interface&,
			     execute_proxy&,
			     D&) = &system::automaton_destroyed;
	m_scheduler.schedule (make_runnable (boost::bind (ptr,
							  boost::ref (m_scheduler.m_system),
							  automaton,
							  boost::ref (m_scheduler),
							  boost::ref (m_scheduler.m_execute_proxy),
							  boost::ref (d))));
      }
      
      destroy_success_proxy (simple_scheduler& scheduler) :
	m_scheduler (scheduler)
      { }
    };
    destroy_success_proxy m_destroy_success_proxy;

    struct destroy_failure_proxy
    {
      simple_scheduler& m_scheduler;

      template <class P, class I, class D>
      void automaton_dne (const automaton_handle<P>&,
			  const automaton_handle<I>&,
			  D&) {
	// An automaton attempting to destroy another automaton was itself destroyed.
	// Do nothing.
      }

      template <class P, class D>
      void target_automaton_dne (const automaton_handle<P>& automaton,
				 D& d) {
	void (system::*ptr) (const automaton_handle<P>&,
			     scheduler_interface&,
			     execute_proxy&,
			     D&) = &system::target_automaton_dne;
	m_scheduler.schedule (make_runnable (boost::bind (ptr,
							  boost::ref (m_scheduler.m_system),
							  automaton,
							  boost::ref (m_scheduler),
							  boost::ref (m_scheduler.m_execute_proxy),
							  boost::ref (d))));
      }

      template <class P, class D>
      void destroyer_not_creator (const automaton_handle<P>& automaton,
				  D& d) {
	void (system::*ptr) (const automaton_handle<P>&,
			     scheduler_interface&,
			     execute_proxy&,
			     D&) = &system::destroyer_not_creator;
	m_scheduler.schedule (make_runnable (boost::bind (ptr,
							  boost::ref (m_scheduler.m_system),
							  automaton,
							  boost::ref (m_scheduler),
							  boost::ref (m_scheduler.m_execute_proxy),
							  boost::ref (d))));
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
      m_current_aid (-1),
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

    template <class C, class G, class D>
    void create (const C* ptr,
		 G generator,
		 D& d) {
      void (system::*create_ptr) (const automaton_handle<C>&,
      				  G,
				  scheduler_interface&,
      				  create_proxy&,
				  destroy_success_proxy&,
				  D&) = &system::create;
      schedule (make_runnable (boost::bind (create_ptr,
					    boost::ref (m_system),
					    get_current_aid (ptr),
					    generator,  // We want a copy, not a reference.
					    boost::ref (*this),
					    boost::ref (m_create_proxy),
					    boost::ref (m_destroy_success_proxy),
					    boost::ref (d))));
    }
    
    template <class C, class P, class D>
    void declare (const C* ptr,
		  P* parameter,
		  D& d) {
      void (system::*declare_ptr) (const automaton_handle<C>&,
				   P*,
				   declare_proxy&,
				   rescind_success_proxy&,
				   D&) = &system::declare;
      schedule (make_runnable (boost::bind (declare_ptr,
					    boost::ref (m_system),
					    get_current_aid (ptr),
					    parameter,
					    boost::ref (m_declare_proxy),
					    boost::ref (m_rescind_success_proxy),
					    boost::ref (d))));
    }

    template <class C, class OI, class OM, class II, class IM, class D>
    void bind (const C* ptr,
	       const automaton_handle<OI>& output_automaton,
	       OM OI::*output_member_ptr,
	       const automaton_handle<II>& input_automaton,
	       IM II::*input_member_ptr,
	       D& d) {
      void (system::*bind_ptr) (const action<OI, OM>&,
      				const action<II, IM>&,
      				const automaton_handle<C>&,
      				bind_proxy&,
				unbind_success_proxy&,
				D&) = &system::bind;
      schedule (make_runnable (boost::bind (bind_ptr,
					    boost::ref (m_system),
					    make_action (output_automaton, output_member_ptr),
					    make_action (input_automaton, input_member_ptr),
					    get_current_aid (ptr),
					    boost::ref (m_bind_proxy),
					    boost::ref (m_unbind_success_proxy),
					    boost::ref (d))));
    }

    template <class OI, class OM, class OP, class II, class IM, class D>
    void bind (const OI* ptr,
	       OM OI::*output_member_ptr,
	       const parameter_handle<OP>& output_parameter,
	       const automaton_handle<II>& input_automaton,
	       IM II::*input_member_ptr,
	       D& d) {
      void (system::*bind_ptr) (const action<OI, OM>&,
      				const action<II, IM>&,
      				bind_proxy&,
				unbind_success_proxy&,
				D&) = &system::bind;
      schedule (make_runnable (boost::bind (bind_ptr,
					    boost::ref (m_system),
					    make_action (get_current_aid (ptr), output_member_ptr, output_parameter),
					    make_action (input_automaton, input_member_ptr),
					    boost::ref (m_bind_proxy),
					    boost::ref (m_unbind_success_proxy),
					    boost::ref (d))));
    }
    
    template <class OI, class OM, class II, class IM, class IP, class D>
    void bind (const II* ptr,
	       const automaton_handle<OI>& output_automaton,
	       OM OI::*output_member_ptr,
	       IM II::*input_member_ptr,
	       const parameter_handle<IP>& input_parameter,
	       D& d) {
      void (system::*bind_ptr) (const action<OI, OM>&,
      				const action<II, IM>&,
      				bind_proxy&,
				unbind_success_proxy&,
				D&) = &system::bind;
      schedule (make_runnable (boost::bind (bind_ptr,
					    boost::ref (m_system),
					    make_action (output_automaton, output_member_ptr),
					    make_action (get_current_aid (ptr), input_member_ptr, input_parameter),
					    boost::ref (m_bind_proxy),
					    boost::ref (m_unbind_success_proxy),
					    boost::ref (d))));
    }

    template <class C, class OI, class OM, class II, class IM, class D>
    void unbind (const C* ptr,
		 const automaton_handle<OI>& output_automaton,
		 OM OI::*output_member_ptr,
		 const automaton_handle<II>& input_automaton,
		 IM II::*input_member_ptr,
		 D& d) {
      void (system::*unbind_ptr) (const action<OI, OM>&,
				  const action<II, IM>&,
				  const automaton_handle<C>&,
				  unbind_failure_proxy&,
				  D&) = &system::unbind;
      schedule (make_runnable (boost::bind (unbind_ptr,
					    boost::ref (m_system),
					    make_action (output_automaton, output_member_ptr),
					    make_action (input_automaton, input_member_ptr),
					    get_current_aid (ptr),
					    boost::ref (m_unbind_failure_proxy),
					    boost::ref (d))));
    }

    template <class OI, class OM, class OP, class II, class IM, class D>
    void unbind (const OI* ptr,
		 OM OI::*output_member_ptr,
		 const parameter_handle<OP>& output_parameter,
		 const automaton_handle<II>& input_automaton,
		 IM II::*input_member_ptr,
		 D& d) {
      void (system::*unbind_ptr) (const action<OI, OM>&,
				  const action<II, IM>&,
				  unbind_failure_proxy&,
				  D&) = &system::unbind;
      schedule (make_runnable (boost::bind (unbind_ptr,
					    boost::ref (m_system),
					    make_action (get_current_aid (ptr), output_member_ptr, output_parameter),
					    make_action (input_automaton, input_member_ptr),
					    boost::ref (m_unbind_failure_proxy),
					    boost::ref (d))));
    }
    
    template <class OI, class OM, class II, class IM, class IP, class D>
    void unbind (const II* ptr,
		 const automaton_handle<OI>& output_automaton,
		 OM OI::*output_member_ptr,
		 IM II::*input_member_ptr,
		 const parameter_handle<IP>& input_parameter,
		 D& d) {
      void (system::*unbind_ptr) (const action<OI, OM>&,
				  const action<II, IM>&,
				  unbind_failure_proxy&,
				  D&) = &system::unbind;
      schedule (make_runnable (boost::bind (unbind_ptr,
					    boost::ref (m_system),
					    make_action (output_automaton, output_member_ptr),
					    make_action (get_current_aid (ptr), input_member_ptr, input_parameter),
					    boost::ref (m_unbind_failure_proxy),
					    boost::ref (d))));
    }

    template <class C, class P, class D>
    void rescind (const C* ptr,
		  const parameter_handle<P>& parameter,
		  D& d) {
      void (system::*rescind_ptr) (const automaton_handle<C>&,
				   const parameter_handle<P>&,
				   rescind_failure_proxy&,
				   D&) = &system::rescind;
      schedule (make_runnable (boost::bind (rescind_ptr,
					    boost::ref (m_system),
					    get_current_aid (ptr),
					    parameter,
					    boost::ref (m_rescind_failure_proxy),
					    boost::ref (d))));
    }

    template <class C, class I, class D>
    void destroy (const C* ptr,
		  const automaton_handle<I>& automaton,
		  D& d) {
      void (system::*destroy_ptr) (const automaton_handle<C>&,
				   const automaton_handle<I>&,
				   destroy_failure_proxy&,
				   D&) = &system::destroy;
      schedule (make_runnable (boost::bind (destroy_ptr,
					    boost::ref (m_system),
					    get_current_aid (ptr),
					    automaton,
					    boost::ref (m_destroy_failure_proxy),
					    boost::ref (d))));
    }

    template <class I, class M>
    void schedule (const I* ptr,
		   M I::*member_ptr) {
      action<I, M> ac = make_action (get_current_aid (ptr), member_ptr);
      void (system::*execute_ptr) (const action<I,M>&,
      				   scheduler_interface&,
      				   execute_proxy&) = &system::execute;
      schedule (make_runnable (boost::bind (execute_ptr,
					    boost::ref (m_system),
					    ac,
					    boost::ref (*this),
					    boost::ref (m_execute_proxy))));
    }

    template <class G>
    void run (G generator) {
      BOOST_ASSERT (m_runq.size () == 0);
      BOOST_ASSERT (runnable_interface::count () == 0);
      m_running = true;
      m_system.create (generator, *this, m_create_proxy, m_destroy_success_proxy);
      while (keep_going ()) {
	runnable_interface* r = m_runq.pop ();
	(*r) ();
	delete r;
      }
      m_running = false;
    }

    void clear (void) {
      for (blocking_queue<runnable_interface*>::iterator pos = m_runq.begin ();
	   pos != m_runq.end ();
	   ++pos) {
	delete (*pos);
      }
      m_runq.clear ();
      m_system.clear ();
    }
            
  };

}

#endif
