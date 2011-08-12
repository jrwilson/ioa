#include <ioa/simple_scheduler.hpp>

#include <ioa/system_scheduler_interface.hpp>

#include "model.hpp"
#include "blocking_list.hpp"
#include "thread_key.hpp"
#include "lock.hpp"
#include "thread.hpp"

#include <algorithm>
#include <queue>

#include <fcntl.h>
#include <sys/select.h>
#include <unistd.h>

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

  class simple_scheduler_impl :
    public system_scheduler_interface
  {
  private:

    model m_model;
    blocking_list<std::pair<bool, runnable_interface*> > m_sysq;
    blocking_list<std::pair<bool, action_runnable_interface*> > m_execq;
    int m_wakeup_fd[2];
    blocking_list<time_action> m_timerq;
    blocking_list<fd_action> m_readq;
    blocking_list<fd_action> m_writeq;
    // TODO:  Replace with block set.  Actually, all of these could be sets.
    blocking_list<int> m_closeq;
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

    struct compare_action_runnable
    {
      bool operator() (const action_runnable_interface* x,
		       const action_runnable_interface* y) const {
	return (*x) < (*y);
      }
    };

    bool keep_going () {
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
	wakeup_io_thread ();
      }
      return retval;
    }

    void schedule_sysq (runnable_interface* r) {
      m_sysq.push (std::make_pair (true, r));
    }

    void process_sysq () {
      clear_current_aid ();
      while (thread_keep_going ()) {
	std::pair<bool, runnable_interface*> r = m_sysq.pop ();
	if (r.first) {
	  (*r.second) (m_model);
	  delete r.second;
	}
      }
    }

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
	lock lock (m_execq.list_mutex);
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

    void process_execq () {
      clear_current_aid ();
      while (thread_keep_going ()) {
	std::pair<bool, runnable_interface*> r = m_execq.pop ();
	if (r.first) {
	  (*r.second) (m_model);
	  delete r.second;
	}
      }
    }

    void wakeup_io_thread () {
      char c;
      ssize_t bytes_written = write (m_wakeup_fd[1], &c, 1);
      assert (bytes_written == 1);
    }

    void schedule_timerq (action_runnable_interface* r, const time& offset) {
      if (m_timerq.push (std::make_pair (time::now () + offset, r)) == 1) {
	wakeup_io_thread ();
      }
    }
  
    void schedule_readq (action_runnable_interface* r, int fd) {
      if (m_readq.push (std::make_pair (fd, r)) == 1) {
	wakeup_io_thread ();
      }
    }

    void schedule_writeq (action_runnable_interface* r, int fd) {
      if (m_writeq.push (std::make_pair (fd, r)) == 1) {
	wakeup_io_thread ();
      }
    }

    void process_ioq () {
      clear_current_aid ();
    
      std::priority_queue<time_action, std::vector<time_action>, std::greater<time_action> > timer_queue;
      std::set<action_runnable_interface*, compare_action_runnable> timer_set;
      std::map<int, action_runnable_interface*> read_actions;
      std::map<int, action_runnable_interface*> write_actions;
    
      fd_set read_set;
      FD_ZERO (&read_set);
      fd_set write_set;
      FD_ZERO (&write_set);

      // TODO:  We iterate over the same data structures many times.  Could we do this better?
    
      while (thread_keep_going ()) {
	// Process registrations.
	{
	  lock lock (m_timerq.list_mutex);
	  while (!m_timerq.list.empty ()) {
	    time_action a = m_timerq.list.front ();
	    m_timerq.list.pop_front ();

	    if (timer_set.find (a.second) == timer_set.end ()) {
	      timer_queue.push (a);
	      timer_set.insert (a.second);
	    }
	    else {
	      // Duplicate.
	      delete a.second;
	    }
	  }
	}

	{
	  lock lock (m_readq.list_mutex);
	  while (!m_readq.list.empty ()) {
	    fd_action a = m_readq.list.front ();
	    m_readq.list.pop_front ();
	    
	    if (read_actions.find (a.first) == read_actions.end ()) {
	      read_actions.insert (a);
	    }
	    else {
	      delete a.second;
	    }
	  }
	}

	{
	  lock lock (m_writeq.list_mutex);
	  while (!m_writeq.list.empty ()) {
	    fd_action a = m_writeq.list.front ();
	    m_writeq.list.pop_front ();
	    
	    if (write_actions.find (a.first) == write_actions.end ()) {
	      write_actions.insert (a);
	    }
	    else {
	      delete a.second;
	    }
	  }
	}

	// Determine timeout for select.
	// Default is to wait forever.
	struct timeval* test_timeout;
	struct timeval timeout;
      
	if (timer_queue.empty ()) {
	  test_timeout = 0;
	}
	else {
	  time now = time::now ();

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

	// Remove closed fds.
	{
	  lock lock (m_closeq.list_mutex);
	  while (!m_closeq.list.empty ()) {
	    int fd = m_closeq.list.front ();
	    m_closeq.list.pop_front ();

	    std::map<int, action_runnable_interface*>::iterator p;

	    p = read_actions.find (fd);
	    if (p != read_actions.end ()) {
	      delete p->second;
	      read_actions.erase (p);
	    }

	    p = write_actions.find (fd);
	    if (p != write_actions.end ()) {
	      delete p->second;
	      write_actions.erase (p);
	    }

	    ::close (fd);	    
	  }
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
      
	// Add the wake-up channel.
	FD_SET (m_wakeup_fd[0], &read_set);

	// TODO: Do better than FD_SETSIZE.
	int max_fd = m_wakeup_fd[0];
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
	  time now = time::now ();
	
	  while (!timer_queue.empty () && timer_queue.top ().first < now) {
	    schedule_execq (timer_queue.top ().second);
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
	      schedule_execq (pos->second);
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
	      schedule_execq (pos->second);
	      write_actions.erase (pos++);
	    }
	    else {
	      ++pos;
	    }
	  }
	}

	// Process the wakeup pipe.      
	if (select_result > 0) {
	  if (FD_ISSET (m_wakeup_fd[0], &read_set)) {
	    // Drain it.
	    // There are three queues: timer, read, and write.
	    // Each queue writes 1 byte when it goes from empty to non-empty.
	    // Thus, we need to read 3 bytes at maximum.
	    char c[3];
	    assert (read (m_wakeup_fd[0], c, 3) > 0);
	  }
	}

      }
    }

  public:
    simple_scheduler_impl () :
      m_model (*this)
    { }

    aid_t get_current_aid () {
      aid_t retval = m_current_aid.get ();
      assert (retval != -1);
      return retval;
    }

    size_t binding_count (const action_executor_interface& ac) {
      return m_model.binding_count (ac);
    }
  
    void schedule (automaton::sys_create_type automaton::*member_ptr) {
      // TODO:  Could these go on the execq?
      schedule_sysq (new sys_create_runnable (get_current_aid ()));
    }
  
    void schedule (automaton::sys_bind_type automaton::*member_ptr) {
      schedule_sysq (new sys_bind_runnable (get_current_aid ()));
    }

    void schedule (automaton::sys_unbind_type automaton::*member_ptr) {
      schedule_sysq (new sys_unbind_runnable (get_current_aid ()));
    }
  
    void schedule (automaton::sys_destroy_type automaton::*member_ptr) {
      schedule_sysq (new sys_destroy_runnable (get_current_aid ()));
    }

    void schedule (automaton::sys_self_destruct_type automaton::*member_ptr) {
      schedule_sysq (new sys_self_destruct_runnable (get_current_aid ()));
    }

    void schedule (action_runnable_interface* r) {
      schedule_execq (r);
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

    void run (std::auto_ptr<generator_interface> generator) {
      int r;
    
      assert (m_sysq.list.size () == 0);
      assert (m_execq.list.size () == 0);
      assert (!keep_going ());
    
      // Create a pipe to communicate with the timer thread.
      r = pipe (m_wakeup_fd);
      assert (r == 0);
      r = fcntl (m_wakeup_fd[0], F_SETFL, O_NONBLOCK);
      assert (r == 0);
      r = fcntl (m_wakeup_fd[1], F_SETFL, O_NONBLOCK);
      assert (r == 0);

      // Comes pipe creation because we might want to schedule with delay.
      m_model.create (generator);
    
      thread sysq_thread (*this, &simple_scheduler_impl::process_sysq);
      thread execq_thread (*this, &simple_scheduler_impl::process_execq);
      thread timerq_thread (*this, &simple_scheduler_impl::process_ioq);


      sysq_thread.join ();
      execq_thread.join ();
      timerq_thread.join ();

      // There are no runnables left in the system, thus, there is no more work to do.
      // If all of the automata have been coded correctly, then we have reached "fixed point".

      // Consequently, we are going to reset.

      // We clear the system first because it might add something to a run queue.
      m_model.clear ();
    
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
    
      // TODO:  Do I need to close both ends?
      close (m_wakeup_fd[0]);
      close (m_wakeup_fd[1]);

      // Notice that the post-conditions match the preconditions.
      assert (m_sysq.list.size () == 0);
      assert (m_execq.list.size () == 0);
      assert (!keep_going ());
    }
  
    void close (int fd) {
      if (m_closeq.push (fd) == 1) {
	wakeup_io_thread ();
      }
    }

    void set_current_aid (const aid_t aid) {
      // This is to be used during generation so that any allocated memory can be associated with the automaton.
      assert (aid != -1);
      m_current_aid.set (aid);
    }
  
    void clear_current_aid () {
      m_current_aid.set (-1);
    }

    void create (const aid_t automaton,
		 std::auto_ptr<generator_interface> generator,
		 void* const key) {
      schedule_sysq (new create_runnable (automaton, generator, key));
    }

    void bind (const aid_t automaton,
	       shared_ptr<bind_executor_interface> exec,
	       void* const key) {
      schedule_sysq (new bind_runnable (automaton, exec, key));
    }
  
    void unbind (const aid_t automaton,
		 void* const key) {
      schedule_sysq (new unbind_runnable (automaton, key));
    }
  
    void destroy (const aid_t automaton,
		  void* const key) {
      schedule_sysq (new destroy_runnable (automaton, key));
    }

    void self_destruct (const aid_t automaton) {
      schedule_sysq (new self_destruct_runnable (automaton));
    }

    void created (const aid_t aid,
		  const created_t t,
		  void* const key,
		  const aid_t child) {
      schedule_sysq (make_action_runnable (automaton_handle<automaton> (aid), &automaton::sys_created, automaton::created_arg_t (t, key, child), system_input_category ()));
    }
  
    void bound (const aid_t aid,
		const bound_t t,
		void* const key) {
      schedule_sysq (make_action_runnable (automaton_handle<automaton> (aid), &automaton::sys_bound, std::make_pair (t, key), system_input_category ()));
    }

    void output_bound (const output_executor_interface& exec) {
      schedule_sysq (new output_bound_runnable (exec));
      // Schedule the output.
      schedule_execq (new output_exec_runnable (exec));
    }

    void input_bound (const input_executor_interface& exec) {
      schedule_sysq (new input_bound_runnable (exec));
    }

    void unbound (const aid_t aid,
		  const unbound_t t,
		  void* const key) {
      schedule_sysq (make_action_runnable (automaton_handle<automaton> (aid), &automaton::sys_unbound, std::make_pair (t, key), system_input_category ()));
    }

    void output_unbound (const output_executor_interface& exec) {
      schedule_sysq (new output_unbound_runnable (exec));
    }

    void input_unbound (const input_executor_interface& exec) {
      schedule_sysq (new input_unbound_runnable (exec));
    }

    void destroyed (const aid_t aid,
		    const destroyed_t t,
		    void* const key) {
      schedule_sysq (make_action_runnable (automaton_handle<automaton> (aid), &automaton::sys_destroyed, std::make_pair (t, key), system_input_category ()));
    }
  };

  simple_scheduler::simple_scheduler () :
    m_impl (new simple_scheduler_impl ())
  { }

  simple_scheduler::~simple_scheduler () {
    delete m_impl;
  }
    
  aid_t simple_scheduler::get_current_aid () {
    return m_impl->get_current_aid ();
  }
  
  size_t simple_scheduler::binding_count (const action_executor_interface& ac) {
    return m_impl->binding_count (ac);
  }
  
  void simple_scheduler::schedule (automaton::sys_create_type automaton::*ptr) {
    m_impl->schedule (ptr);
  }
    
  void simple_scheduler::schedule (automaton::sys_bind_type automaton::*ptr) {
    m_impl->schedule (ptr);
  }
  
  void simple_scheduler::schedule (automaton::sys_unbind_type automaton::*ptr) {
    m_impl->schedule (ptr);
  }
  
  void simple_scheduler::schedule (automaton::sys_destroy_type automaton::*ptr) {
    m_impl->schedule (ptr);
  }
  
  void simple_scheduler::schedule (automaton::sys_self_destruct_type automaton::*ptr) {
    m_impl->schedule (ptr);
  }

  void simple_scheduler::schedule (action_runnable_interface* r) {
    m_impl->schedule (r);
  }
  
  void simple_scheduler::schedule_after (action_runnable_interface* r,
					 const time& offset) {
    m_impl->schedule_after (r, offset);
  }
  
  void simple_scheduler::schedule_read_ready (action_runnable_interface* r,
					      int fd) {
    m_impl->schedule_read_ready (r, fd);
  }
  
  void simple_scheduler::schedule_write_ready (action_runnable_interface* r,
					       int fd) {
    m_impl->schedule_write_ready (r, fd);
  }

  void simple_scheduler::close (int fd) {
    m_impl->close (fd);
  }

  void simple_scheduler::run (std::auto_ptr<generator_interface> generator) {
    m_impl->run (generator);
  }
  
}
