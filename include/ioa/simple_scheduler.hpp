#ifndef __simple_scheduler_hpp__
#define __simple_scheduler_hpp__

#include <ioa/scheduler.hpp>
#include <ioa/action_runnable.hpp>
#include <ioa/blocking_list.hpp>
#include <ioa/thread_key.hpp>
#include <ioa/time.hpp>

namespace ioa {

  typedef std::pair<action_runnable_interface*, time> action_time;
  typedef std::pair<action_runnable_interface*, int> action_fd;

  class simple_scheduler
  {
  private:

    static blocking_list<std::pair<bool, runnable_interface*> > m_sysq;
    static blocking_list<std::pair<bool, action_runnable_interface*> > m_execq;
    static int m_wakeup_fd[2];
    static blocking_list<action_time> m_timerq;
    static blocking_list<action_fd> m_readq;
    static blocking_list<action_fd> m_writeq;
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
    static void wakeup_io_thread ();
    static void schedule_timerq (action_runnable_interface* r, const time& offset);
    static void schedule_readq (action_runnable_interface* r, int);
    static void schedule_writeq (action_runnable_interface* r, int);
    static void process_ioq ();
    
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

    static void output_bound (const output_executor_interface&);

    static void input_bound (const input_executor_interface&);

    static void bind_key_dne (const aid_t automaton,
			      void* const key);

    static void unbound (const aid_t automaton,
			 void* const key);

    static void output_unbound (const output_executor_interface&);

    static void input_unbound (const input_executor_interface&);

    static void create_key_dne (const aid_t automaton,
				void* const key);

    static void automaton_destroyed (const aid_t automaton,
				     void* const key);
    
    static void recipient_dne (const aid_t automaton,
			       void* const key);

    static void event_delivered (const aid_t automaton,
				 void* const key);
    
    static aid_t get_current_aid ();

    static void schedule (automaton_interface::sys_create_type automaton_interface::*member_ptr);

    static void schedule (automaton_interface::sys_bind_type automaton_interface::*member_ptr);

    static void schedule (automaton_interface::sys_unbind_type automaton_interface::*member_ptr);

    static void schedule (automaton_interface::sys_destroy_type automaton_interface::*member_ptr);
  
    template <class I, class M>
    static void schedule (M I::*member_ptr) {
      schedule_execq (make_action_runnable (make_action (automaton_handle<I> (get_current_aid ()), member_ptr)));
    }

    template <class I, class M>
    static void schedule (M I::*member_ptr,
                          const typename M::parameter_type& param) {
      schedule_execq (make_action_runnable (make_action (automaton_handle<I> (get_current_aid ()), member_ptr, param)));
    }

    template <class I, class M>
    static void schedule_after (M I::*member_ptr,
				time offset) {
      schedule_timerq (make_action_runnable (make_action (automaton_handle<I> (get_current_aid ()), member_ptr)), offset);
    }

    template <class I, class M>
    static void schedule_after (M I::*member_ptr,
				const typename M::parameter_type& param,
				time offset) {
      schedule_timerq (make_action_runnable (make_action (automaton_handle<I> (get_current_aid ()), member_ptr, param)), offset);
    }

    template <class I, class M>
    static void schedule_read_ready (M I::*member_ptr,
				     int fd) {
      schedule_readq (make_action_runnable (make_action (automaton_handle<I> (get_current_aid ()), member_ptr)), fd);
    }

    template <class I, class M>
    static void schedule_read_ready (M I::*member_ptr,
				     const typename M::parameter_type& param,
				     int fd) {
      schedule_readq (make_action_runnable (make_action (automaton_handle<I> (get_current_aid ()), member_ptr, param)), fd);
    }

    template <class I, class M>
    static void schedule_write_ready (M I::*member_ptr,
				     int fd) {
      schedule_writeq (make_action_runnable (make_action (automaton_handle<I> (get_current_aid ()), member_ptr)), fd);
    }

    template <class I, class M>
    static void schedule_write_ready (M I::*member_ptr,
				     const typename M::parameter_type& param,
				     int fd) {
      schedule_writeq (make_action_runnable (make_action (automaton_handle<I> (get_current_aid ()), member_ptr, param)), fd);
    }
    
    static void run (shared_ptr<generator_interface> generator);
  };

  // Implement the scheduler.

  template <class I, class M>
  void scheduler::schedule (M I::*member_ptr) {
    simple_scheduler::schedule (member_ptr);
  }

  template <class I, class M>
  void scheduler::schedule (M I::*member_ptr,
			    const typename M::parameter_type & p) {
    simple_scheduler::schedule (member_ptr, p);
  }

  template <class I, class M>
  void scheduler::schedule_after (M I::*member_ptr,
				  time offset) {
    simple_scheduler::schedule_after (member_ptr, offset);
  }
  
  template <class I, class M>
  void scheduler::schedule_after (M I::*member_ptr,
				  const typename M::parameter_type & p,
				  time offset) {
    simple_scheduler::schedule_after (member_ptr, p, offset);
  }

  template <class I, class M>
  void scheduler::schedule_read_ready (M I::*member_ptr,
				       int fd) {
    simple_scheduler::schedule_read_ready (member_ptr, fd);
  }

  template <class I, class M>
  void scheduler::schedule_read_ready (M I::*member_ptr,
				       const typename M::parameter_type& param,
				       int fd) {
    simple_scheduler::schedule_read_ready (member_ptr, param, fd);
  }
    
  template <class I, class M>
  void scheduler::schedule_write_ready (M I::*member_ptr,
				       int fd) {
    simple_scheduler::schedule_write_ready (member_ptr, fd);
  }

  template <class I, class M>
  void scheduler::schedule_write_ready (M I::*member_ptr,
				       const typename M::parameter_type& param,
				       int fd) {
    simple_scheduler::schedule_write_ready (member_ptr, param, fd);
  }

}

#endif
