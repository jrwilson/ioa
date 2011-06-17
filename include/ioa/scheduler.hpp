#ifndef __scheduler_hpp__
#define __scheduler_hpp__

#include <ioa/time.hpp>
#include <ioa/shared_ptr.hpp>
#include <ioa/action.hpp>
#include <ioa/generator_interface.hpp>
#include <ioa/scheduler_interface.hpp>
#include <ioa/action_runnable.hpp>
#include <ioa/automaton_interface.hpp>

namespace ioa {

  extern scheduler_interface* scheduler;

  aid_t get_current_aid ();

  void schedule (automaton_interface::sys_create_type automaton_interface::*ptr);
  void schedule (automaton_interface::sys_bind_type automaton_interface::*ptr);
  void schedule (automaton_interface::sys_unbind_type automaton_interface::*ptr);
  void schedule (automaton_interface::sys_destroy_type automaton_interface::*ptr);
  
  template <class I, class M>
  void schedule (M I::*member_ptr) {
    assert (scheduler != 0);
    scheduler->schedule (make_action_runnable (automaton_handle<I> (get_current_aid ()), member_ptr));
  }
  
  template <class I, class M>
  void schedule (M I::*member_ptr,
		 const typename M::parameter_type& param) {
    assert (scheduler != 0);
    scheduler->schedule (make_action_runnable (automaton_handle<I> (get_current_aid ()), member_ptr, param));
  }
  
  template <class I, class M>
  void schedule_after (M I::*member_ptr,
		       const time& offset) {
    assert (scheduler != 0);
    scheduler->schedule_after (make_action_runnable (automaton_handle<I> (get_current_aid ()), member_ptr), offset);
  }
  
  template <class I, class M>
  void schedule_after (M I::*member_ptr,
		       const typename M::parameter_type& param,
		       const time& offset) {
    assert (scheduler != 0);
    scheduler->schedule_after (make_action_runnable (automaton_handle<I> (get_current_aid ()), member_ptr, param), offset);
  }
  
  template <class I, class M>
  void schedule_read_ready (M I::*member_ptr,
			    int fd) {
    assert (scheduler != 0);
    scheduler->schedule_read_ready (make_action_runnable (automaton_handle<I> (get_current_aid ()), member_ptr), fd);
  }
  
  template <class I, class M>
  void schedule_read_ready (M I::*member_ptr,
			    const typename M::parameter_type& param,
			    int fd) {
    assert (scheduler != 0);
    scheduler->schedule_read_ready (make_action_runnable (automaton_handle<I> (get_current_aid ()), member_ptr, param), fd);
  }
  
  template <class I, class M>
  void schedule_write_ready (M I::*member_ptr,
			     int fd) {
    assert (scheduler != 0);
    scheduler->schedule_write_ready (make_action_runnable (automaton_handle<I> (get_current_aid ()), member_ptr), fd);
  }
  
  template <class I, class M>
  void schedule_write_ready (M I::*member_ptr,
			     const typename M::parameter_type& param,
			     int fd) {
    assert (scheduler != 0);
    scheduler->schedule_write_ready (make_action_runnable (make_action (automaton_handle<I> (get_current_aid ()), member_ptr, param)), fd);
  }
  
  template <class I, class M>
  size_t bind_count (M I::*member_ptr) {
    assert (scheduler != 0);
    return scheduler->bind_count (make_action (automaton_handle<I> (get_current_aid ()), member_ptr));
  }
  
  template <class I, class M>
  size_t bind_count (M I::*member_ptr,
		     const typename M::parameter_type& param) {
    assert (scheduler != 0);
    return scheduler->bind_count (make_action (automaton_handle<I> (get_current_aid ()), member_ptr, param));
  }
  
  void run (scheduler_interface& s,
	    shared_ptr<generator_interface> generator);
  
}

#endif
