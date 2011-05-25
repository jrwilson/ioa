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
    system m_system;
    blocking_queue<runnable_interface*> m_runq;
    bool m_ignore_schedule;
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
      if (!m_ignore_schedule) {
	m_runq.push (r);
      }
      else {
	delete r;
      }
    }

  public:
    simple_scheduler () :
      m_current_aid (-1),
      m_current_this (0)
    { }

    ~simple_scheduler () {
      clear ();
    }

    template <class C, class G, class D>
    void create (const C* ptr,
		 G generator,
		 D& d) {
      automaton_handle<typename G::result_type> (system::*create_ptr) (const automaton_handle<C>&,
								       G,
								       scheduler_interface&,
								       D&) = &system::create;
      schedule (make_runnable (boost::bind (create_ptr,
					    boost::ref (m_system),
					    get_current_aid (ptr),
					    generator,  // We want a copy, not a reference.
					    boost::ref (*this),
					    boost::ref (d))));
    }
    
    template <class C, class P, class D>
    void declare (const C* ptr,
		  P* parameter,
		  D& d) {
      parameter_handle<P> (system::*declare_ptr) (const automaton_handle<C>&,
						  P*,
						  scheduler_interface&,
						  D&) = &system::declare;
      schedule (make_runnable (boost::bind (declare_ptr,
					    boost::ref (m_system),
					    get_current_aid (ptr),
					    parameter,
					    boost::ref (*this),
					    boost::ref (d))));
    }

    template <class C, class OI, class OM, class II, class IM, class D>
    void bind (const C* ptr,
	       const automaton_handle<OI>& output_automaton,
	       OM OI::*output_member_ptr,
	       const automaton_handle<II>& input_automaton,
	       IM II::*input_member_ptr,
	       D& d) {
      bool (system::*bind_ptr) (const action<OI, OM>&,
      				const action<II, IM>&,
      				const automaton_handle<C>&,
				scheduler_interface&,
				D&) = &system::bind;
      schedule (make_runnable (boost::bind (bind_ptr,
					    boost::ref (m_system),
					    make_action (output_automaton, output_member_ptr),
					    make_action (input_automaton, input_member_ptr),
					    get_current_aid (ptr),
					    boost::ref (*this),
					    boost::ref (d))));
    }

    template <class OI, class OM, class OP, class II, class IM, class D>
    void bind (const OI* ptr,
	       OM OI::*output_member_ptr,
	       const parameter_handle<OP>& output_parameter,
	       const automaton_handle<II>& input_automaton,
	       IM II::*input_member_ptr,
	       D& d) {
      bool (system::*bind_ptr) (const action<OI, OM>&,
      				const action<II, IM>&,
				scheduler_interface&,
				D&) = &system::bind;
      schedule (make_runnable (boost::bind (bind_ptr,
					    boost::ref (m_system),
					    make_action (get_current_aid (ptr), output_member_ptr, output_parameter),
					    make_action (input_automaton, input_member_ptr),
					    boost::ref (*this),
					    boost::ref (d))));
    }
    
    template <class OI, class OM, class II, class IM, class IP, class D>
    void bind (const II* ptr,
	       const automaton_handle<OI>& output_automaton,
	       OM OI::*output_member_ptr,
	       IM II::*input_member_ptr,
	       const parameter_handle<IP>& input_parameter,
	       D& d) {
      bool (system::*bind_ptr) (const action<OI, OM>&,
      				const action<II, IM>&,
				scheduler_interface&,
				D&) = &system::bind;
      schedule (make_runnable (boost::bind (bind_ptr,
					    boost::ref (m_system),
					    make_action (output_automaton, output_member_ptr),
					    make_action (get_current_aid (ptr), input_member_ptr, input_parameter),
					    boost::ref (*this),
					    boost::ref (d))));
    }

    template <class C, class OI, class OM, class II, class IM, class D>
    void unbind (const C* ptr,
		 const automaton_handle<OI>& output_automaton,
		 OM OI::*output_member_ptr,
		 const automaton_handle<II>& input_automaton,
		 IM II::*input_member_ptr,
		 D& d) {
      bool (system::*unbind_ptr) (const action<OI, OM>&,
				  const action<II, IM>&,
				  const automaton_handle<C>&,
				  scheduler_interface&,
				  D&) = &system::unbind;
      schedule (make_runnable (boost::bind (unbind_ptr,
					    boost::ref (m_system),
					    make_action (output_automaton, output_member_ptr),
					    make_action (input_automaton, input_member_ptr),
					    get_current_aid (ptr),
					    boost::ref (*this),
					    boost::ref (d))));
    }

    template <class OI, class OM, class OP, class II, class IM, class D>
    void unbind (const OI* ptr,
		 OM OI::*output_member_ptr,
		 const parameter_handle<OP>& output_parameter,
		 const automaton_handle<II>& input_automaton,
		 IM II::*input_member_ptr,
		 D& d) {
      bool (system::*unbind_ptr) (const action<OI, OM>&,
				  const action<II, IM>&,
				  scheduler_interface&,
				  D&) = &system::unbind;
      schedule (make_runnable (boost::bind (unbind_ptr,
					    boost::ref (m_system),
					    make_action (get_current_aid (ptr), output_member_ptr, output_parameter),
					    make_action (input_automaton, input_member_ptr),
					    boost::ref (*this),
					    boost::ref (d))));
    }
    
    template <class OI, class OM, class II, class IM, class IP, class D>
    void unbind (const II* ptr,
		 const automaton_handle<OI>& output_automaton,
		 OM OI::*output_member_ptr,
		 IM II::*input_member_ptr,
		 const parameter_handle<IP>& input_parameter,
		 D& d) {
      bool (system::*unbind_ptr) (const action<OI, OM>&,
				  const action<II, IM>&,
				  scheduler_interface&,
				  D&) = &system::unbind;
      schedule (make_runnable (boost::bind (unbind_ptr,
					    boost::ref (m_system),
					    make_action (output_automaton, output_member_ptr),
					    make_action (get_current_aid (ptr), input_member_ptr, input_parameter),
					    boost::ref (*this),
					    boost::ref (d))));
    }

    template <class C, class P, class D>
    void rescind (const C* ptr,
		  const parameter_handle<P>& parameter,
		  D& d) {
      bool (system::*rescind_ptr) (const automaton_handle<C>&,
				   const parameter_handle<P>&,
				   scheduler_interface&,
				   D&) = &system::rescind;
      schedule (make_runnable (boost::bind (rescind_ptr,
					    boost::ref (m_system),
					    get_current_aid (ptr),
					    parameter,
					    boost::ref (*this),
					    boost::ref (d))));
    }

    template <class C, class I, class D>
    void destroy (const C* ptr,
		  const automaton_handle<I>& automaton,
		  D& d) {
      bool (system::*destroy_ptr) (const automaton_handle<C>&,
				   const automaton_handle<I>&,
				   scheduler_interface&,
				   D&) = &system::destroy;
      schedule (make_runnable (boost::bind (destroy_ptr,
					    boost::ref (m_system),
					    get_current_aid (ptr),
					    automaton,
					    boost::ref (*this),
					    boost::ref (d))));
    }

    template <class I, class M>
    void schedule (const I* ptr,
		   M I::*member_ptr) {
      action<I, M> ac = make_action (get_current_aid (ptr), member_ptr);
      bool (system::*execute_ptr) (const action<I,M>&,
      				   scheduler_interface&) = &system::execute;
      schedule (make_runnable (boost::bind (execute_ptr,
					    boost::ref (m_system),
					    ac,
					    boost::ref (*this))));
    }

    template <class G>
    void run (G generator) {
      BOOST_ASSERT (m_runq.size () == 0);
      BOOST_ASSERT (runnable_interface::count () == 0);
      m_system.create (generator, *this);
      while (keep_going ()) {
	runnable_interface* r = m_runq.pop ();
	(*r) ();
	delete r;
      }
    }

    void clear (void) {
      for (blocking_queue<runnable_interface*>::iterator pos = m_runq.begin ();
	   pos != m_runq.end ();
	   ++pos) {
	delete (*pos);
      }
      m_runq.clear ();
      m_ignore_schedule = true;
      m_system.clear ();
      m_ignore_schedule = false;
    }
            
  };

}

#endif
