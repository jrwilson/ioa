#ifndef __simple_scheduler_hpp__
#define __simple_scheduler_hpp__

#include "blocking_queue.hpp"
#include "runnable.hpp"
#include "system.hpp"
#include <boost/bind.hpp>
#include <boost/thread.hpp>

namespace ioa {

  // TODO:  DUPLICATE ACTIONS!!!
  // TODO:  EVENTS!!!
  // TODO:  What happens when we send an event to a destroyed automaton?

  class simple_scheduler :
    public scheduler_interface
  {
  private:
    system m_system;
    blocking_list<std::pair<bool, runnable_interface*> > m_sysq;
    blocking_list<std::pair<bool, action_runnable_interface*> > m_execq;
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

    bool keep_going () const {
      // The criteria for continuing is simple: a runnable exists.
      return runnable_interface::count () != 0;
    }

    bool thread_keep_going () {
      bool retval = keep_going ();
      if (!retval) {
	/*
	  The run queue processing threads have the following structure.
	  while (keep_going ()) {
            runnable* r = runq.pop ();
	    // Do something with runnable and delete it.
	  }
	  
	  It is possible that keep_going () was true to enable the loop but becomes false immediatley before calling runq.pop ().
	  Since there are no runnables and no runnables will be produced, the thread will block forever.
	  Consequenty, we push a sentinel value to unblock the processing threads.
	*/
	m_sysq.push (std::pair<bool, runnable_interface*> (false, 0));
	m_execq.push (std::pair<bool, action_runnable_interface*> (false, 0));
      }
      return retval;
    }

    void schedule_sysq (runnable_interface* r) {
      m_sysq.push (std::make_pair (true, r));
    }

    struct action_runnable_equal
    {
      const action_runnable_interface* m_ptr;
      
      action_runnable_equal (const action_runnable_interface* ptr) :
	m_ptr (ptr)
      { }

      bool operator() (const std::pair<bool, action_runnable_interface*>& x) const {
	if (x.first) {
	  return (*m_ptr) == (*x.second);
	}
	else {
	  return false;
	}
      }
    };

    void schedule_execq (action_runnable_interface* r) {
      /*
	Imagine an automaton with with an internal action that does nothing but schedule itself twice.
	Let (n) denote the number of runnables in the execq for the action.
	We start with (1).
	The action is removed and executed resulting in (2).
	One copy is removed and executed result in (3).
	...
	After n rounds the execq contains n copies.
	This is bad.
	We need to remove duplicates.
       */
      bool duplicate;
      {
	boost::shared_lock<boost::shared_mutex> lock (m_execq.mutex);
	duplicate = std::find_if (m_execq.list.begin (), m_execq.list.end (), action_runnable_equal (r)) != m_execq.list.end ();
      }

      /*
	One might ask, "Can't someone sneak in a duplicate action_runnable between unlocking m_execq.mutex and executing m_execq.push ()?"
	The answer is no and here's why.
	Automata can only schedule their own actions.
	In order to insert a duplicate, an automaton would have to call schedule () concurrently.
	We explicitly prevent this to adhere to the I/O automata model.
       */

      if (!duplicate) {
	m_execq.push (std::make_pair (true, r));
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
      schedule_sysq (make_runnable (boost::bind (create_ptr,
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
      schedule_sysq (make_runnable (boost::bind (declare_ptr,
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
      bid_t (system::*bind_ptr) (const action<OI, OM>&,
				 const action<II, IM>&,
				 const automaton_handle<C>&,
				 scheduler_interface&,
				 D&) = &system::bind;
      schedule_sysq (make_runnable (boost::bind (bind_ptr,
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
      bid_t (system::*bind_ptr) (const action<OI, OM>&,
				 const action<II, IM>&,
				 scheduler_interface&,
				 D&) = &system::bind;
      schedule_sysq (make_runnable (boost::bind (bind_ptr,
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
      bid_t (system::*bind_ptr) (const action<OI, OM>&,
				 const action<II, IM>&,
				 scheduler_interface&,
				 D&) = &system::bind;
      schedule_sysq (make_runnable (boost::bind (bind_ptr,
						 boost::ref (m_system),
						 make_action (output_automaton, output_member_ptr),
						 make_action (get_current_aid (ptr), input_member_ptr, input_parameter),
						 boost::ref (*this),
						 boost::ref (d))));
    }

    template <class C, class D>
    void unbind (const C* ptr,
		 const bid_t bid,
		 D& d) {
      bool (system::*unbind_ptr) (const bid_t,
				  const automaton_handle<C>&,
				  scheduler_interface&,
				  D&) = &system::unbind;
      schedule_sysq (make_runnable (boost::bind (unbind_ptr,
						 boost::ref (m_system),
						 bid,
						 get_current_aid (ptr),
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
      schedule_sysq (make_runnable (boost::bind (rescind_ptr,
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
      schedule_sysq (make_runnable (boost::bind (destroy_ptr,
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
      schedule_execq (make_action_runnable (boost::bind (execute_ptr,
							 boost::ref (m_system),
							 ac,
							 boost::ref (*this)),
					    ac));
    }

  private:

    void process_sysq () {
      while (thread_keep_going ()) {
	std::pair<bool, runnable_interface*> r = m_sysq.pop ();
	if (r.first) {
	  (*r.second) ();
	  delete r.second;
	}
      }
    }
    
    void process_execq () {
      while (thread_keep_going ()) {
	std::pair<bool, runnable_interface*> r = m_execq.pop ();
	if (r.first) {
	  (*r.second) ();
	  delete r.second;
	}
      }
    }

  public:
    template <class G>
    void run (G generator) {
      BOOST_ASSERT (m_sysq.list.size () == 0);
      BOOST_ASSERT (m_execq.list.size () == 0);
      BOOST_ASSERT (!keep_going ());
      m_system.create (generator, *this);

      boost::thread sysq_thread (&simple_scheduler::process_sysq, boost::ref (*this));
      boost::thread execq_thread (&simple_scheduler::process_execq, boost::ref (*this));
      sysq_thread.join ();
      execq_thread.join ();
    }

    void clear (void) {
      // We clear the system first because it might add something to a run queue.
      m_system.clear ();

      // Then, we clear the run queues.
      for (std::list<std::pair<bool, runnable_interface*> >::iterator pos = m_sysq.list.begin ();
	   pos != m_sysq.list.end ();
	   ++pos) {
	delete pos->second;
      }
      m_sysq.list.clear ();

      for (std::list<std::pair<bool, action_runnable_interface*> >::iterator pos = m_execq.list.begin ();
	   pos != m_execq.list.end ();
	   ++pos) {
	delete pos->second;
      }
      m_execq.list.clear ();

      // Notice that the post-conditions of clear () match those of run ().
      BOOST_ASSERT (m_sysq.list.size () == 0);
      BOOST_ASSERT (m_execq.list.size () == 0);
      BOOST_ASSERT (!keep_going ());
    }
            
  };

}

#endif
