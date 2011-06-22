#include <ioa/global_fifo_scheduler.hpp>

#include <ioa/system_scheduler_interface.hpp>

#include "model.hpp"

#include <algorithm>
#include <queue>

#include <sys/select.h>

#include "sys_create_runnable.hpp"
#include "sys_bind_runnable.hpp"
#include "sys_unbind_runnable.hpp"
#include "sys_destroy_runnable.hpp"

#include "create_runnable.hpp"
#include "bind_runnable.hpp"
#include "unbind_runnable.hpp"
#include "destroy_runnable.hpp"

#include "output_exec_runnable.hpp"
#include "output_bound_runnable.hpp"
#include "input_bound_runnable.hpp"
#include "output_unbound_runnable.hpp"
#include "input_unbound_runnable.hpp"

namespace ioa {

  typedef std::pair<time, action_runnable_interface*> time_action;
  typedef std::pair<int, action_runnable_interface*> fd_action;

  class global_fifo_scheduler_impl :
    public system_scheduler_interface
  {
  private:

    model m_model;
    std::queue<runnable_interface*> m_configq;
    std::list<action_runnable_interface*> m_userq;
    std::queue<time_action> m_timerq;
    std::queue<fd_action> m_readq;
    std::queue<fd_action> m_writeq;
    aid_t m_current_aid;

    struct action_runnable_equal
    {
      const action_runnable_interface* m_ptr;

      action_runnable_equal (const action_runnable_interface* ptr) :
  	m_ptr (ptr)
      { }

      bool operator() (action_runnable_interface*& x) const {
	return (*m_ptr) == (*x);
      }
    };

    void schedule_configq (runnable_interface* r) {
      m_configq.push (r);
    }

    void schedule_userq (action_runnable_interface* r) {
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
      if (std::find_if (m_userq.begin (), m_userq.end (), action_runnable_equal (r)) == m_userq.end ()) {
    	m_userq.push_back (r);	
      }
      else {
    	delete r;
      }
    }

    void schedule_timerq (action_runnable_interface* r, const time& offset) {
      struct timeval now;
      int s = gettimeofday (&now, 0);
      assert (s == 0);
      time release_time (now);
      release_time += offset;
      
      m_timerq.push (std::make_pair (release_time, r));
    }
  
    void schedule_readq (action_runnable_interface* r, int fd) {
      m_readq.push (std::make_pair (fd, r));
    }

    void schedule_writeq (action_runnable_interface* r, int fd) {
      m_writeq.push (std::make_pair (fd, r));
    }

  public:
    global_fifo_scheduler_impl () :
      m_model (*this),
      m_current_aid (-1)
    { }

    aid_t get_current_aid () {
      assert (m_current_aid != -1);
      return m_current_aid;
    }

    size_t bind_count (const action_executor_interface& ac) {
      return m_model.bind_count (ac);
    }
  
    void schedule (automaton::sys_create_type automaton::*member_ptr) {
      // TODO:  Could these go on the userq?
      schedule_configq (new sys_create_runnable (get_current_aid ()));
    }
  
    void schedule (automaton::sys_bind_type automaton::*member_ptr) {
      schedule_configq (new sys_bind_runnable (get_current_aid ()));
    }

    void schedule (automaton::sys_unbind_type automaton::*member_ptr) {
      schedule_configq (new sys_unbind_runnable (get_current_aid ()));
    }
  
    void schedule (automaton::sys_destroy_type automaton::*member_ptr) {
      schedule_configq (new sys_destroy_runnable (get_current_aid ()));
    }

    void schedule (action_runnable_interface* r) {
      schedule_userq (r);
    }

    void schedule_after (action_runnable_interface* r,
			 const time& offset) {
      schedule_timerq (r, offset);
    }

    void schedule_read_ready (action_runnable_interface* r,
			      int fd) {
      schedule_readq (r, fd);
    }

    void schedule_write_ready (action_runnable_interface* r,
			       int fd) {
      schedule_writeq (r, fd);
    }

    void run (shared_ptr<generator_interface> generator) {
      assert (m_configq.empty ());
      assert (m_userq.empty ());
      assert (runnable_interface::count () == 0);

      std::priority_queue<time_action, std::vector<time_action>, std::greater<time_action> > timer_queue;
      std::map<int, action_runnable_interface*> read_actions;
      std::map<int, action_runnable_interface*> write_actions;
      
      fd_set read_set;
      FD_ZERO (&read_set);
      fd_set write_set;
      FD_ZERO (&write_set);

      clear_current_aid ();
    
      m_model.create (generator);

      // TODO:  We iterate over the same data structures many times.  Could we do this better?

      while (runnable_interface::count () != 0) {
	// There is work to do.
    
    	// Process registrations.
	while (!m_timerq.empty ()) {
	  timer_queue.push (m_timerq.front ());
	  m_timerq.pop ();
	}

	while (!m_readq.empty ()) {
	  std::map<int, action_runnable_interface*>::iterator pos = read_actions.find (m_readq.front ().first);
	  if (pos != read_actions.end ()) {
	    // Action already registered using this fd.
	    delete pos->second;
	    read_actions.erase (pos);
	  }
	  read_actions.insert (m_readq.front ());
	  m_readq.pop ();
	}
	
	while (!m_writeq.empty ()) {
	  std::map<int, action_runnable_interface*>::iterator pos = write_actions.find (m_writeq.front ().first);
	  if (pos != write_actions.end ()) {
	    // Action already registered using this fd.
	    delete pos->second;
	    write_actions.erase (pos);
	  }
	  write_actions.insert (m_writeq.front ());
	  m_writeq.pop ();
	}

    	// Determine timeout for select.
    	struct timeval* test_timeout;
    	struct timeval timeout;


	// Start by assuming we need to wait forever.
	test_timeout = 0;

	// If the timer queue is empty, set a timeout.
	if (!timer_queue.empty ()) {
    	  struct timeval n;
    	  int r = gettimeofday (&n, 0);
    	  assert (r == 0);
    	  time now (n);

    	  if (timer_queue.top ().first > now) {
    	    // Timer is some time in future.
    	    timeout = timer_queue.top ().first - now;
    	  }
    	  else {
    	    // Timer is in the past.  Return immediately.
    	    timeout = time (0, 0);
    	  }

    	  test_timeout = &timeout;
    	}

	// If we have work to do, go immediately.
	if (!m_configq.empty () || !m_userq.empty ()) {
	  timeout = time (0, 0);
	  test_timeout = &timeout;
	}

	// We only need to select if we have fds or timers.
	if (!read_actions.empty () || !write_actions.empty () || !timer_queue.empty ()) {
	  
	  // Determine the read set.
	  for (std::map<int, action_runnable_interface*>::const_iterator pos = read_actions.begin ();
	       pos != read_actions.end ();
	       ++pos) {
	    FD_SET (pos->first, &read_set);
	  }
	  
	  // Determine the write set.
	  for (std::map<int, action_runnable_interface*>::const_iterator pos = write_actions.begin ();
	       pos != write_actions.end ();
	       ++pos) {
	    FD_SET (pos->first, &write_set);
	  }
	  
	  // TODO: Do better than FD_SETSIZE.
	  int max_fd = 0;
	  if (!read_actions.empty ()) {
	    max_fd = std::max ((--read_actions.end ())->first, max_fd);
	  }
	  if (!write_actions.empty ()) {
	    max_fd = std::max ((--write_actions.end ())->first, max_fd);
	  }
	  int select_result = select (max_fd + 1, &read_set, &write_set, 0, test_timeout);
	  assert (select_result >= 0);
	  
	  // Process timers.
	  {
	    struct timeval n;
	    int r = gettimeofday (&n, 0);
	    assert (r == 0);
	    time now (n);
	    
	    while (!timer_queue.empty () && timer_queue.top ().first < now) {
	      schedule_userq (timer_queue.top ().second);
	      timer_queue.pop ();
	    }
	  }
	  
	  // Process reads.
	  if (select_result > 0) {
	    for (std::map<int, action_runnable_interface*>::iterator pos = read_actions.begin ();
		 pos != read_actions.end ();
		 ) {
	      if (FD_ISSET (pos->first, &read_set)) {
		FD_CLR (pos->first, &read_set);
		schedule_userq (pos->second);
		read_actions.erase (pos++);
	      }
	      else {
		++pos;
	      }
	    }
	  }
	  
	  // Process writes.
	  if (select_result > 0) {
	    for (std::map<int, action_runnable_interface*>::iterator pos = write_actions.begin ();
		 pos != write_actions.end ();
		 ) {
	      if (FD_ISSET (pos->first, &write_set)) {
		FD_CLR (pos->first, &write_set);
		schedule_userq (pos->second);
		write_actions.erase (pos++);
	      }
	      else {
		++pos;
	      }
	    }
	  }

	}

	// Process configuration actions.
	if (!m_configq.empty ()) {
	  runnable_interface* r = m_configq.front ();
	  m_configq.pop ();
	  (*r) (m_model);
	  delete r;
	}

	if (!m_userq.empty ()) {
	  runnable_interface* r = m_userq.front ();
	  m_userq.pop_front ();
	  (*r) (m_model);
	  delete r;
	}

      }

      // There are no runnables left in the system, thus, there is no more work to do.
      // If all of the automata have been coded correctly, then we have reached "fixed point".

      // Consequently, we are going to reset.

      // We clear the system first because it might add something to a run queue.
      m_model.clear ();
    
      // Then, we clear the run queues.
      while (!m_configq.empty ()) {
	runnable_interface* r = m_configq.front ();
	m_configq.pop ();
	delete r;
      }
    
      for (std::list<action_runnable_interface*>::iterator pos = m_userq.begin ();
      	   pos != m_userq.end ();
      	   ++pos) {
      	delete (*pos);
      }
      m_userq.clear ();
    
      // Notice that the post-conditions match the preconditions.
      assert (m_configq.empty ());
      assert (m_userq.empty ());
      assert (runnable_interface::count () == 0);
    }
  
    void set_current_aid (const aid_t aid) {
      // This is to be used during generation so that any allocated memory can be associated with the automaton.
      assert (aid != -1);
      m_current_aid = aid;
    }
  
    void clear_current_aid () {
      m_current_aid = -1;
    }

    void create (const aid_t automaton,
		 shared_ptr<generator_interface> generator,
		 void* const key) {
      schedule_configq (new create_runnable (automaton, generator, key));
    }

    void bind (const aid_t automaton,
	       shared_ptr<bind_executor_interface> exec,
	       void* const key) {
      schedule_configq (new bind_runnable (automaton, exec, key));
    }
  
    void unbind (const aid_t automaton,
		 void* const key) {
      schedule_configq (new unbind_runnable (automaton, key));
    }
  
    void destroy (const aid_t automaton,
		  void* const key) {
      schedule_configq (new destroy_runnable (automaton, key));
    }

    void create_key_exists (const aid_t aid,
			    void* const key) {
      schedule_configq (make_action_runnable (automaton_handle<automaton> (aid), &automaton::sys_create_key_exists, key, system_input_category ()));
    }

    void instance_exists (const aid_t aid,
			  void* const key) {
      schedule_configq (make_action_runnable (automaton_handle<automaton> (aid), &automaton::sys_instance_exists, key, system_input_category ()));
    }
  
    void automaton_created (const aid_t aid,
			    void* const key,
			    const aid_t child) {
      schedule_configq (make_action_runnable (automaton_handle<automaton> (aid), &automaton::sys_automaton_created, std::make_pair (key, child), system_input_category ()));
    }
  
    void bind_key_exists (const aid_t aid,
			  void* const key) {
      schedule_configq (make_action_runnable (automaton_handle<automaton> (aid), &automaton::sys_bind_key_exists, key, system_input_category ()));
    }

    void output_automaton_dne (const aid_t aid,
			       void* const key) {
      schedule_configq (make_action_runnable (automaton_handle<automaton> (aid), &automaton::sys_output_automaton_dne, key, system_input_category ()));
    }

    void input_automaton_dne (const aid_t aid,
			      void* const key) {
      schedule_configq (make_action_runnable (automaton_handle<automaton> (aid), &automaton::sys_input_automaton_dne, key, system_input_category ()));
    }
  
    void binding_exists (const aid_t aid,
			 void* const key) {
      schedule_configq (make_action_runnable (automaton_handle<automaton> (aid), &automaton::sys_binding_exists, key, system_input_category ()));
    }
  
    void input_action_unavailable (const aid_t aid,
				   void* const key) {
      schedule_configq (make_action_runnable (automaton_handle<automaton> (aid), &automaton::sys_input_action_unavailable, key, system_input_category ()));
    }
  
    void output_action_unavailable (const aid_t aid,
				    void* const key) {
      schedule_configq (make_action_runnable (automaton_handle<automaton> (aid), &automaton::sys_output_action_unavailable, key, system_input_category ()));
    }
    
    void bound (const aid_t aid,
		void* const key) {
      schedule_configq (make_action_runnable (automaton_handle<automaton> (aid), &automaton::sys_bound, key, system_input_category ()));
    }

    void output_bound (const output_executor_interface& exec) {
      schedule_configq (new output_bound_runnable (exec));
      // Schedule the output.
      schedule_userq (new output_exec_runnable (exec));
    }

    void input_bound (const input_executor_interface& exec) {
      schedule_configq (new input_bound_runnable (exec));
    }

    void bind_key_dne (const aid_t aid,
		       void* const key) {
      schedule_configq (make_action_runnable (automaton_handle<automaton> (aid), &automaton::sys_bind_key_dne, key, system_input_category ()));
    }
  
    void unbound (const aid_t aid,
		  void* const key) {
      schedule_configq (make_action_runnable (automaton_handle<automaton> (aid), &automaton::sys_unbound, key, system_input_category ()));
    }

    void output_unbound (const output_executor_interface& exec) {
      schedule_configq (new output_unbound_runnable (exec));
    }

    void input_unbound (const input_executor_interface& exec) {
      schedule_configq (new input_unbound_runnable (exec));
    }

    void create_key_dne (const aid_t aid,
			 void* const key) {
      schedule_configq (make_action_runnable (automaton_handle<automaton> (aid), &automaton::sys_create_key_dne, key, system_input_category ()));
    }
  
    void automaton_destroyed (const aid_t aid,
			      void* const key) {
      schedule_configq (make_action_runnable (automaton_handle<automaton> (aid), &automaton::sys_automaton_destroyed, key, system_input_category ()));
    }
    
  };

  global_fifo_scheduler::global_fifo_scheduler () :
    m_impl (new global_fifo_scheduler_impl ())
  { }

  global_fifo_scheduler::~global_fifo_scheduler () {
    delete m_impl;
  }
    
  aid_t global_fifo_scheduler::get_current_aid () {
    return m_impl->get_current_aid ();
  }
  
  size_t global_fifo_scheduler::bind_count (const action_executor_interface& ac) {
    return m_impl->bind_count (ac);
  }
  
  void global_fifo_scheduler::schedule (automaton::sys_create_type automaton::*ptr) {
    m_impl->schedule (ptr);
  }
    
  void global_fifo_scheduler::schedule (automaton::sys_bind_type automaton::*ptr) {
    m_impl->schedule (ptr);
  }
  
  void global_fifo_scheduler::schedule (automaton::sys_unbind_type automaton::*ptr) {
    m_impl->schedule (ptr);
  }
  
  void global_fifo_scheduler::schedule (automaton::sys_destroy_type automaton::*ptr) {
    m_impl->schedule (ptr);
  }
  
  void global_fifo_scheduler::schedule (action_runnable_interface* r) {
    m_impl->schedule (r);
  }
  
  void global_fifo_scheduler::schedule_after (action_runnable_interface* r,
					      const time& offset) {
    m_impl->schedule_after (r, offset);
  }
  
  void global_fifo_scheduler::schedule_read_ready (action_runnable_interface* r,
						   int fd) {
    m_impl->schedule_read_ready (r, fd);
  }
  
  void global_fifo_scheduler::schedule_write_ready (action_runnable_interface* r,
						    int fd) {
    m_impl->schedule_write_ready (r, fd);
  }
  
  void global_fifo_scheduler::run (shared_ptr<generator_interface> generator) {
    m_impl->run (generator);
  }
  
}
