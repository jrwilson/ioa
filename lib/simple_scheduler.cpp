#include <ioa/simple_scheduler.hpp>

#include <algorithm>
#include <queue>

#include <fcntl.h>
#include <sys/select.h>
#include <unistd.h>
#include <ioa/thread.hpp>
#include <ioa/sys_create_runnable.hpp>
#include <ioa/sys_bind_runnable.hpp>
#include <ioa/sys_unbind_runnable.hpp>
#include <ioa/sys_destroy_runnable.hpp>

#include <ioa/create_runnable.hpp>
#include <ioa/bind_runnable.hpp>
#include <ioa/unbind_runnable.hpp>
#include <ioa/destroy_runnable.hpp>

#include <ioa/output_bound_runnable.hpp>

namespace ioa {

  simple_scheduler::simple_scheduler () :
    m_model (*this)
  { }

  bool simple_scheduler::keep_going () {
    // The criteria for continuing is simple: a runnable exists.
    return runnable_interface::count () != 0;
  }

  bool simple_scheduler::thread_keep_going () {
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

  void simple_scheduler::schedule_sysq (runnable_interface* r) {
    m_sysq.push (std::make_pair (true, r));
  }

  void simple_scheduler::process_sysq () {
    clear_current_aid ();
    while (thread_keep_going ()) {
      std::pair<bool, runnable_interface*> r = m_sysq.pop ();
      if (r.first) {
	(*r.second) (m_model);
	delete r.second;
      }
    }
  }

  void simple_scheduler::schedule_execq (action_runnable_interface* r) {
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

  void simple_scheduler::process_execq () {
    clear_current_aid ();
    while (thread_keep_going ()) {
      std::pair<bool, runnable_interface*> r = m_execq.pop ();
      if (r.first) {
	(*r.second) (m_model);
	delete r.second;
      }
    }
  }

  void simple_scheduler::wakeup_io_thread () {
    char c;
    ssize_t bytes_written = write (m_wakeup_fd[1], &c, 1);
    assert (bytes_written == 1);
  }

  void simple_scheduler::schedule_timerq (action_runnable_interface* r, const time& offset) {
    struct timeval now;
    int s = gettimeofday (&now, 0);
    assert (s == 0);
    time release_time (now);
    release_time += offset;

    if (m_timerq.push (std::make_pair (r, release_time)) == 1) {
      wakeup_io_thread ();
    }
  }
  
  void simple_scheduler::schedule_readq (action_runnable_interface* r, int fd) {
    if (m_readq.push (std::make_pair (r, fd)) == 1) {
      wakeup_io_thread ();
    }
  }

  void simple_scheduler::schedule_writeq (action_runnable_interface* r, int fd) {
    if (m_writeq.push (std::make_pair (r, fd)) == 1) {
      wakeup_io_thread ();
    }
  }

  void simple_scheduler::process_ioq () {
    clear_current_aid ();
    
    std::priority_queue<action_time, std::vector<action_time>, std::greater<action_time> > timer_queue;
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
	  timer_queue.push (m_timerq.list.front ());
	  m_timerq.list.pop_front ();
	}
      }

      {
	lock lock (m_readq.list_mutex);
	while (!m_readq.list.empty ()) {
	  std::map<int, action_runnable_interface*>::iterator pos = read_actions.find (m_readq.list.front ().second);
	  if (pos != read_actions.end ()) {
	    // Action already registered using this fd.
	    delete pos->second;
	    read_actions.erase (pos);
	  }
	  read_actions.insert (std::make_pair (m_readq.list.front ().second, m_readq.list.front ().first));
	  m_readq.list.pop_front ();
	}
      }

      {
	lock lock (m_writeq.list_mutex);
	while (!m_writeq.list.empty ()) {
	  std::map<int, action_runnable_interface*>::iterator pos = write_actions.find (m_writeq.list.front ().second);
	  if (pos != write_actions.end ()) {
	    // Action already registered using this fd.
	    delete pos->second;
	    write_actions.erase (pos);
	  }
	  write_actions.insert (std::make_pair (m_writeq.list.front ().second, m_writeq.list.front ().first));
	  m_writeq.list.pop_front ();
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
	struct timeval n;
	int r = gettimeofday (&n, 0);
	assert (r == 0);
	time now (n);

	if (timer_queue.top ().second > now) {
	  // Timer is some time in future.
	  timeout = timer_queue.top ().second - now;
	}
	else {
	  // Timer is in the past.  Return immediately.
	  timeout = time (0, 0);
	}

	test_timeout = &timeout;
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
	struct timeval n;
	int r = gettimeofday (&n, 0);
	assert (r == 0);
	time now (n);
	
	while (!timer_queue.empty () && timer_queue.top ().second < now) {
	  schedule_execq (timer_queue.top ().first);
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
    
  aid_t simple_scheduler::get_current_aid () {
    aid_t retval = m_current_aid.get ();
    assert (retval != -1);
    return retval;
  }

  size_t simple_scheduler::bind_count (const action_interface& ac) {
    return m_model.bind_count (ac);
  }
  
  void simple_scheduler::schedule (automaton_interface::sys_create_type automaton_interface::*member_ptr) {
    // TODO:  Could these go on the execq?
    schedule_sysq (new sys_create_runnable (get_current_aid ()));
  }
  
  void simple_scheduler::schedule (automaton_interface::sys_bind_type automaton_interface::*member_ptr) {
    schedule_sysq (new sys_bind_runnable (get_current_aid ()));
  }

  void simple_scheduler::schedule (automaton_interface::sys_unbind_type automaton_interface::*member_ptr) {
    schedule_sysq (new sys_unbind_runnable (get_current_aid ()));
  }
  
  void simple_scheduler::schedule (automaton_interface::sys_destroy_type automaton_interface::*member_ptr) {
    schedule_sysq (new sys_destroy_runnable (get_current_aid ()));
  }

  void simple_scheduler::schedule (action_runnable_interface* r) {
    schedule_execq (r);
  }

  void simple_scheduler::schedule_after (action_runnable_interface* r,
					 const time& offset) {
    schedule_timerq (r, offset);
  }

  void simple_scheduler::schedule_read_ready (action_runnable_interface* r,
					      int fd) {
    schedule_readq (r, fd);
  }

  void simple_scheduler::schedule_write_ready (action_runnable_interface* r,
					       int fd) {
    schedule_writeq (r, fd);
  }
  


  void simple_scheduler::run (shared_ptr<generator_interface> generator) {
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
    
    thread sysq_thread (*this, &simple_scheduler::process_sysq);
    thread execq_thread (*this, &simple_scheduler::process_execq);
    thread timerq_thread (*this, &simple_scheduler::process_ioq);


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
  
  void simple_scheduler::set_current_aid (const aid_t aid) {
    // This is to be used during generation so that any allocated memory can be associated with the automaton.
    assert (aid != -1);
    m_current_aid.set (aid);
  }
  
  void simple_scheduler::clear_current_aid () {
    m_current_aid.set (-1);
  }

  void simple_scheduler::create (const aid_t automaton,
				 shared_ptr<generator_interface> generator,
				 void* const key) {
    schedule_sysq (new create_runnable (automaton, generator, key));
  }

  void simple_scheduler::bind (const aid_t automaton,
			       shared_ptr<bind_executor_interface> exec,
			       void* const key) {
    schedule_sysq (new bind_runnable (automaton, exec, key));
  }
  
  void simple_scheduler::unbind (const aid_t automaton,
				 void* const key) {
    schedule_sysq (new unbind_runnable (automaton, key));
  }
  
  void simple_scheduler::destroy (const aid_t automaton,
				  void* const key) {
    schedule_sysq (new destroy_runnable (automaton, key));
  }

  void simple_scheduler::create_key_exists (const aid_t automaton,
					    void* const key) {
    schedule_sysq (make_action_runnable (make_action (automaton_handle<automaton_interface> (automaton), &automaton_interface::sys_create_key_exists, key)));
  }

  void simple_scheduler::instance_exists (const aid_t automaton,
					  void* const key) {
    schedule_sysq (make_action_runnable (make_action (automaton_handle<automaton_interface> (automaton), &automaton_interface::sys_instance_exists, key)));
  }
  
  void simple_scheduler::automaton_created (const aid_t automaton,
					    void* const key,
					    const aid_t child) {
    schedule_sysq (make_action_runnable (make_action (automaton_handle<automaton_interface> (automaton), &automaton_interface::sys_automaton_created, std::make_pair (key, child))));
  }
  
  void simple_scheduler::bind_key_exists (const aid_t automaton,
					  void* const key) {
    schedule_sysq (make_action_runnable (make_action (automaton_handle<automaton_interface> (automaton), &automaton_interface::sys_bind_key_exists, key)));
  }

  void simple_scheduler::output_automaton_dne (const aid_t automaton,
					       void* const key) {
    schedule_sysq (make_action_runnable (make_action (automaton_handle<automaton_interface> (automaton), &automaton_interface::sys_output_automaton_dne, key)));
  }

  void simple_scheduler::input_automaton_dne (const aid_t automaton,
					      void* const key) {
    schedule_sysq (make_action_runnable (make_action (automaton_handle<automaton_interface> (automaton), &automaton_interface::sys_input_automaton_dne, key)));
  }
  
  void simple_scheduler::binding_exists (const aid_t automaton,
					 void* const key) {
    schedule_sysq (make_action_runnable (make_action (automaton_handle<automaton_interface> (automaton), &automaton_interface::sys_binding_exists, key)));
  }
  
  void simple_scheduler::input_action_unavailable (const aid_t automaton,
						   void* const key) {
    schedule_sysq (make_action_runnable (make_action (automaton_handle<automaton_interface> (automaton), &automaton_interface::sys_input_action_unavailable, key)));
  }
  
  void simple_scheduler::output_action_unavailable (const aid_t automaton,
						    void* const key) {
    schedule_sysq (make_action_runnable (make_action (automaton_handle<automaton_interface> (automaton), &automaton_interface::sys_output_action_unavailable, key)));
  }
    
  void simple_scheduler::bound (const aid_t automaton,
				void* const key) {
    schedule_sysq (make_action_runnable (make_action (automaton_handle<automaton_interface> (automaton), &automaton_interface::sys_bound, key)));
  }

  void simple_scheduler::output_bound (const output_executor_interface& exec) {
    schedule_sysq (new output_bound_runnable (exec));
    // Schedule the output.
    schedule_execq (new output_exec_runnable (exec));
  }

  void simple_scheduler::input_bound (const input_executor_interface& exec) {
    schedule_sysq (new input_bound_runnable (exec));
  }

  void simple_scheduler::bind_key_dne (const aid_t automaton,
				       void* const key) {
    schedule_sysq (make_action_runnable (make_action (automaton_handle<automaton_interface> (automaton), &automaton_interface::sys_bind_key_dne, key)));
  }
  
  void simple_scheduler::unbound (const aid_t automaton,
				  void* const key) {
    schedule_sysq (make_action_runnable (make_action (automaton_handle<automaton_interface> (automaton), &automaton_interface::sys_unbound, key)));
  }

  void simple_scheduler::output_unbound (const output_executor_interface& exec) {
    schedule_sysq (new output_unbound_runnable (exec));
  }

  void simple_scheduler::input_unbound (const input_executor_interface& exec) {
    schedule_sysq (new input_unbound_runnable (exec));
  }

  void simple_scheduler::create_key_dne (const aid_t automaton,
					 void* const key) {
    schedule_sysq (make_action_runnable (make_action (automaton_handle<automaton_interface> (automaton), &automaton_interface::sys_create_key_dne, key)));
  }
  
  void simple_scheduler::automaton_destroyed (const aid_t automaton,
					      void* const key) {
    schedule_sysq (make_action_runnable (make_action (automaton_handle<automaton_interface> (automaton), &automaton_interface::sys_automaton_destroyed, key)));
  }
  
  void simple_scheduler::recipient_dne (const aid_t automaton,
					void* const key) {
    schedule_sysq (make_action_runnable (make_action (automaton_handle<automaton_interface> (automaton), &automaton_interface::sys_recipient_dne, key)));
  }

  void simple_scheduler::event_delivered (const aid_t automaton,
					  void* const key) {
    schedule_sysq (make_action_runnable (make_action (automaton_handle<automaton_interface> (automaton), &automaton_interface::sys_event_delivered, key)));
  }

  // Implement the system scheduler.
  // These don't belong in a *.cpp file because each scheduler defines them differently.
  // void system_scheduler::set_current_aid (const aid_t aid) {
  //   simple_scheduler::set_current_aid (aid);
  // }
  
  // void system_scheduler::clear_current_aid () {
  //   simple_scheduler::clear_current_aid ();
  // }
  
  // void system_scheduler::create (const aid_t automaton,
  // 				 shared_ptr<generator_interface> generator,
  // 				 void* const key) {
  //   simple_scheduler::create (automaton, generator, key);
  // }

  // void system_scheduler::bind (const aid_t automaton,
  // 			       shared_ptr<bind_executor_interface> exec,
  // 			       void* const key) {
  //   simple_scheduler::bind (automaton, exec, key);
  // }

  // void system_scheduler::unbind (const aid_t automaton,
  // 				 void* const key) {
  //   simple_scheduler::unbind (automaton, key);
  // }
  
  // void system_scheduler::destroy (const aid_t automaton,
  // 				  void* const key) {
  //   simple_scheduler::destroy (automaton, key);
  // }

  // void system_scheduler::create_key_exists (const aid_t automaton,
  // 					    void* const key) {
  //   simple_scheduler::create_key_exists (automaton, key);
  // }
  
  // void system_scheduler::instance_exists (const aid_t automaton,
  // 					  void* const key) {
  //   simple_scheduler::instance_exists (automaton, key);
  // }
  
  // void system_scheduler::automaton_created (const aid_t automaton,
  // 					    void* const key,
  // 					    const aid_t child) {
  //   simple_scheduler::automaton_created (automaton, key, child);
  // }
  
  // void system_scheduler::bind_key_exists (const aid_t automaton,
  // 					  void* const key) {
  //   simple_scheduler::bind_key_exists (automaton, key);
  // }
  
  // void system_scheduler::output_automaton_dne (const aid_t automaton,
  // 					       void* const key) {
  //   simple_scheduler::output_automaton_dne (automaton, key);
  // }
  
  // void system_scheduler::input_automaton_dne (const aid_t automaton,
  // 					      void* const key) {
  //   simple_scheduler::input_automaton_dne (automaton, key);
  // }
  
  // void system_scheduler::binding_exists (const aid_t automaton,
  // 					 void* const key) {
  //   simple_scheduler::binding_exists (automaton, key);
  // }
  
  // void system_scheduler::input_action_unavailable (const aid_t automaton,
  // 						   void* const key) {
  //   simple_scheduler::input_action_unavailable (automaton, key);
  // }
  
  // void system_scheduler::output_action_unavailable (const aid_t automaton,
  // 						    void* const key) {
  //   simple_scheduler::output_action_unavailable (automaton, key);
  // }

  // void system_scheduler::bound (const aid_t automaton,
  // 			      void* const key) {
  //   simple_scheduler::bound (automaton, key);
  // }

  // void system_scheduler::output_bound (const output_executor_interface& exec) {
  //   simple_scheduler::output_bound (exec);
  // }

  // void system_scheduler::input_bound (const input_executor_interface& exec) {
  //   simple_scheduler::input_bound (exec);
  // }
  
  // void system_scheduler::bind_key_dne (const aid_t automaton,
  // 				       void* const key) {
  //   simple_scheduler::bind_key_dne (automaton, key);
  // }
  
  // void system_scheduler::unbound (const aid_t automaton,
  // 				  void* const key) {
  //   simple_scheduler::unbound (automaton, key);
  // }

  // void system_scheduler::output_unbound (const output_executor_interface& exec) {
  //   simple_scheduler::output_unbound (exec);
  // }

  // void system_scheduler::input_unbound (const input_executor_interface& exec) {
  //   simple_scheduler::input_unbound (exec);
  // }
  
  // void system_scheduler::create_key_dne (const aid_t automaton,
  // 					 void* const key) {
  //   simple_scheduler::create_key_dne (automaton, key);
  // }
  
  // void system_scheduler::automaton_destroyed (const aid_t automaton,
  // 					      void* const key) {
  //   simple_scheduler::automaton_destroyed (automaton, key);
  // }
  
  // void system_scheduler::recipient_dne (const aid_t automaton,
  // 					void* const key) {
  //   simple_scheduler::recipient_dne (automaton, key);
  // }
  
  // void system_scheduler::event_delivered (const aid_t automaton,
  // 					  void* const key) {
  //   simple_scheduler::event_delivered (automaton, key);
  // }
  
  // aid_t scheduler::get_current_aid () {
  //   return simple_scheduler::get_current_aid ();
  // }
  
  // void scheduler::run (shared_ptr<generator_interface> generator) {
  //   simple_scheduler::run (generator);
  // }
  
}
