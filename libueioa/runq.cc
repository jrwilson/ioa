#include "runq.h"

#include <assert.h>
#include <pthread.h>
#include <stdlib.h>

#include "table.hh"

bool
Runnable::operator== (const Runnable& runnable) const
{
  if (type != runnable.type) {
    return false;
  }
  if (aid != runnable.aid) {
    return false;
  }
  if (param != runnable.param) {
    return false;
  }

  switch (type) {
  case SYSTEM_INPUT:
  case SYSTEM_OUTPUT:
  case ALARM_INPUT:
  case READ_INPUT:
  case WRITE_INPUT:
    return true;
    break;
  case FREE_INPUT:
    return
      free_input.caller_aid == runnable.free_input.caller_aid &&
      free_input.free_input == runnable.free_input.free_input &&
      free_input.bid == runnable.free_input.bid;
    break;
  case OUTPUT:
    return output.output == runnable.output.output;
    break;
  case INTERNAL:
    return internal.internal == runnable.internal.internal;
    break;
  }

  /* Not reached. */
  assert (0);
  return true;
}

class RunnableAidEqual {
private:
  const aid_t m_aid;
public:
  RunnableAidEqual (const aid_t aid) :
    m_aid (aid) { }
  bool operator() (const Runnable& runnable) { return m_aid == runnable.aid; }
};

class RunnableAidParamEqual {
private:
  const aid_t m_aid;
  const void* m_param;
public:
  RunnableAidParamEqual (const aid_t aid, const void* param) :
    m_aid (aid),
    m_param (param) { }
  bool operator() (const Runnable& runnable)
  {
    return
      m_aid == runnable.aid &&
      m_param == runnable.param;
  }
};

Runq::Runq (void) :
  m_index (m_table)
{
  pthread_cond_init (&m_cond, NULL);
  pthread_mutex_init (&m_mutex, NULL);
}

Runq::~Runq (void)
{
  pthread_cond_destroy (&m_cond);
  pthread_mutex_destroy (&m_mutex);
}

size_t
Runq::size (void)
{
  size_t retval;
  pthread_mutex_lock (&m_mutex);
  retval = m_index.size ();
  pthread_mutex_unlock (&m_mutex);
  return retval;
}

bool
Runq::empty (void)
{
  bool retval;
  pthread_mutex_lock (&m_mutex);
  retval = m_index.empty ();
  pthread_mutex_unlock (&m_mutex);
  return retval;
}

void
Runq::push (const Runnable& runnable)
{
  pthread_mutex_lock (&m_mutex);
  m_index.insert_unique (runnable);
  pthread_cond_broadcast (&m_cond);
  pthread_mutex_unlock (&m_mutex);
}

void
Runq::insert_system_input (aid_t aid)
{
  assert (aid != -1);

  Runnable runnable;
  runnable.type = SYSTEM_INPUT;
  runnable.aid = aid;
  runnable.param = NULL;
  push (runnable);
}

void
Runq::insert_system_output (aid_t aid)
{
  assert (aid != -1);

  Runnable runnable;
  runnable.type = SYSTEM_OUTPUT;
  runnable.aid = aid;
  runnable.param = NULL;

  push (runnable);
}

void
Runq::insert_alarm_input (aid_t aid)
{
  assert (aid != -1);

  Runnable runnable;
  runnable.type = ALARM_INPUT;
  runnable.aid = aid;
  runnable.param = NULL;
  push (runnable);
}

void
Runq::insert_read_input (aid_t aid)
{
  assert (aid != -1);

  Runnable runnable;
  runnable.type = READ_INPUT;
  runnable.aid = aid;
  runnable.param = NULL;
  push (runnable);
}

void
Runq::insert_write_input (aid_t aid)
{
  assert (aid != -1);

  Runnable runnable;
  runnable.type = WRITE_INPUT;
  runnable.aid = aid;
  runnable.param = NULL;
  push (runnable);
}

void
Runq::insert_free_input (aid_t caller_aid, aid_t aid, input_t free_input, bid_t bid)
{
  assert (aid != -1);

  Runnable runnable;
  runnable.type = FREE_INPUT;
  runnable.aid = aid;
  runnable.param = NULL;
  runnable.free_input.caller_aid = caller_aid;
  runnable.free_input.free_input = free_input;
  runnable.free_input.bid = bid;
  push (runnable);
}

void
Runq::insert_output (aid_t aid, output_t output, void* param)
{
  assert (aid != -1);
  assert (output != NULL);

  Runnable runnable;
  runnable.type = OUTPUT;
  runnable.aid = aid;
  runnable.param = param;
  runnable.output.output = output;
  push (runnable);
}

void
Runq::insert_internal (aid_t aid, internal_t internal, void* param)
{
  assert (aid != -1);
  assert (internal != NULL);

  Runnable runnable;
  runnable.type = INTERNAL;
  runnable.aid = aid;
  runnable.param = param;
  runnable.internal.internal = internal;
  push (runnable);
}

Runnable
Runq::pop (void)
{
  pthread_mutex_lock (&m_mutex);
  while (m_index.empty ()) {
    pthread_cond_wait (&m_cond, &m_mutex);
  }
  Runnable retval = m_index.front ();
  m_index.pop_front ();
  pthread_mutex_unlock (&m_mutex);
  return retval;
}

void
Runq::purge_aid (aid_t aid)
{
  pthread_mutex_lock (&m_mutex);
  m_index.remove_if (RunnableAidEqual (aid));
  pthread_mutex_unlock (&m_mutex);
}

void
Runq::purge_aid_param (aid_t aid, void* param)
{
  pthread_mutex_lock (&m_mutex);
  m_index.remove_if (RunnableAidParamEqual (aid, param));
  pthread_mutex_unlock (&m_mutex);
}
