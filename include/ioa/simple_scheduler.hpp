#ifndef __simple_scheduler_hpp__
#define __simple_scheduler_hpp__

#include <ioa/scheduler.hpp>

#include <ioa/create_runnable.hpp>
#include <ioa/bind_runnable.hpp>
#include <ioa/unbind_runnable.hpp>
#include <ioa/destroy_runnable.hpp>
#include <ioa/action_runnable.hpp>

#include <ioa/blocking_list.hpp>
#include <ioa/thread.hpp>
#include <ioa/thread_key.hpp>
#include <queue>
#include <fcntl.h>
#include <sys/select.h>
#include <unistd.h>

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
    static thread_key<const automaton_interface*> m_current_this;

    static bool keep_going () {
      // The criteria for continuing is simple: a runnable exists.
      return runnable_interface::count () != 0;
    }

    static bool thread_keep_going () {
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

    static void schedule_sysq (runnable_interface* r) {
      m_sysq.push (std::make_pair (true, r));
    }

    static void process_sysq () {
      clear_current_aid ();
      while (thread_keep_going ()) {
	std::pair<bool, runnable_interface*> r = m_sysq.pop ();
	if (r.first) {
	  (*r.second) ();
	  delete r.second;
	}
      }
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

    static void schedule_execq (action_runnable_interface* r) {
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

    static void process_execq () {
      clear_current_aid ();
      while (thread_keep_going ()) {
	std::pair<bool, runnable_interface*> r = m_execq.pop ();
	if (r.first) {
	  (*r.second) ();
	  delete r.second;
	}
      }
    }

    static void wakeup_timer_thread () {
      char c;
      ssize_t bytes_written = write (m_wakeup_fd[1], &c, 1);
      assert (bytes_written == 1);
    }

    static void schedule_timerq (action_runnable_interface* r, const time& offset) {
      struct timeval now;
      int s = gettimeofday (&now, 0);
      assert (s == 0);
      time release_time (now);
      release_time += offset;

      // TODO:  Only wake thread if the queue goes from empty to non-empty.
      m_timerq.push (std::make_pair (r, release_time));
      wakeup_timer_thread ();
    }

    static void process_timerq () {
      clear_current_aid ();

      int r;

      std::priority_queue<action_time,
			  std::vector<action_time>,
			  std::greater<action_time> > timer_queue;
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

  public:

    static void set_current_aid (const aid_t aid) {
      // This is to be used during generation so that any allocated memory can be associated with the automaton.
      assert (aid != -1);
      m_current_aid.set (aid);
      m_current_this.set (0);
    }

    static void set_current_aid (const aid_t aid,
				 const automaton_interface& current_this) {
      // This is for all cases except generation.
      assert (aid != -1);
      assert (&current_this != 0);
      m_current_aid.set (aid);
      m_current_this.set (&current_this);
    }

    static void clear_current_aid () {
      m_current_aid.set (-1);
      m_current_this.set (0);
    }

    template <class I>
    static automaton_handle<I> get_current_aid (const I* ptr) {
      // The system uses set_current_aid to alert the scheduler that the code that is executing belongs to the given automaton.
      // When the automaton invokes the scheduler, this ID is used in the production of the corresponding runnable.
      // To be type-safe, we require an automaton_handle<T> instead of an aid_t.
      // This function (get_current_aid) is responsible for producing the handle.

      // First, we check that set_current_aid was called.
      assert (m_current_aid.get () != -1);
      assert (m_current_this.get () != 0);

      // Second, we need to make sure that the user didn't inappropriately cast "this."
      const I* tmp = dynamic_cast<const I*> (m_current_this.get ());
      assert (tmp == ptr);

      return system::cast_aid (ptr, m_current_aid.get ());
    }

    template <class C, class I, class D>
    static void create (const C* ptr,
			std::auto_ptr<generator_interface<I> > generator,
			D& d) {
      schedule_sysq (make_create_runnable (get_current_aid (ptr), generator, d));
    }

    template <class C, class OI, class OM, class II, class IM, class D>
    static void bind (const C* ptr,
		      const action<OI, OM>& output_action,
		      const action<II, IM>& input_action,
		      D& d) {
      schedule_sysq (make_bind_runnable (output_action, input_action, get_current_aid (ptr), d));
    }

    template <class C, class D>
    static void unbind (const C* ptr,
			const bid_t bid,
			D& d) {
      schedule_sysq (make_unbind_runnable (bid, get_current_aid (ptr), d));
    }

    template <class C, class I, class D>
    static void destroy (const C* ptr,
			 const automaton_handle<I>& automaton,
			 D& d) {
      schedule_sysq (make_destroy_runnable (get_current_aid (ptr), automaton, d));
    }

    template <class I, class M>
    static void schedule (const I* ptr,
  			  M I::*member_ptr) {
      schedule_execq (make_action_runnable (make_action (get_current_aid (ptr), member_ptr)));
    }

    template <class I, class M>
    static void schedule (const I* ptr,
                          M I::*member_ptr,
                          const typename M::parameter_type & param) {
      schedule_execq (make_action_runnable (make_action (get_current_aid (ptr), member_ptr, param)));
    }

    template <class I, class M>
    static void schedule (const I* ptr,
  			  M I::*member_ptr,
  			  time offset) {
      schedule_timerq (make_action_runnable (make_action (get_current_aid (ptr), member_ptr)), offset);
    }

    template <class I>
    static void run (std::auto_ptr<generator_interface<I> > generator) {
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

  };

  // Implement the system scheduler.
  // These don't belong in a *.cpp file because each scheduler defines them differently.
  void system_scheduler::set_current_aid (const aid_t aid) {
    simple_scheduler::set_current_aid (aid);
  }

  void system_scheduler::set_current_aid (const aid_t aid,
					  const automaton_interface& current_this) {
    simple_scheduler::set_current_aid (aid, current_this);
  }

  void system_scheduler::clear_current_aid () {
    simple_scheduler::clear_current_aid ();
  }

  // Implement the scheduler.

  template <class I>
  automaton_handle<I> scheduler::get_current_aid (const I* ptr) {
    return simple_scheduler::get_current_aid (ptr);
  }

  template <class C, class I, class D>
  void scheduler::create (const C* ptr,
			  std::auto_ptr<generator_interface<I> > generator,
			  D& d) {
    simple_scheduler::create (ptr, generator, d);
  }

  template <class C, class OI, class OM, class II, class IM, class D>
  void scheduler::bind (const C* ptr,
			const action<OI, OM>& output_action,
			const action<II, IM>& input_action,
			D& d) {
    simple_scheduler::bind (ptr, output_action, input_action, d);
  }

  template <class C, class D>
  void scheduler::unbind (const C* ptr,
			  const bid_t bid,
			  D& d) {
    simple_scheduler::unbind (ptr, bid, d);
  }

  template <class C, class I, class D>
  void scheduler::destroy (const C* ptr,
			   const automaton_handle<I>& automaton,
			   D& d) {
    simple_scheduler::destroy (ptr, automaton, d);
  }

  template <class I, class M>
  void scheduler::schedule (const I* ptr,
			    M I::*member_ptr) {
    simple_scheduler::schedule (ptr, member_ptr);
  }

  template <class I, class M>
  void scheduler::schedule (const I* ptr,
			    M I::*member_ptr,
          const typename M::parameter_type & p) {
    simple_scheduler::schedule (ptr, member_ptr, p);
  }

  template <class I, class M>
  void scheduler::schedule (const I* ptr,
			    M I::*member_ptr,
			    time offset) {
    simple_scheduler::schedule (ptr, member_ptr, offset);
  }

  template <class I>
  void scheduler::run (std::auto_ptr<generator_interface<I> > generator) {
    simple_scheduler::run (generator);
  }

  // TODO:  EVENTS!!!
  // TODO:  What happens when we send an event to a destroyed automaton?

  // class simple_scheduler
  // {
  //   void clear (void) {
  //     // We clear the system first because it might add something to a run queue.
  //     system::clear ();

  //     // Then, we clear the run queues.
  //     for (std::list<std::pair<bool, runnable_interface*> >::iterator pos = m_sysq.list.begin ();
  // 	   pos != m_sysq.list.end ();
  // 	   ++pos) {
  // 	delete pos->second;
  //     }
  //     m_sysq.list.clear ();

  //     for (std::list<std::pair<bool, action_runnable_interface*> >::iterator pos = m_execq.list.begin ();
  // 	   pos != m_execq.list.end ();
  // 	   ++pos) {
  // 	delete pos->second;
  //     }
  //     m_execq.list.clear ();

  //     // Notice that the post-conditions of clear () match those of run ().
  //     assert (m_sysq.list.size () == 0);
  //     assert (m_execq.list.size () == 0);
  //     assert (!keep_going ());
  //   }

  // };

}

#endif
