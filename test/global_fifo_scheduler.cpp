#include "minunit.h"

#include <ioa/global_fifo_scheduler.hpp>
#include <ioa/action_wrapper.hpp>
#include "../lib/null_shared_mutex.hpp"
#include "../lib/null_shared_lock.hpp"
#include "../lib/null_mutex.hpp"

#include <iostream>
#include <fcntl.h>

static const char*
get_set_current_aid ()
{
  std::cout << __func__ << std::endl;

  ioa::global_fifo_scheduler sched;

  sched.set_current_aid (10);
  mu_assert (sched.get_current_aid () == 10);
  sched.clear_current_aid ();

  return 0;
}

class test_automaton1 :
  public ioa::automaton_base
{
public:
  bool state;

  bool action_precondition () const {
    return true;
  }

  void action_effect () {
    state = true;
  }

  void action_schedule () const { }
  
  UP_INTERNAL (test_automaton1, action);
};

static const char*
schedule ()
{
  std::cout << __func__ << std::endl;

  ioa::global_fifo_scheduler sched;
  ioa::null_shared_mutex shared_mutex;
  ioa::automaton_set automaton_set;
  test_automaton1 instance;
  ioa::null_mutex mutex;
  ioa::automaton_handle<test_automaton1> handle (automaton_set.create (&instance));

  sched.schedule (new ioa::action_runnable<test_automaton1, test_automaton1::action_type, ioa::null_shared_lock> (sched, shared_mutex, automaton_set, instance, mutex, handle, &test_automaton1::action));
  sched.run ();

  mu_assert (instance.state);

  return 0;
}

class test_automaton2 :
  public ioa::automaton_base
{
public:
  ioa::time t;

  bool action_precondition () const {
    return true;
  }

  void action_effect () {
    t = ioa::time::now ();
  }

  void action_schedule () const { }
  
  UP_INTERNAL (test_automaton2, action);
};

static const char*
schedule_after ()
{
  std::cout << __func__ << " (this should take about 10 seconds)" << std::endl;

  ioa::global_fifo_scheduler sched;
  ioa::null_shared_mutex shared_mutex;
  ioa::automaton_set automaton_set;
  test_automaton2 instance;
  ioa::null_mutex mutex;
  ioa::automaton_handle<test_automaton2> handle (automaton_set.create (&instance));

  ioa::time t1 = ioa::time::now () + ioa::time (10, 0);

  sched.schedule_after (new ioa::action_runnable<test_automaton2, test_automaton2::action_type, ioa::null_shared_lock> (sched, shared_mutex, automaton_set, instance, mutex, handle, &test_automaton2::action), ioa::time (10,0));
  sched.run ();

  mu_assert (instance.t - t1 < ioa::time (3,0));

  return 0;
}

static const char*
schedule_read_ready ()
{
  std::cout << __func__ << std::endl;

  ioa::global_fifo_scheduler sched;
  ioa::null_shared_mutex shared_mutex;
  ioa::automaton_set automaton_set;
  test_automaton1 instance;
  ioa::null_mutex mutex;
  ioa::automaton_handle<test_automaton1> handle (automaton_set.create (&instance));

  int fds[2];
  mu_assert (pipe (fds) == 0);
  mu_assert (fcntl (fds[0], F_SETFL, O_NONBLOCK) == 0);
  mu_assert (fcntl (fds[1], F_SETFL, O_NONBLOCK) == 0);

  char c = 'A';
  mu_assert (write (fds[1], &c, 1) == 1);

  sched.schedule_read_ready (new ioa::action_runnable<test_automaton1, test_automaton1::action_type, ioa::null_shared_lock> (sched, shared_mutex, automaton_set, instance, mutex, handle, &test_automaton1::action), fds[0]);
  sched.run ();

  mu_assert (instance.state);

  close (fds[0]);
  close (fds[1]);

  return 0;
}

static const char*
schedule_write_ready ()
{
  std::cout << __func__ << std::endl;

  ioa::global_fifo_scheduler sched;
  ioa::null_shared_mutex shared_mutex;
  ioa::automaton_set automaton_set;
  test_automaton1 instance;
  ioa::null_mutex mutex;
  ioa::automaton_handle<test_automaton1> handle (automaton_set.create (&instance));

  int fds[2];
  mu_assert (pipe (fds) == 0);
  mu_assert (fcntl (fds[0], F_SETFL, O_NONBLOCK) == 0);
  mu_assert (fcntl (fds[1], F_SETFL, O_NONBLOCK) == 0);

  sched.schedule_write_ready (new ioa::action_runnable<test_automaton1, test_automaton1::action_type, ioa::null_shared_lock> (sched, shared_mutex, automaton_set, instance, mutex, handle, &test_automaton1::action), fds[1]);
  sched.run ();

  mu_assert (instance.state);

  close (fds[0]);
  close (fds[1]);

  return 0;
}

static const char*
close_fd ()
{
  std::cout << __func__ << std::endl;

  ioa::global_fifo_scheduler sched;
  ioa::null_shared_mutex shared_mutex;
  ioa::automaton_set automaton_set;
  test_automaton1 instance;
  ioa::null_mutex mutex;
  ioa::automaton_handle<test_automaton1> handle (automaton_set.create (&instance));

  int fds[2];
  mu_assert (pipe (fds) == 0);
  mu_assert (fcntl (fds[0], F_SETFL, O_NONBLOCK) == 0);
  mu_assert (fcntl (fds[1], F_SETFL, O_NONBLOCK) == 0);

  sched.schedule_read_ready (new ioa::action_runnable<test_automaton1, test_automaton1::action_type, ioa::null_shared_lock> (sched, shared_mutex, automaton_set, instance, mutex, handle, &test_automaton1::action), fds[0]);
  sched.close (fds[0]);
  sched.run ();

  mu_assert (instance.state);

  close (fds[1]);

  return 0;
}

const char*
all_tests ()
{
  mu_run_test (get_set_current_aid);
  mu_run_test (schedule);
  mu_run_test (schedule_after);
  mu_run_test (schedule_read_ready);
  mu_run_test (schedule_write_ready);
  mu_run_test (close_fd);

  return 0;
}
