#ifndef __action_runnable_hpp__
#define __action_runnable_hpp__

#include <ioa/action_executor.hpp>
#include <ioa/shared_mutex.hpp>
#include <ioa/automaton_set.hpp>
#include <ioa/binding_set.hpp>

#include <iostream>

namespace ioa {

  class action_runnable_interface
  {
  public:
    virtual ~action_runnable_interface () { }
    
    virtual const action_executor_interface& get_action () const = 0;

    virtual void operator() () = 0;
    
    bool operator== (const action_runnable_interface& x) const {
      return get_action () == x.get_action ();
    }
    
    bool operator< (const action_runnable_interface& x) const {
      return get_action () < x.get_action ();
    }
  };

  template <class I, class M, class K, class L>
  class action_runnable_impl;

  template <class I, class M, class L>
  class action_runnable_impl<I, M, output_category, L> :
    public action_runnable_interface
  {
  private:
    scheduler_interface& m_scheduler;
    shared_mutex& m_shared_mutex;
    automaton_set& m_automaton_set;
    binding_set& m_binding_set;
    action_executor<I, M> m_exec;
    
  public:
    action_runnable_impl (scheduler_interface& scheduler,
			  shared_mutex& shared_mutex,
			  automaton_set& automaton_set,
			  binding_set& binding_set,
			  I& instance,
			  mutex& mutex,
			  const automaton_handle<I>& handle,
			  M I::*member_ptr) :
      m_scheduler (scheduler),
      m_shared_mutex (shared_mutex),
      m_automaton_set (automaton_set),
      m_binding_set (binding_set),
      m_exec (instance, mutex, handle, member_ptr)
    { }

    action_runnable_impl (scheduler_interface& scheduler,
			  shared_mutex& shared_mutex,
			  automaton_set& automaton_set,
			  binding_set& binding_set,
			  I& instance,
			  mutex& mutex,
			  const automaton_handle<I>& handle,
			  M I::*member_ptr,
			  const typename M::parameter_type& parameter) :
      m_scheduler (scheduler),
      m_shared_mutex (shared_mutex),
      m_automaton_set (automaton_set),
      m_binding_set (binding_set),
      m_exec (instance, mutex, handle, member_ptr, parameter)
    { }
    
    void operator() () {
      L lock (m_shared_mutex);
      if (m_automaton_set.exists (m_exec.get_aid ())) {
	m_exec (m_scheduler, m_binding_set.begin (m_exec), m_binding_set.end (m_exec));
      }
    }
    
    const action_executor_interface& get_action () const {
      return m_exec;
    }
  };

  template <class I, class M, class L>
  class action_runnable_impl<I, M, internal_category, L> :
    public action_runnable_interface
  {
  private:
    scheduler_interface& m_scheduler;
    shared_mutex& m_shared_mutex;
    automaton_set& m_automaton_set;
    action_executor<I, M> m_exec;
    
  public:
    action_runnable_impl (scheduler_interface& scheduler,
			  shared_mutex& shared_mutex,
			  automaton_set& automaton_set,
			  I& instance,
			  mutex& mutex,
			  const automaton_handle<I>& handle,
			  M I::*member_ptr) :
      m_scheduler (scheduler),
      m_shared_mutex (shared_mutex),
      m_automaton_set (automaton_set),
      m_exec (instance, mutex, handle, member_ptr)
    { }

    action_runnable_impl (scheduler_interface& scheduler,
			  shared_mutex& shared_mutex,
			  automaton_set& automaton_set,
			  I& instance,
			  mutex& mutex,
			  const automaton_handle<I>& handle,
			  M I::*member_ptr,
			  const typename M::parameter_type& parameter) :
      m_scheduler (scheduler),
      m_shared_mutex (shared_mutex),
      m_automaton_set (automaton_set),
      m_exec (instance, mutex, handle, member_ptr, parameter)
    { }
    
    void operator() () {
      L lock (m_shared_mutex);
      if (m_automaton_set.exists (m_exec.get_aid ())) {
	m_exec (m_scheduler);
      }
    }
    
    const action_executor_interface& get_action () const {
      return m_exec;
    }
  };

  template <class I, class M, class L>
  class action_runnable :
    public action_runnable_impl<I, M, typename M::action_category, L>
  {
  public:
    action_runnable (scheduler_interface& scheduler,
		     shared_mutex& shared_mutex,
		     automaton_set& automaton_set,
		     binding_set& binding_set,
		     I& instance,
		     mutex& mutex,
		     const automaton_handle<I>& handle,
		     M I::*member_ptr) :
      action_runnable_impl<I, M, typename M::action_category, L> (scheduler, shared_mutex, automaton_set, binding_set, instance, mutex, handle, member_ptr)
    { }

    action_runnable (scheduler_interface& scheduler,
		     shared_mutex& shared_mutex,
		     automaton_set& automaton_set,
		     I& instance,
		     mutex& mutex,
		     const automaton_handle<I>& handle,
		     M I::*member_ptr) :
      action_runnable_impl<I, M, typename M::action_category, L> (scheduler, shared_mutex, automaton_set, instance, mutex, handle, member_ptr)
    { }

    action_runnable (scheduler_interface& scheduler,
		     shared_mutex& shared_mutex,
		     automaton_set& automaton_set,
		     binding_set& binding_set,
		     I& instance,
		     mutex& mutex,
		     const automaton_handle<I>& handle,
		     M I::*member_ptr,
		     const typename M::parameter_type& parameter) :
      action_runnable_impl<I, M, typename M::action_category, L> (scheduler, shared_mutex, automaton_set, binding_set, instance, mutex, handle, member_ptr, parameter)
    { }

    action_runnable (scheduler_interface& scheduler,
		     shared_mutex& shared_mutex,
		     automaton_set& automaton_set,
		     I& instance,
		     mutex& mutex,
		     const automaton_handle<I>& handle,
		     M I::*member_ptr,
		     const typename M::parameter_type& parameter) :
      action_runnable_impl<I, M, typename M::action_category, L> (scheduler, shared_mutex, automaton_set, instance, mutex, handle, member_ptr, parameter)
    { }
  };
  
  // template <class I, class M>
  // action_runnable<I, M>* make_action_runnable (const automaton_handle<I>& handle,
  // 					       M I::*member_ptr) {
  //   return new action_runnable<I, M> (handle, member_ptr);
  // }

  // template <class I, class M>
  // action_runnable<I, M>* make_action_runnable (const automaton_handle<I>& handle,
  // 					       M I::*member_ptr,
  // 					       const typename M::parameter_type& parameter) {
  //   return new action_runnable<I, M> (handle, member_ptr, parameter);
  // }

  // template <class I, class M>
  // action_runnable<I, M>* make_action_runnable (const automaton_handle<I>& handle,
  // 					       M I::*member_ptr,
  // 					       const typename M::value_type& value,
  // 					       typename M::action_category category) {
  //   return new action_runnable<I, M> (handle, member_ptr, value, category);
  // }
  
}

#endif
