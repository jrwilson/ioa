#include <ioa/simple_scheduler.hpp>

namespace ioa {

  blocking_list<std::pair<bool, runnable_interface*> > simple_scheduler::m_sysq;
  blocking_list<std::pair<bool, action_runnable_interface*> > simple_scheduler::m_execq;
  int simple_scheduler::m_wakeup_fd[2];
  blocking_list<action_time> simple_scheduler::m_timerq;
  thread_key<aid_t> simple_scheduler::m_current_aid;

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
      wakeup_timer_thread ();
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
	(*r.second) ();
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
	(*r.second) ();
	delete r.second;
      }
    }
  }

  void simple_scheduler::wakeup_timer_thread () {
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
    
    // TODO:  Only wake thread if the queue goes from empty to non-empty.
    m_timerq.push (std::make_pair (r, release_time));
    wakeup_timer_thread ();
  }
  
  void simple_scheduler::process_timerq () {
    clear_current_aid ();
    
    int r;
    
    std::priority_queue<action_time, std::vector<action_time>, std::greater<action_time> > timer_queue;
    fd_set read_set;
    
    while (thread_keep_going ()) {
      // Process registrations.
      {
	lock lock (m_timerq.list_mutex);
	while (!m_timerq.list.empty ()) {
	  timer_queue.push (m_timerq.list.front ());
	  m_timerq.list.pop_front ();
	}
      }
      
      struct timeval n;
      r = gettimeofday (&n, 0);
      assert (r == 0);
      time now (n);
      
      while (!timer_queue.empty () && timer_queue.top ().second < now) {
	schedule_execq (timer_queue.top ().first);
	timer_queue.pop ();
      }
      
      struct timeval timeout;
      struct timeval* test_timeout = 0;
      
      if (!timer_queue.empty ()) {
	timeout = timer_queue.top ().second - now;
	test_timeout = &timeout;
      }
      
      // TODO: Do better than FD_SETSIZE.
      // TODO: Don't ZERO every time.
      FD_ZERO (&read_set);
      FD_SET (m_wakeup_fd[0], &read_set);
      int r = select (FD_SETSIZE, &read_set, 0, 0, test_timeout);
      if (r > 0) {
	if (FD_ISSET (m_wakeup_fd[0], &read_set)) {
	  // Drain the wakeup pipe.
	  char c;
	  // TODO:  Do better than reading one character at a time.
	  while (read (m_wakeup_fd[0], &c, 1) == 1) { }
	}
      }
      else if (r < 0) {
	assert (false);
      }
    }
  }
    
  aid_t simple_scheduler::get_current_aid () {
    aid_t retval = m_current_aid.get ();
    assert (retval != -1);
    return retval;
  }
  
  void simple_scheduler::run (shared_ptr<generator_interface> generator) {
    int r;
    
    assert (m_sysq.list.size () == 0);
    assert (m_execq.list.size () == 0);
    assert (!keep_going ());
    system::create (generator);
    
    // Create a pipe to communicate with the timer thread.
    r = pipe (m_wakeup_fd);
    assert (r == 0);
    r = fcntl (m_wakeup_fd[0], F_SETFL, O_NONBLOCK);
    assert (r == 0);
    r = fcntl (m_wakeup_fd[1], F_SETFL, O_NONBLOCK);
    assert (r == 0);
    
    thread sysq_thread (&simple_scheduler::process_sysq);
    thread execq_thread (&simple_scheduler::process_execq);
    thread timerq_thread (&simple_scheduler::process_timerq);
    
    sysq_thread.join ();
    execq_thread.join ();
    timerq_thread.join ();
    
    // TODO:  Do I need to close both ends?
    close (m_wakeup_fd[0]);
    close (m_wakeup_fd[1]);
  }
  
  void simple_scheduler::clear (void) {
    // We clear the system first because it might add something to a run queue.
    system::clear ();
    
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
    
    // Notice that the post-conditions of clear () match those of run ().
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
    schedule_execq (make_action_runnable (make_action (automaton_handle<automaton_interface> (automaton), &automaton_interface::sys_create_key_exists, key)));
  }

  void simple_scheduler::instance_exists (const aid_t automaton,
					  void* const key) {
    schedule_execq (make_action_runnable (make_action (automaton_handle<automaton_interface> (automaton), &automaton_interface::sys_instance_exists, key)));
  }
  
  void simple_scheduler::automaton_created (const aid_t automaton,
					    void* const key,
					    const aid_t child) {
    schedule_execq (make_action_runnable (make_action (automaton_handle<automaton_interface> (automaton), &automaton_interface::sys_automaton_created, std::make_pair (key, child))));
  }
  
  void simple_scheduler::bind_key_exists (const aid_t automaton,
					  void* const key) {
    schedule_execq (make_action_runnable (make_action (automaton_handle<automaton_interface> (automaton), &automaton_interface::sys_bind_key_exists, key)));
  }

  void simple_scheduler::output_automaton_dne (const aid_t automaton,
					       void* const key) {
    schedule_execq (make_action_runnable (make_action (automaton_handle<automaton_interface> (automaton), &automaton_interface::sys_output_automaton_dne, key)));
  }

  void simple_scheduler::input_automaton_dne (const aid_t automaton,
					      void* const key) {
    schedule_execq (make_action_runnable (make_action (automaton_handle<automaton_interface> (automaton), &automaton_interface::sys_input_automaton_dne, key)));
  }
  
  void simple_scheduler::binding_exists (const aid_t automaton,
					 void* const key) {
    schedule_execq (make_action_runnable (make_action (automaton_handle<automaton_interface> (automaton), &automaton_interface::sys_binding_exists, key)));
  }
  
  void simple_scheduler::input_action_unavailable (const aid_t automaton,
						   void* const key) {
    schedule_execq (make_action_runnable (make_action (automaton_handle<automaton_interface> (automaton), &automaton_interface::sys_input_action_unavailable, key)));
  }
  
  void simple_scheduler::output_action_unavailable (const aid_t automaton,
						    void* const key) {
    schedule_execq (make_action_runnable (make_action (automaton_handle<automaton_interface> (automaton), &automaton_interface::sys_output_action_unavailable, key)));
  }
    
  void simple_scheduler::bound (const aid_t automaton,
				void* const key) {
    schedule_execq (make_action_runnable (make_action (automaton_handle<automaton_interface> (automaton), &automaton_interface::sys_bound, key)));
  }

  void simple_scheduler::bind_key_dne (const aid_t automaton,
				       void* const key) {
    schedule_execq (make_action_runnable (make_action (automaton_handle<automaton_interface> (automaton), &automaton_interface::sys_bind_key_dne, key)));
  }
  
  void simple_scheduler::unbound (const aid_t automaton,
				  void* const key) {
    schedule_execq (make_action_runnable (make_action (automaton_handle<automaton_interface> (automaton), &automaton_interface::sys_unbound, key)));
  }

  void simple_scheduler::create_key_dne (const aid_t automaton,
					 void* const key) {
    schedule_execq (make_action_runnable (make_action (automaton_handle<automaton_interface> (automaton), &automaton_interface::sys_create_key_dne, key)));
  }
  
  void simple_scheduler::automaton_destroyed (const aid_t automaton,
					      void* const key) {
    schedule_execq (make_action_runnable (make_action (automaton_handle<automaton_interface> (automaton), &automaton_interface::sys_automaton_destroyed, key)));
  }
  
  void simple_scheduler::recipient_dne (const aid_t automaton,
					void* const key) {
    schedule_execq (make_action_runnable (make_action (automaton_handle<automaton_interface> (automaton), &automaton_interface::sys_recipient_dne, key)));
  }

  void simple_scheduler::event_delivered (const aid_t automaton,
					  void* const key) {
    schedule_execq (make_action_runnable (make_action (automaton_handle<automaton_interface> (automaton), &automaton_interface::sys_event_delivered, key)));
  }

  // Implement the system scheduler.
  // These don't belong in a *.cpp file because each scheduler defines them differently.
  void system_scheduler::set_current_aid (const aid_t aid) {
    simple_scheduler::set_current_aid (aid);
  }
  
  void system_scheduler::clear_current_aid () {
    simple_scheduler::clear_current_aid ();
  }
  
  void system_scheduler::create (const aid_t automaton,
				 shared_ptr<generator_interface> generator,
				 void* const key) {
    simple_scheduler::create (automaton, generator, key);
  }

  void system_scheduler::bind (const aid_t automaton,
			       shared_ptr<bind_executor_interface> exec,
			       void* const key) {
    simple_scheduler::bind (automaton, exec, key);
  }

  void system_scheduler::unbind (const aid_t automaton,
				 void* const key) {
    simple_scheduler::unbind (automaton, key);
  }
  
  void system_scheduler::destroy (const aid_t automaton,
				  void* const key) {
    simple_scheduler::destroy (automaton, key);
  }

  void system_scheduler::create_key_exists (const aid_t automaton,
					    void* const key) {
    simple_scheduler::create_key_exists (automaton, key);
  }
  
  void system_scheduler::instance_exists (const aid_t automaton,
					  void* const key) {
    simple_scheduler::instance_exists (automaton, key);
  }
  
  void system_scheduler::automaton_created (const aid_t automaton,
					    void* const key,
					    const aid_t child) {
    simple_scheduler::automaton_created (automaton, key, child);
  }
  
  void system_scheduler::bind_key_exists (const aid_t automaton,
					  void* const key) {
    simple_scheduler::bind_key_exists (automaton, key);
  }
  
  void system_scheduler::output_automaton_dne (const aid_t automaton,
					       void* const key) {
    simple_scheduler::output_automaton_dne (automaton, key);
  }
  
  void system_scheduler::input_automaton_dne (const aid_t automaton,
					      void* const key) {
    simple_scheduler::input_automaton_dne (automaton, key);
  }
  
  void system_scheduler::binding_exists (const aid_t automaton,
					 void* const key) {
    simple_scheduler::binding_exists (automaton, key);
  }
  
  void system_scheduler::input_action_unavailable (const aid_t automaton,
						   void* const key) {
    simple_scheduler::input_action_unavailable (automaton, key);
  }
  
  void system_scheduler::output_action_unavailable (const aid_t automaton,
						    void* const key) {
    simple_scheduler::output_action_unavailable (automaton, key);
  }

  void system_scheduler::bound (const aid_t automaton,
			      void* const key) {
    simple_scheduler::bound (automaton, key);
  }
  
  void system_scheduler::bind_key_dne (const aid_t automaton,
				       void* const key) {
    simple_scheduler::bind_key_dne (automaton, key);
  }
  
  void system_scheduler::unbound (const aid_t automaton,
				  void* const key) {
    simple_scheduler::unbound (automaton, key);
  }
  
  void system_scheduler::create_key_dne (const aid_t automaton,
					 void* const key) {
    simple_scheduler::create_key_dne (automaton, key);
  }
  
  void system_scheduler::automaton_destroyed (const aid_t automaton,
					      void* const key) {
    simple_scheduler::automaton_destroyed (automaton, key);
  }
  
  void system_scheduler::recipient_dne (const aid_t automaton,
					void* const key) {
    simple_scheduler::recipient_dne (automaton, key);
  }
  
  void system_scheduler::event_delivered (const aid_t automaton,
					  void* const key) {
    simple_scheduler::event_delivered (automaton, key);
  }
  
  aid_t scheduler::get_current_aid () {
    return simple_scheduler::get_current_aid ();
  }
  
  void scheduler::run (shared_ptr<generator_interface> generator) {
    simple_scheduler::run (generator);
  }
  
  void scheduler::clear () {
    simple_scheduler::clear ();
  }
  
}
