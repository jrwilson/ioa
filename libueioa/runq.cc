#include "runq.h"

#include <assert.h>
#include <pthread.h>
#include <stdlib.h>

#include "table.h"

static bool
runnable_equal (const void* x0, const void* y0)
{
  const runnable_t* x = (const runnable_t*)x0;
  const runnable_t* y = (const runnable_t*)y0;
  if (x->type != y->type) {
    return false;
  }
  if (x->aid != y->aid) {
    return false;
  }
  if (x->param != y->param) {
    return false;
  }

  switch (x->type) {
  case SYSTEM_INPUT:
  case SYSTEM_OUTPUT:
  case ALARM_INPUT:
  case READ_INPUT:
  case WRITE_INPUT:
    return true;
    break;
  case FREE_INPUT:
    return
      x->free_input.caller_aid == y->free_input.caller_aid &&
      x->free_input.free_input == y->free_input.free_input &&
      x->free_input.bid == y->free_input.bid;
    break;
  case OUTPUT:
    return x->output.output == y->output.output;
    break;
  case INTERNAL:
    return x->internal.internal == y->internal.internal;
    break;
  }

  /* Not reached. */
  assert (0);
  return true;
}


static bool
runnable_aid_equal (const void* x0, const void* y0)
{
  const runnable_t* x = (const runnable_t*)x0;
  const runnable_t* y = (const runnable_t*)y0;
  return x->aid == y->aid;
}

static bool
runnable_aid_param_equal (const void* x0, const void* y0)
{
  const runnable_t* x = (const runnable_t*)x0;
  const runnable_t* y = (const runnable_t*)y0;
  return x->aid == y->aid && x->param == y->param;
}

struct runq_struct {
  pthread_cond_t cond;
  pthread_mutex_t mutex;
  table_t* table;
  index_t* index;
};

runq_t*
runq_create (void)
{
  runq_t* runq = (runq_t*)malloc (sizeof (runq_t));
  pthread_cond_init (&runq->cond, NULL);
  pthread_mutex_init (&runq->mutex, NULL);
  runq->table = table_create (sizeof (runnable_t));
  runq->index = index_create_list (runq->table);
  return runq;
}

void
runq_destroy (runq_t* runq)
{
  assert (runq != NULL);
  pthread_cond_destroy (&runq->cond);
  pthread_mutex_destroy (&runq->mutex);
  table_destroy (runq->table);
  free (runq);
}

size_t
runq_size (runq_t* runq)
{
  assert (runq != NULL);
  size_t retval;
  pthread_mutex_lock (&runq->mutex);
  retval = index_size (runq->index);
  pthread_mutex_unlock (&runq->mutex);
  return retval;
}

bool
runq_empty (runq_t* runq)
{
  assert (runq != NULL);
  bool retval;
  pthread_mutex_lock (&runq->mutex);
  retval = index_empty (runq->index);
  pthread_mutex_unlock (&runq->mutex);
  return retval;
}



static void
push (runq_t* runq, runnable_t* runnable)
{
  assert (runq != NULL);
  assert (runnable != NULL);
  pthread_mutex_lock (&runq->mutex);
  index_insert_unique (runq->index, runnable_equal, runnable);
  pthread_cond_broadcast (&runq->cond);
  pthread_mutex_unlock (&runq->mutex);
}

void
runq_insert_system_input (runq_t* runq, aid_t aid)
{
  assert (runq != NULL);
  assert (aid != -1);

  runnable_t runnable;
  runnable.type = SYSTEM_INPUT;
  runnable.aid = aid;
  runnable.param = NULL;
  push (runq, &runnable);
}

void
runq_insert_system_output (runq_t* runq, aid_t aid)
{
  assert (runq != NULL);
  assert (aid != -1);

  runnable_t runnable;
  runnable.type = SYSTEM_OUTPUT;
  runnable.aid = aid;
  runnable.param = NULL;

  push (runq, &runnable);
}

void
runq_insert_alarm_input (runq_t* runq, aid_t aid)
{
  assert (runq != NULL);
  assert (aid != -1);

  runnable_t runnable;
  runnable.type = ALARM_INPUT;
  runnable.aid = aid;
  runnable.param = NULL;
  push (runq, &runnable);
}

void
runq_insert_read_input (runq_t* runq, aid_t aid)
{
  assert (runq != NULL);
  assert (aid != -1);

  runnable_t runnable;
  runnable.type = READ_INPUT;
  runnable.aid = aid;
  runnable.param = NULL;
  push (runq, &runnable);
}

void
runq_insert_write_input (runq_t* runq, aid_t aid)
{
  assert (runq != NULL);
  assert (aid != -1);

  runnable_t runnable;
  runnable.type = WRITE_INPUT;
  runnable.aid = aid;
  runnable.param = NULL;
  push (runq, &runnable);
}

void
runq_insert_free_input (runq_t* runq, aid_t caller_aid, aid_t aid, input_t free_input, bid_t bid)
{
  assert (runq != NULL);
  assert (aid != -1);

  runnable_t runnable;
  runnable.type = FREE_INPUT;
  runnable.aid = aid;
  runnable.param = NULL;
  runnable.free_input.caller_aid = caller_aid;
  runnable.free_input.free_input = free_input;
  runnable.free_input.bid = bid;
  push (runq, &runnable);
}

void
runq_insert_output (runq_t* runq, aid_t aid, output_t output, void* param)
{
  assert (runq != NULL);
  assert (aid != -1);
  assert (output != NULL);

  runnable_t runnable;
  runnable.type = OUTPUT;
  runnable.aid = aid;
  runnable.param = param;
  runnable.output.output = output;
  push (runq, &runnable);
}

void
runq_insert_internal (runq_t* runq, aid_t aid, internal_t internal, void* param)
{
  assert (runq != NULL);
  assert (aid != -1);
  assert (internal != NULL);

  runnable_t runnable;
  runnable.type = INTERNAL;
  runnable.aid = aid;
  runnable.param = param;
  runnable.internal.internal = internal;
  push (runq, &runnable);
}

void
runq_pop (runq_t* runq, runnable_t* runnable)
{
  assert (runq != NULL);
  assert (runnable != NULL);

  pthread_mutex_lock (&runq->mutex);
  while (index_empty (runq->index)) {
    pthread_cond_wait (&runq->cond, &runq->mutex);
  }
  runnable_t* r = (runnable_t*)index_front (runq->index);
  *runnable = *r;
  index_pop_front (runq->index);
  pthread_mutex_unlock (&runq->mutex);
}

void
runq_purge_aid (runq_t* runq, aid_t aid)
{
  assert (runq != NULL);

  runnable_t key;
  key.aid = aid;
  pthread_mutex_lock (&runq->mutex);
  index_remove (runq->index,
		index_begin (runq->index),
		index_end (runq->index),
		runnable_aid_equal,
		&key);
  pthread_mutex_unlock (&runq->mutex);
}

void
runq_purge_aid_param (runq_t* runq, aid_t aid, void* param)
{
  assert (runq != NULL);

  runnable_t key;
  key.aid = aid;
  key.param = param;
  pthread_mutex_lock (&runq->mutex);
  index_remove (runq->index,
		index_begin (runq->index),
		index_end (runq->index),
		runnable_aid_param_equal,
		&key);
  pthread_mutex_unlock (&runq->mutex);
}
