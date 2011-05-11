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

  template <class T>
  class system_event :
    public runnable_interface
  {
  private:
    internal_scheduler_interface& m_scheduler;
    system& m_system;
    generic_automaton_handle m_automaton;
    runnable<T> m_t;
    
  public:
    system_event (internal_scheduler_interface& scheduler,
		  system& system,
  		  const generic_automaton_handle& automaton,
  		  const T& t) :
      m_scheduler (scheduler),
      m_system (system),
      m_automaton (automaton),
      m_t (t)
    { }
    
    void operator () () {
      m_system.execute (m_automaton, m_t, m_scheduler, m_scheduler);      
    }
  };

  template <class T>
  system_event<T>* make_system_event (internal_scheduler_interface& scheduler,
				      system& system,
  				      const generic_automaton_handle& automaton,
  				      const T& t) {
    return new system_event<T> (scheduler, system, automaton, t);
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

    template <class C>
    void create (const C* ptr,
		 automaton_interface* instance) {
      void (system::*create_ptr) (const generic_automaton_handle&,
				  automaton_interface*,
				  create_listener_interface&) = &system::create;
      m_scheduler.schedule (make_runnable (boost::bind (create_ptr,
							boost::ref (m_scheduler.get_system ()),
							m_scheduler.get_current_handle (ptr),
							instance,
							boost::ref (m_scheduler))));
    }
    
    template <class C>
    void declare (const C* ptr,
		  void* parameter) {
      m_scheduler.schedule (make_runnable (boost::bind (&system::declare<declare_listener_interface>,
							boost::ref (m_scheduler.get_system ()),
							m_scheduler.get_current_handle (ptr),
							parameter,
							boost::ref (m_scheduler))));
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
				bind_listener_interface&) = &system::bind<OM, IM>;
      m_scheduler.schedule (make_runnable (boost::bind (bind_ptr,
							boost::ref (m_scheduler.get_system ()),
							make_action (output_automaton, output_member_ptr),
							make_action (input_automaton, input_member_ptr),
							m_scheduler.get_current_handle (ptr),
							boost::ref (m_scheduler))));
    }

    template <class OI, class OM, class OP, class II, class IM>
    void bind (const OI* ptr,
	       OM OI::*output_member_ptr,
	       const parameter_handle<OP>& output_parameter,
	       const automaton_handle<II>& input_automaton,
	       IM II::*input_member_ptr) {
      void (system::*bind_ptr) (const action<OM>&,
				const action<IM>&,
				bind_listener_interface&) = &system::bind<OM, IM>;
      m_scheduler.schedule (make_runnable (boost::bind (bind_ptr,
							boost::ref (m_scheduler.get_system ()),
							make_action (m_scheduler.get_current_handle (ptr), output_member_ptr, output_parameter),
							make_action (input_automaton, input_member_ptr),
							boost::ref (m_scheduler))));
    }
    
    template <class OI, class OM, class II, class IM, class IP>
    void bind (const II* ptr,
	       const automaton_handle<OI>& output_automaton,
	       OM OI::*output_member_ptr,
	       IM II::*input_member_ptr,
	       const parameter_handle<IP>& input_parameter) {
      void (system::*bind_ptr) (const action<OM>&,
				const action<IM>&,
				bind_listener_interface&) = &system::bind<OM, IM>;
      m_scheduler.schedule (make_runnable (boost::bind (bind_ptr,
							boost::ref (m_scheduler.get_system ()),
							make_action (output_automaton, output_member_ptr),
							make_action (m_scheduler.get_current_handle (ptr), input_member_ptr, input_parameter),
							boost::ref (m_scheduler))));
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
				unbind_listener_interface&) = &system::unbind<OM, IM>;
      m_scheduler.schedule (make_runnable (boost::bind (unbind_ptr,
							boost::ref (m_scheduler.get_system ()),
							make_action (output_automaton, output_member_ptr),
							make_action (input_automaton, input_member_ptr),
							m_scheduler.get_current_handle (ptr),
							boost::ref (m_scheduler))));
    }

    template <class OI, class OM, class OP, class II, class IM>
    void unbind (const OI* ptr,
	       OM OI::*output_member_ptr,
	       const parameter_handle<OP>& output_parameter,
	       const automaton_handle<II>& input_automaton,
	       IM II::*input_member_ptr) {
      void (system::*unbind_ptr) (const action<OM>&,
				const action<IM>&,
				unbind_listener_interface&) = &system::unbind<OM, IM>;
      m_scheduler.schedule (make_runnable (boost::bind (unbind_ptr,
							boost::ref (m_scheduler.get_system ()),
							make_action (m_scheduler.get_current_handle (ptr), output_member_ptr, output_parameter),
							make_action (input_automaton, input_member_ptr),
							boost::ref (m_scheduler))));
    }
    
    template <class OI, class OM, class II, class IM, class IP>
    void unbind (const II* ptr,
	       const automaton_handle<OI>& output_automaton,
	       OM OI::*output_member_ptr,
	       IM II::*input_member_ptr,
	       const parameter_handle<IP>& input_parameter) {
      void (system::*unbind_ptr) (const action<OM>&,
				const action<IM>&,
				unbind_listener_interface&) = &system::unbind<OM, IM>;
      m_scheduler.schedule (make_runnable (boost::bind (unbind_ptr,
							boost::ref (m_scheduler.get_system ()),
							make_action (output_automaton, output_member_ptr),
							make_action (m_scheduler.get_current_handle (ptr), input_member_ptr, input_parameter),
							boost::ref (m_scheduler))));
    }

    template <class C>
    void rescind (const C* ptr,
		  const generic_parameter_handle& parameter) {
      m_scheduler.schedule (make_runnable (boost::bind (&system::rescind<rescind_listener_interface>,
      							boost::ref (m_scheduler.get_system ()),
      							m_scheduler.get_current_handle (ptr),
      							parameter,
      							boost::ref (m_scheduler))));
    }

    template <class C>
    void destroy (const C* ptr,
		  const generic_automaton_handle& automaton) {
      m_scheduler.schedule (make_runnable (boost::bind (&system::destroy<destroy_listener_interface>,
      							boost::ref (m_scheduler.get_system ()),
      							m_scheduler.get_current_handle (ptr),
      							automaton,
      							boost::ref (m_scheduler))));
    }

  private:
    template <class M>
    void schedule (const action<M>& ac,
  		   output_category /* */) {
      void (system::*execute_ptr) (const output_action_interface&,
				   scheduler_interface&,
				   execute_listener_interface&) = &system::execute;
      m_scheduler.schedule (make_runnable (boost::bind (execute_ptr,
							boost::ref (m_scheduler.get_system ()),
							ac,
							boost::ref (m_scheduler),
							boost::ref (m_scheduler))));
    }
    
    template <class M>
    void schedule (const action<M>& ac,
  		   internal_category /* */) {
      void (system::*execute_ptr) (const independent_action_interface&,
				   scheduler_interface&,
				   execute_listener_interface&) = &system::execute;
      m_scheduler.schedule (make_runnable (boost::bind (execute_ptr,
							boost::ref (m_scheduler.get_system ()),
							ac,
							boost::ref (m_scheduler),
							boost::ref (m_scheduler))));
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

}

#endif
