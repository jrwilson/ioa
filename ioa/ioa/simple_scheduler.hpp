#ifndef __simple_scheduler_hpp__
#define __simple_scheduler_hpp__

#include "blocking_queue.hpp"
#include "runnable.hpp"
#include "system.hpp"
#include "thread.hpp"
#include <queue>
#include <functional>
#include <fcntl.h>

namespace ioa {

  // TODO:  EVENTS!!!
  // TODO:  What happens when we send an event to a destroyed automaton?

  template <class C, class I, class D>
  class create_runnable :
    public runnable_interface
  {
  private:
    system& m_system;
    const automaton_handle<C> m_automaton;
    std::auto_ptr<generator_interface<I> > m_generator;
    scheduler_interface& m_scheduler;
    D& m_d;
  public:
    create_runnable (system& system,
		     const automaton_handle<C>& automaton,
		     std::auto_ptr<generator_interface<I> > generator,
		     scheduler_interface& scheduler,
		     D& d) :
      m_system (system),
      m_automaton (automaton),
      m_generator (generator),
      m_scheduler (scheduler),
      m_d (d)
    { }

    void operator() () {
      m_system.create (m_automaton, m_generator, m_scheduler, m_d);
    }
  };
  
  template <class C, class I, class D>
  create_runnable<C, I, D>* make_create_runnable (system& system,
						  const automaton_handle<C>& automaton,
						  std::auto_ptr<generator_interface<I> > generator,
						  scheduler_interface& scheduler,
						  D& d) {
    return new create_runnable<C, I, D> (system, automaton, generator, scheduler, d);
  }

  template <class OI, class OM, class II, class IM, class C, class D>
  class bind_runnable :
    public runnable_interface
  {
  private:
    system& m_system;
    const action<OI, OM> m_output_action;
    const action<II, IM> m_input_action;
    const automaton_handle<C> m_automaton;
    scheduler_interface& m_scheduler;
    D& m_d;

  public:
    bind_runnable (system& system,
		   const action<OI, OM> output_action,
		   const action<II, IM> input_action,
		   const automaton_handle<C>& automaton,
		   scheduler_interface& scheduler,
		   D& d) :
      m_system (system),
      m_output_action (output_action),
      m_input_action (input_action),
      m_automaton (automaton),
      m_scheduler (scheduler),
      m_d (d)
    { }

    void operator() () {
      m_system.bind (m_output_action, m_input_action, m_automaton, m_scheduler, m_d);
    }
  };

  template <class OI, class OM, class II, class IM, class C, class D>
  bind_runnable<OI, OM, II, IM, C, D>* make_bind_runnable (system& system,
							   const action<OI, OM> output_action,
							   const action<II, IM> input_action,
							   const automaton_handle<C>& automaton,
							   scheduler_interface& scheduler,
							   D& d) {
    return new bind_runnable<OI, OM, II, IM, C, D> (system, output_action, input_action, automaton, scheduler, d);
  }

  template <class C, class D>
  class unbind_runnable :
    public runnable_interface
  {
  private:
    system& m_system;
    const bid_t m_bid;
    const automaton_handle<C> m_automaton;
    scheduler_interface& m_scheduler;
    D& m_d;
    
  public:
    unbind_runnable (system& system,
		     const bid_t bid,
		     const automaton_handle<C>& automaton,
		     scheduler_interface& scheduler,
		     D& d) :
      m_system (system),
      m_bid (bid),
      m_automaton (automaton),
      m_scheduler (scheduler),
      m_d (d)
    { }

    void operator() () {
      m_system.unbind (m_bid, m_automaton, m_scheduler, m_d);
    }
  };
  
  template <class C, class D>
  unbind_runnable<C, D>* make_unbind_runnable (system& system,
					       const bid_t bid,
					       const automaton_handle<C>& automaton,
					       scheduler_interface& scheduler,
					       D& d) {
    return new unbind_runnable<C, D> (system, bid, automaton, scheduler, d);
  }

  template <class C, class I, class D>
  class destroy_runnable :
    public runnable_interface
  {
  private:
    system& m_system;
    const automaton_handle<C> m_automaton;
    const automaton_handle<I> m_target;
    scheduler_interface& m_scheduler;
    D& m_d;

  public:
    destroy_runnable (system& system,
		      const automaton_handle<C>& automaton,
		      const automaton_handle<I>& target,
		      scheduler_interface& scheduler,
		      D& d) :
      m_system (system),
      m_automaton (automaton),
      m_target (target),
      m_scheduler (scheduler),
      m_d (d)
    { }

    void operator() () {
      m_system.destroy (m_automaton, m_target, m_scheduler, m_d);
    }
  };

  template <class C, class I, class D>
  destroy_runnable<C, I, D>* make_destroy_runnable (system& system,
						    const automaton_handle<C>& automaton,
						    const automaton_handle<I>& target,
						    scheduler_interface& scheduler,
						    D& d) {
    return new destroy_runnable<C, I, D> (system, automaton, target, scheduler, d);
  }

  template <class I, class M>
  class action_runnable :
    public action_runnable_interface
  {
  private:
    system& m_system;
    const action<I, M> m_action;
    scheduler_interface& m_scheduler;

  public:
    action_runnable (system& system,
		     const action<I, M> action,
		     scheduler_interface& scheduler) :
      m_system (system),
      m_action (action),
      m_scheduler (scheduler)
    { }
    
    void operator() () {
      m_system.execute (m_action, m_scheduler);
    }

    const action_interface& get_action () const {
      return m_action;
    }
  };

  template <class I, class M>
  action_runnable<I, M>* make_action_runnable (system& system,
					       const action<I, M> action,
					       scheduler_interface& scheduler) {
    return new action_runnable<I, M> (system, action, scheduler);
  }

  typedef std::pair<action_runnable_interface*, time> action_time;

  bool operator< (const action_time& x,
		  const action_time& y) {
    return x.second < y.second;
  }

  class simple_scheduler :
    public scheduler_interface
  {
  private:
    system m_system;
    blocking_list<std::pair<bool, runnable_interface*> > m_sysq;
    blocking_list<std::pair<bool, action_runnable_interface*> > m_execq;
    int m_wakeup_fd[2];
    blocking_list<action_time> m_timerq;
    aid_t m_current_aid;
    const automaton_interface* m_current_this;

  public:    
    template <class I>
    automaton_handle<I> get_current_aid (const I* ptr) const {
      // The system uses set_current_aid to alert the scheduler that the code that is executing belongs to the given automaton.
      // When the automaton invokes the scheduler, this ID is used in the production of the corresponding runnable.
      // To be type-safe, we require an automaton_handle<T> instead of an aid_t.
      // This function (get_current_aid) is responsible for producing the handle.

      // First, we check that set_current_aid was called.
      assert (m_current_aid != -1);
      assert (m_current_this != 0);

      // Second, we need to make sure that the user didn't inappropriately cast "this."
      const I* tmp = dynamic_cast<const I*> (m_current_this);
      assert (tmp == ptr);

      return m_system.cast_aid (ptr, m_current_aid);
    }

  private:
    void set_current_aid (const aid_t aid) {
      // This is to be used during generation so that any allocated memory can be associated with the automaton.
      assert (aid != -1);
      m_current_aid = aid;
      m_current_this = 0;
    }

    void set_current_aid (const aid_t aid,
			  const automaton_interface& current_this) {
      // This is for all cases except generation.
      assert (aid != -1);
      assert (&current_this != 0);
      m_current_aid = aid;
      m_current_this = &current_this;
    }

    void clear_current_aid () {
      m_current_aid = -1;
      m_current_this = 0;
    }

    bool keep_going () const {
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
	wakeup_timer_thread ();
      }
      return retval;
    }

    void schedule_sysq (runnable_interface* r) {
      m_sysq.push (std::make_pair (true, r));
    }

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
	lock lock (m_execq.get_mutex ());
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

    void wakeup_timer_thread () {
      char c;
      ssize_t bytes_written = write (m_wakeup_fd[1], &c, 1);
      assert (bytes_written == 1);
    }

    void schedule_timerq (action_runnable_interface* r, const time& offset) {
      struct timeval now;
      int s = gettimeofday (&now, 0);
      assert (s == 0);
      time release_time (now);
      release_time += offset;

      // TODO:  Only wake thread if the queue goes from empty to non-empty.
      m_timerq.push (std::make_pair (r, release_time));
      wakeup_timer_thread ();
    }

  public:
    simple_scheduler () :
      m_current_aid (-1),
      m_current_this (0)
    { }

    ~simple_scheduler () {
      clear ();
    }

    template <class C, class I, class D>
    void create (const C* ptr,
		 std::auto_ptr<generator_interface<I> > generator,
		 D& d) {
      schedule_sysq (make_create_runnable (m_system, get_current_aid (ptr), generator, *this, d));
    }
    
    template <class C, class OI, class OM, class II, class IM, class D>
    void bind (const C* ptr,
	       const action<OI, OM>& output_action,
	       const action<II, IM>& input_action,
	       D& d) {
      schedule_sysq (make_bind_runnable (m_system, output_action, input_action, get_current_aid (ptr), *this, d));
    }

    template <class C, class D>
    void unbind (const C* ptr,
		 const bid_t bid,
		 D& d) {
      schedule_sysq (make_unbind_runnable (m_system, bid, get_current_aid (ptr), *this, d));
    }

    template <class C, class I, class D>
    void destroy (const C* ptr,
		  const automaton_handle<I>& automaton,
		  D& d) {
      schedule_sysq (make_destroy_runnable (m_system, get_current_aid (ptr), automaton, *this, d));
    }

    template <class I, class M>
    void schedule (const I* ptr,
		   M I::*member_ptr,
		   const time& offset) {
      action_runnable_interface* r = make_action_runnable (m_system, make_action (get_current_aid (ptr), member_ptr), *this);
      if (offset == time ()) {
	schedule_execq (r);
      }
      else {
	schedule_timerq (r, offset);
      }
    }

  private:

    void process_sysq () {
      while (thread_keep_going ()) {
	std::pair<bool, runnable_interface*> r = m_sysq.pop ();
	if (r.first) {
	  (*r.second) ();
	  delete r.second;
	}
      }
    }
    
    void process_execq () {
      while (thread_keep_going ()) {
	std::pair<bool, runnable_interface*> r = m_execq.pop ();
	if (r.first) {
	  (*r.second) ();
	  delete r.second;
	}
      }
    }

    void process_timerq () {
      int r;

      std::priority_queue<action_time,
			  std::vector<action_time>,
			  std::greater<action_time> > timer_queue;

      fd_set read_set;
      FD_ZERO (&read_set);
      FD_SET (m_wakeup_fd[0], &read_set);

      while (thread_keep_going ()) {
	// Process registrations.
	{
	  lock lock (m_timerq.get_mutex ());
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
	fd_set test_read_set;
	FD_COPY (&read_set, &test_read_set);
	int r = select (FD_SETSIZE, &test_read_set, 0, 0, test_timeout);
	if (r > 0) {
	  if (FD_ISSET (m_wakeup_fd[0], &test_read_set)) {
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

  public:
    template <class I>
    void run (std::auto_ptr<generator_interface<I> > generator) {
      int r;

      assert (m_sysq.list.size () == 0);
      assert (m_execq.list.size () == 0);
      assert (!keep_going ());
      m_system.create (generator, *this);

      // Create a pipe to communicate with the timer thread.
      r = pipe (m_wakeup_fd);
      assert (r == 0);
      r = fcntl (m_wakeup_fd[0], F_SETFL, O_NONBLOCK);
      assert (r == 0);
      r = fcntl (m_wakeup_fd[1], F_SETFL, O_NONBLOCK);
      assert (r == 0);

      thread sysq_thread (&simple_scheduler::process_sysq, *this);
      thread execq_thread (&simple_scheduler::process_execq, *this);
      thread timerq_thread (&simple_scheduler::process_timerq, *this);

      sysq_thread.join ();
      execq_thread.join ();
      timerq_thread.join ();

      // TODO:  Do I need to close both ends?
      close (m_wakeup_fd[0]);
      close (m_wakeup_fd[1]);
    }

    void clear (void) {
      // We clear the system first because it might add something to a run queue.
      m_system.clear ();

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
            
  };

}

#endif
