#ifndef __simple_scheduler_hpp__
#define __simple_scheduler_hpp__

#include <ioa/scheduler.hpp>

#include <ioa/sys_create_runnable.hpp>
#include <ioa/sys_bind_runnable.hpp>
#include <ioa/sys_unbind_runnable.hpp>
#include <ioa/sys_destroy_runnable.hpp>

#include <ioa/create_runnable.hpp>
#include <ioa/bind_runnable.hpp>
#include <ioa/unbind_runnable.hpp>
#include <ioa/destroy_runnable.hpp>
#include <ioa/action_runnable.hpp>

#include <ioa/blocking_list.hpp>
#include <ioa/thread.hpp>
#include <ioa/thread_key.hpp>
#include <queue>
#include <algorithm>
#include <fcntl.h>
#include <sys/select.h>
#include <unistd.h>
#include <ioa/time.hpp>

namespace ioa {

  typedef std::pair<action_runnable_interface*, time> action_time;

  class simple_scheduler
  {
  private:

    static blocking_list<std::pair<bool, runnable_interface*> > m_sysq;
    static blocking_list<std::pair<bool, action_runnable_interface*> > m_execq;
    static int m_wakeup_fd[2];
    static blocking_list<action_time> m_timerq;
    static thread_key<aid_t> m_current_aid;

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

    static bool keep_going ();
    static bool thread_keep_going ();
    static void schedule_sysq (runnable_interface* r);
    static void process_sysq ();
    static void schedule_execq (action_runnable_interface* r);
    static void process_execq ();
    static void wakeup_timer_thread ();
    static void schedule_timerq (action_runnable_interface* r, const time& offset);
    static void process_timerq ();
    
  public:

    static void set_current_aid (const aid_t aid);
    static void clear_current_aid ();

    static void create (const aid_t automaton,
			shared_ptr<generator_interface> generator,
			void* const key);

    static void bind (const aid_t automaton,
		      shared_ptr<bind_executor_interface> generator,
		      void* const key);

    static void unbind (const aid_t automaton,
			void* const key);

    static void destroy (const aid_t automaton,
			 void* const key);

    static void create_key_exists (const aid_t automaton,
				   void* const key);

    static void instance_exists (const aid_t automaton,
				 void* const key);

    static void automaton_created (const aid_t automaton,
				   void* const key,
				   const aid_t child);

    static void bind_key_exists (const aid_t automaton,
				 void* const key);

    static void output_automaton_dne (const aid_t automaton,
				      void* const key);

    static void input_automaton_dne (const aid_t automaton,
				      void* const key);

    static void binding_exists (const aid_t automaton,
				void* const key);

    static void input_action_unavailable (const aid_t automaton,
					  void* const key);

    static void output_action_unavailable (const aid_t automaton,
					   void* const key);
    
    static void bound (const aid_t automaton,
		       void* const key);

    static void bind_key_dne (const aid_t automaton,
			      void* const key);

    static void unbound (const aid_t automaton,
			 void* const key);

    static void create_key_dne (const aid_t automaton,
				void* const key);

    static void automaton_destroyed (const aid_t automaton,
				     void* const key);
    
    static void recipient_dne (const aid_t automaton,
			       void* const key);

    static void event_delivered (const aid_t automaton,
				 void* const key);
    
    static aid_t get_current_aid ();

    // TODO:  Move these functions and includes to cpp file.

    static void schedule (automaton_interface::sys_create_type automaton_interface::*member_ptr) {
      schedule_execq (new sys_create_runnable (get_current_aid ()));
    }

    static void schedule (automaton_interface::sys_bind_type automaton_interface::*member_ptr) {
      schedule_execq (new sys_bind_runnable (get_current_aid ()));
    }

    static void schedule (automaton_interface::sys_unbind_type automaton_interface::*member_ptr) {
      schedule_execq (new sys_unbind_runnable (get_current_aid ()));
    }

    static void schedule (automaton_interface::sys_destroy_type automaton_interface::*member_ptr) {
      schedule_execq (new sys_destroy_runnable (get_current_aid ()));
    }
  
    template <class I, class M>
    static void schedule (M I::*member_ptr) {
      schedule_execq (make_action_runnable (make_action (automaton_handle<I> (get_current_aid ()), member_ptr)));
    }


    template <class I, class M>
    static void schedule (M I::*member_ptr,
  			  time offset) {
      schedule_timerq (make_action_runnable (make_action (automaton_handle<I> (get_current_aid ()), member_ptr)), offset);
    }
    
    static void run (shared_ptr<generator_interface> generator);
    static void clear (void);
  };

  // Implement the scheduler.
  
  template <class I, class M>
  void scheduler::schedule (M I::*member_ptr) {
    simple_scheduler::schedule (member_ptr);
  }
  
  template <class I, class M>
  void scheduler::schedule (M I::*member_ptr,
			    time offset) {
    simple_scheduler::schedule (member_ptr, offset);
  }
  
  // TODO:  EVENTS!!!
  // TODO:  What happens when we send an event to a destroyed automaton?
}

#endif
