#ifndef __simple_scheduler_hpp__
#define __simple_scheduler_hpp__

#include <ioa/scheduler.hpp>
#include <ioa/action_runnable.hpp>
#include <ioa/blocking_list.hpp>
#include <ioa/thread_key.hpp>
#include <ioa/time.hpp>
#include <ioa/automaton_interface.hpp>

namespace ioa {

  typedef std::pair<action_runnable_interface*, time> action_time;
  typedef std::pair<action_runnable_interface*, int> action_fd;

  class simple_scheduler :
    public system_scheduler_interface,
    public scheduler_interface
  {
  private:

    model m_model;
    blocking_list<std::pair<bool, runnable_interface*> > m_sysq;
    blocking_list<std::pair<bool, action_runnable_interface*> > m_execq;
    int m_wakeup_fd[2];
    blocking_list<action_time> m_timerq;
    blocking_list<action_fd> m_readq;
    blocking_list<action_fd> m_writeq;
    thread_key<aid_t> m_current_aid;

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

    bool keep_going ();
    bool thread_keep_going ();
    void schedule_sysq (runnable_interface* r);
    void process_sysq ();
    void schedule_execq (action_runnable_interface* r);
    void process_execq ();
    void wakeup_io_thread ();
    void schedule_timerq (action_runnable_interface* r, const time& offset);
    void schedule_readq (action_runnable_interface* r, int);
    void schedule_writeq (action_runnable_interface* r, int);
    void process_ioq ();
    
  public:
    simple_scheduler ();

    void set_current_aid (const aid_t aid);
    void clear_current_aid ();

    void create (const aid_t automaton,
			shared_ptr<generator_interface> generator,
			void* const key);

    void bind (const aid_t automaton,
		      shared_ptr<bind_executor_interface> generator,
		      void* const key);

    void unbind (const aid_t automaton,
			void* const key);

    void destroy (const aid_t automaton,
			 void* const key);

    void create_key_exists (const aid_t automaton,
				   void* const key);

    void instance_exists (const aid_t automaton,
				 void* const key);

    void automaton_created (const aid_t automaton,
				   void* const key,
				   const aid_t child);

    void bind_key_exists (const aid_t automaton,
				 void* const key);

    void output_automaton_dne (const aid_t automaton,
				      void* const key);

    void input_automaton_dne (const aid_t automaton,
				      void* const key);

    void binding_exists (const aid_t automaton,
				void* const key);

    void input_action_unavailable (const aid_t automaton,
					  void* const key);

    void output_action_unavailable (const aid_t automaton,
					   void* const key);
    
    void bound (const aid_t automaton,
		       void* const key);

    void output_bound (const output_executor_interface&);

    void input_bound (const input_executor_interface&);

    void bind_key_dne (const aid_t automaton,
			      void* const key);

    void unbound (const aid_t automaton,
			 void* const key);

    void output_unbound (const output_executor_interface&);

    void input_unbound (const input_executor_interface&);

    void create_key_dne (const aid_t automaton,
				void* const key);

    void automaton_destroyed (const aid_t automaton,
				     void* const key);
    
    void recipient_dne (const aid_t automaton,
			       void* const key);

    void event_delivered (const aid_t automaton,
				 void* const key);
    
    aid_t get_current_aid ();

    size_t bind_count (const action_interface&);

    void schedule (automaton_interface::sys_create_type automaton_interface::*member_ptr);

    void schedule (automaton_interface::sys_bind_type automaton_interface::*member_ptr);

    void schedule (automaton_interface::sys_unbind_type automaton_interface::*member_ptr);

    void schedule (automaton_interface::sys_destroy_type automaton_interface::*member_ptr);
  
    void schedule (action_runnable_interface*);

    void schedule_after (action_runnable_interface*,
			 const time&);

    void schedule_read_ready (action_runnable_interface*,
			      int fd);

    void schedule_write_ready (action_runnable_interface*,
			       int fd);

    template <class I, class M>
    void schedule (M I::*member_ptr) {
      schedule_execq (make_action_runnable (make_action (automaton_handle<I> (get_current_aid ()), member_ptr)));
    }

    template <class I, class M>
    void schedule (M I::*member_ptr,
                          const typename M::parameter_type& param) {
      schedule_execq (make_action_runnable (make_action (automaton_handle<I> (get_current_aid ()), member_ptr, param)));
    }

    template <class I, class M>
    void schedule_after (M I::*member_ptr,
				time offset) {
      schedule_timerq (make_action_runnable (make_action (automaton_handle<I> (get_current_aid ()), member_ptr)), offset);
    }

    template <class I, class M>
    void schedule_after (M I::*member_ptr,
				const typename M::parameter_type& param,
				time offset) {
      schedule_timerq (make_action_runnable (make_action (automaton_handle<I> (get_current_aid ()), member_ptr, param)), offset);
    }

    template <class I, class M>
    void schedule_read_ready (M I::*member_ptr,
				     int fd) {
      schedule_readq (make_action_runnable (make_action (automaton_handle<I> (get_current_aid ()), member_ptr)), fd);
    }

    template <class I, class M>
    void schedule_read_ready (M I::*member_ptr,
				     const typename M::parameter_type& param,
				     int fd) {
      schedule_readq (make_action_runnable (make_action (automaton_handle<I> (get_current_aid ()), member_ptr, param)), fd);
    }

    template <class I, class M>
    void schedule_write_ready (M I::*member_ptr,
				     int fd) {
      schedule_writeq (make_action_runnable (make_action (automaton_handle<I> (get_current_aid ()), member_ptr)), fd);
    }

    template <class I, class M>
    void schedule_write_ready (M I::*member_ptr,
				     const typename M::parameter_type& param,
				     int fd) {
      schedule_writeq (make_action_runnable (make_action (automaton_handle<I> (get_current_aid ()), member_ptr, param)), fd);
    }
    
    void run (shared_ptr<generator_interface> generator);
  };

  // Implement the scheduler.

  // template <class I, class M>
  // void scheduler::schedule (M I::*member_ptr) {
  //   simple_scheduler::schedule (member_ptr);
  // }

  // template <class I, class M>
  // void scheduler::schedule (M I::*member_ptr,
  // 			    const typename M::parameter_type & p) {
  //   simple_scheduler::schedule (member_ptr, p);
  // }

  // template <class I, class M>
  // void scheduler::schedule_after (M I::*member_ptr,
  // 				  time offset) {
  //   simple_scheduler::schedule_after (member_ptr, offset);
  // }
  
  // template <class I, class M>
  // void scheduler::schedule_after (M I::*member_ptr,
  // 				  const typename M::parameter_type & p,
  // 				  time offset) {
  //   simple_scheduler::schedule_after (member_ptr, p, offset);
  // }

  // template <class I, class M>
  // void scheduler::schedule_read_ready (M I::*member_ptr,
  // 				       int fd) {
  //   simple_scheduler::schedule_read_ready (member_ptr, fd);
  // }

  // template <class I, class M>
  // void scheduler::schedule_read_ready (M I::*member_ptr,
  // 				       const typename M::parameter_type& param,
  // 				       int fd) {
  //   simple_scheduler::schedule_read_ready (member_ptr, param, fd);
  // }
    
  // template <class I, class M>
  // void scheduler::schedule_write_ready (M I::*member_ptr,
  // 				       int fd) {
  //   simple_scheduler::schedule_write_ready (member_ptr, fd);
  // }

  // template <class I, class M>
  // void scheduler::schedule_write_ready (M I::*member_ptr,
  // 				       const typename M::parameter_type& param,
  // 				       int fd) {
  //   simple_scheduler::schedule_write_ready (member_ptr, param, fd);
  // }

  // template <class I, class M>
  // size_t scheduler::bind_count (M I::*member_ptr) {
  //   return model::bind_count (make_action (automaton_handle<I> (get_current_aid ()), member_ptr));
  // }
  
  // template <class I, class M>
  // size_t scheduler::bind_count (M I::*member_ptr,
  // 				const typename M::parameter_type& param) {
  //   return model::bind_count (make_action (automaton_handle<I> (get_current_aid ()), member_ptr, param));
  // }


}

#endif
