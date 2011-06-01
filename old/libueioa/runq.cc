#include "runq.hh"

#include <algorithm>

runq::runq (void)
{
  pthread_cond_init (&m_cond, NULL);
  pthread_mutex_init (&m_mutex, NULL);
}

runq::~runq (void)
{
  pthread_cond_destroy (&m_cond);
  pthread_mutex_destroy (&m_mutex);
}

size_t
runq::size (void)
{
  size_t retval;
  pthread_mutex_lock (&m_mutex);
  retval = m_runnables.size ();
  pthread_mutex_unlock (&m_mutex);
  return retval;
}

bool
runq::empty (void)
{
  bool retval;
  pthread_mutex_lock (&m_mutex);
  retval = m_runnables.empty ();
  pthread_mutex_unlock (&m_mutex);
  return retval;
}

void
runq::push (const runnable& runnable)
{
  pthread_mutex_lock (&m_mutex);
  if (std::find (m_runnables.begin (),
		 m_runnables.end (),
		 runnable) == m_runnables.end ()) {
    m_runnables.push_back (runnable);
  }
  pthread_cond_broadcast (&m_cond);
  pthread_mutex_unlock (&m_mutex);
}

void
runq::insert_system_input (aid_t aid)
{
  assert (aid != -1);

  runnable runnable;
  runnable.type = SYSTEM_INPUT;
  runnable.aid = aid;
  runnable.param = NULL;
  push (runnable);
}

void
runq::insert_system_output (aid_t aid)
{
  assert (aid != -1);

  runnable runnable;
  runnable.type = SYSTEM_OUTPUT;
  runnable.aid = aid;
  runnable.param = NULL;

  push (runnable);
}

void
runq::insert_alarm_input (aid_t aid)
{
  assert (aid != -1);

  runnable runnable;
  runnable.type = ALARM_INPUT;
  runnable.aid = aid;
  runnable.param = NULL;
  push (runnable);
}

void
runq::insert_read_input (aid_t aid)
{
  assert (aid != -1);

  runnable runnable;
  runnable.type = READ_INPUT;
  runnable.aid = aid;
  runnable.param = NULL;
  push (runnable);
}

void
runq::insert_write_input (aid_t aid)
{
  assert (aid != -1);

  runnable runnable;
  runnable.type = WRITE_INPUT;
  runnable.aid = aid;
  runnable.param = NULL;
  push (runnable);
}

void
runq::insert_free_input (aid_t caller_aid, aid_t aid, input_t free_input, bid_t bid)
{
  assert (aid != -1);

  runnable runnable;
  runnable.type = FREE_INPUT;
  runnable.aid = aid;
  runnable.param = NULL;
  runnable.free_input.caller_aid = caller_aid;
  runnable.free_input.free_input = free_input;
  runnable.free_input.bid = bid;
  push (runnable);
}

void
runq::insert_output (aid_t aid, output_t output, void* param)
{
  assert (aid != -1);
  assert (output != NULL);

  runnable runnable;
  runnable.type = OUTPUT;
  runnable.aid = aid;
  runnable.param = param;
  runnable.output.output = output;
  push (runnable);
}

void
runq::insert_internal (aid_t aid, internal_t internal, void* param)
{
  assert (aid != -1);
  assert (internal != NULL);

  runnable runnable;
  runnable.type = INTERNAL;
  runnable.aid = aid;
  runnable.param = param;
  runnable.internal.internal = internal;
  push (runnable);
}

runq::runnable
runq::pop (void)
{
  pthread_mutex_lock (&m_mutex);
  while (m_runnables.empty ()) {
    pthread_cond_wait (&m_cond, &m_mutex);
  }
  runnable retval = m_runnables.front ();
  m_runnables.pop_front ();
  pthread_mutex_unlock (&m_mutex);
  return retval;
}

void
runq::purge_aid (aid_t aid)
{
  pthread_mutex_lock (&m_mutex);
  runnable_list::iterator pos = m_runnables.begin ();
  while (pos != m_runnables.end ()) {
    if (pos->aid == aid) {
      pos = m_runnables.erase (pos);
    }
    else {
      ++pos;
    }
  }
  pthread_mutex_unlock (&m_mutex);
}

void
runq::purge_aid_param (aid_t aid, void* param)
{
  pthread_mutex_lock (&m_mutex);
  runnable_list::iterator pos = m_runnables.begin ();
  while (pos != m_runnables.end ()) {
    if (pos->aid == aid && pos->param == param) {
      pos = m_runnables.erase (pos);
    }
    else {
      ++pos;
    }
  }

  pthread_mutex_unlock (&m_mutex);
}
