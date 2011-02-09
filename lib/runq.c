#include "runq.h"

#include <assert.h>
#include <pthread.h>
#include <stdlib.h>

#include "table.h"

static bool
runnable_equal (const void* x0, const void* y0)
{
  const runnable_t* x = x0;
  const runnable_t* y = y0;
  if (x->type != y->type) {
    return false;
  }
  if (x->aid != y->aid) {
    return false;
  }
  switch (x->type) {
  case SYSTEM_INPUT:
  case SYSTEM_OUTPUT:
    return true;
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
  const runnable_t* x = x0;
  const runnable_t* y = y0;
  return x->aid == y->aid;
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
  runq_t* runq = malloc (sizeof (runq_t));
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

  runnable_t runnable = {
    .type = SYSTEM_INPUT,
    .aid = aid
  };
  push (runq, &runnable);
}

void
runq_insert_system_output (runq_t* runq, aid_t aid)
{
  assert (runq != NULL);
  assert (aid != -1);

  runnable_t runnable = {
    .type = SYSTEM_OUTPUT,
    .aid = aid
  };
  push (runq, &runnable);
}

void
runq_insert_output (runq_t* runq, aid_t aid, output_t output)
{
  assert (runq != NULL);
  assert (aid != -1);
  assert (output != NULL);

  runnable_t runnable = {
    .type = OUTPUT,
    .aid = aid,
  };
  runnable.output.output = output;
  push (runq, &runnable);
}

void
runq_insert_internal (runq_t* runq, aid_t aid, internal_t internal)
{
  assert (runq != NULL);
  assert (aid != -1);
  assert (internal != NULL);

  runnable_t runnable = {
    .type = INTERNAL,
    .aid = aid,
  };
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
  runnable_t* r = index_front (runq->index);
  *runnable = *r;
  index_pop_front (runq->index);
  pthread_mutex_unlock (&runq->mutex);
}

void
runq_purge (runq_t* runq, aid_t aid)
{
  assert (runq != NULL);

  runnable_t key = {
    .aid = aid,
  };
  pthread_mutex_lock (&runq->mutex);
  index_remove (runq->index,
		index_begin (runq->index),
		index_end (runq->index),
		runnable_aid_equal,
		&key);
  pthread_mutex_unlock (&runq->mutex);
}
