#include <ioa/global_fifo_scheduler.hpp>

#include <ioa/system_scheduler_interface.hpp>

#include "model.hpp"

#include <algorithm>
#include <queue>

#include <unistd.h>
#include <sys/select.h>

#include "sys_create_runnable.hpp"
#include "sys_bind_runnable.hpp"
#include "sys_unbind_runnable.hpp"
#include "sys_destroy_runnable.hpp"
#include "sys_self_destruct_runnable.hpp"

#include "create_runnable.hpp"
#include "bind_runnable.hpp"
#include "unbind_runnable.hpp"
#include "destroy_runnable.hpp"
#include "self_destruct_runnable.hpp"

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
    std::set<int> m_close;
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

    struct compare_action_runnable
    {
      bool operator() (const action_runnable_interface* x,
		       const action_runnable_interface* y) const {
	return (*x) < (*y);
      }
    };

  public:
    global_fifo_scheduler_impl () :
      m_model (*this),
      m_current_aid (-1)
    { }

    aid_t get_current_aid () {
      assert (m_current_aid != -1);
      return m_current_aid;
    }

    size_t binding_count (const action_executor_interface& ac) {
      return m_model.binding_count (ac);
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

    void schedule (automaton::sys_self_destruct_type automaton::*member_ptr) {
      schedule_configq (new sys_self_destruct_runnable (get_current_aid ()));
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

    void run (const_shared_ptr<generator_interface> generator) {
      assert (m_configq.empty ());
      assert (m_userq.empty ());
      assert (runnable_interface::count () == 0);

      std::priority_queue<time_action, std::vector<time_action>, std::greater<time_action> > timer_queue;
      std::set<action_runnable_interface*, compare_action_runnable> timer_set;
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
	  time_action a = m_timerq.front ();
	  m_timerq.pop ();

	  if (timer_set.find (a.second) == timer_set.end ()) {
	    timer_queue.push (a);
	    timer_set.insert (a.second);
	  }
	  else {
	    // Action already has a time.
	    delete a.second;
	  }
	}

	while (!m_readq.empty ()) {
	  fd_action a = m_readq.front ();
	  m_readq.pop ();

	  if (read_actions.find (a.first) == read_actions.end ()) {
	    read_actions.insert (a);
	  }
	  else {
	    // File descriptor already has an action.
	    delete a.second;
	  }
	}
	
	while (!m_writeq.empty ()) {
	  fd_action a = m_writeq.front ();
	  m_writeq.pop ();

	  if (write_actions.find (a.first) == write_actions.end ()) {
	    write_actions.insert (a);
	  }
	  else {
	    // File descriptor already has an action.
	    delete a.second;
	  }
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
	  
	  // Remove closed fds.
	  for (std::set<int>::const_iterator pos = m_close.begin ();
	       pos != m_close.end ();
	       ++pos) {
	    std::map<int, action_runnable_interface*>::iterator p;

	    p = read_actions.find (*pos);
	    if (p != read_actions.end ()) {
	      delete p->second;
	      read_actions.erase (p);
	    }

	    p = write_actions.find (*pos);
	    if (p != write_actions.end ()) {
	      delete p->second;
	      write_actions.erase (p);
	    }

	    ::close (*pos);	    
	  }

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
	      time_action a= timer_queue.top ();
	      timer_queue.pop ();
	      timer_set.erase (a.second);
	      schedule_userq (a.second);
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

	m_close.clear ();

	// Process configuration actions.
	if (!m_configq.empty ()) {
	  runnable_interface* r = m_configq.front ();
	  m_configq.pop ();
	  (*r) (m_model);
	  delete r;
	}

	// Process user actions.
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

    void close (int fd) {
      m_close.insert (fd);
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
		 const_shared_ptr<generator_interface> generator,
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

    void self_destruct (const aid_t automaton) {
      schedule_configq (new self_destruct_runnable (automaton));
    }

    void created (const aid_t aid,
		  const automaton::created_t t,
		  void* const key,
		  const aid_t child) {
      schedule_configq (make_action_runnable (automaton_handle<automaton> (aid), &automaton::sys_created, automaton::created_arg_t (t, key, child), system_input_category ()));
    }
  
    void bound (const aid_t aid,
		const automaton::bound_t t,
		void* const key) {
      schedule_configq (make_action_runnable (automaton_handle<automaton> (aid), &automaton::sys_bound, std::make_pair (t, key), system_input_category ()));
    }

    void output_bound (const output_executor_interface& exec) {
      schedule_configq (new output_bound_runnable (exec));
      // Schedule the output.
      schedule_userq (new output_exec_runnable (exec));
    }

    void input_bound (const input_executor_interface& exec) {
      schedule_configq (new input_bound_runnable (exec));
    }

    void unbound (const aid_t aid,
		  const automaton::unbound_t t,
		  void* const key) {
      schedule_configq (make_action_runnable (automaton_handle<automaton> (aid), &automaton::sys_unbound, std::make_pair (t, key), system_input_category ()));
    }

    void output_unbound (const output_executor_interface& exec) {
      schedule_configq (new output_unbound_runnable (exec));
    }

    void input_unbound (const input_executor_interface& exec) {
      schedule_configq (new input_unbound_runnable (exec));
    }

    void destroyed (const aid_t aid,
		    const automaton::destroyed_t t,
		    void* const key) {
      schedule_configq (make_action_runnable (automaton_handle<automaton> (aid), &automaton::sys_destroyed, std::make_pair (t, key), system_input_category ()));
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
  
  size_t global_fifo_scheduler::binding_count (const action_executor_interface& ac) {
    return m_impl->binding_count (ac);
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

  void global_fifo_scheduler::schedule (automaton::sys_self_destruct_type automaton::*ptr) {
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

  void global_fifo_scheduler::close (int fd) {
    m_impl->close (fd);
  }
  
  void global_fifo_scheduler::run (const_shared_ptr<generator_interface> generator) {
    m_impl->run (generator);
  }
  
}
