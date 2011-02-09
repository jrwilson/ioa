#include "runq.h"

#include <assert.h>
#include <pthread.h>
#include <string.h>
#include <stdio.h>

#include "xstdlib.h"

struct runnable_struct {
  runnable_t* next;
  runnable_type_t type;
  aid_t aid;
  union {
    struct {
      output_t output;
    } output;
    struct {
      internal_t internal;
    } internal;
  };
};

struct runq_struct {
  pthread_cond_t cond;
  pthread_mutex_t mutex;
  runnable_t* front;
  runnable_t** back;
  size_t size;
};

runq_t*
runq_create (void)
{
  runq_t* runq = xmalloc (sizeof (runq_t));
  pthread_cond_init (&runq->cond, NULL);
  pthread_mutex_init (&runq->mutex, NULL);
  runq->front = NULL;
  runq->back = &runq->front;
  runq->size = 0;
  return runq;
}

void
runq_destroy (runq_t* runq)
{
  assert (runq != NULL);
  pthread_cond_destroy (&runq->cond);
  pthread_mutex_destroy (&runq->mutex);
  while (runq->front) {
    runnable_t* temp = runq->front;
    runq->front = runq->front->next;
    xfree (temp);
  }
  xfree (runq);
}

size_t
runq_size (runq_t* runq)
{
  assert (runq != NULL);
  size_t retval;
  pthread_mutex_lock (&runq->mutex);
  retval = runq->size;
  pthread_mutex_unlock (&runq->mutex);
  return retval;
}

bool
runq_empty (runq_t* runq)
{
  assert (runq != NULL);
  bool retval;
  pthread_mutex_lock (&runq->mutex);
  retval = runq->front == NULL;
  pthread_mutex_unlock (&runq->mutex);
  return retval;
}

static void
runq_push (runq_t* runq, runnable_t* runnable)
{
  assert (runq != NULL);
  assert (runnable != NULL);
  runnable->next = NULL;
  pthread_mutex_lock (&runq->mutex);
  *(runq->back) = runnable;
  runq->back = &runnable->next;
  ++runq->size;
  pthread_cond_broadcast (&runq->cond);
  pthread_mutex_unlock (&runq->mutex);
}

void
runq_insert_system_input (runq_t* runq, aid_t aid)
{
  assert (runq != NULL);
  assert (aid != -1);

  runnable_t* runnable = xmalloc (sizeof (runnable_t));
  runnable->type = SYSTEM_INPUT;
  runnable->aid = aid;
  runq_push (runq, runnable);
}

void
runq_insert_system_output (runq_t* runq, aid_t aid)
{
  assert (runq != NULL);
  assert (aid != -1);

  runnable_t* runnable = xmalloc (sizeof (runnable_t));
  runnable->type = SYSTEM_OUTPUT;
  runnable->aid = aid;
  runq_push (runq, runnable);
}

void
runq_insert_output (runq_t* runq, aid_t aid, output_t output)
{
  assert (runq != NULL);
  assert (aid != -1);
  assert (output != NULL);

  runnable_t* runnable = xmalloc (sizeof (runnable_t));
  runnable->type = OUTPUT;
  runnable->aid = aid;
  runnable->output.output = output;
  runq_push (runq, runnable);
}

void
runq_insert_internal (runq_t* runq, aid_t aid, internal_t internal)
{
  assert (runq != NULL);
  assert (aid != -1);
  assert (internal != NULL);

  runnable_t* runnable = xmalloc (sizeof (runnable_t));
  runnable->type = INTERNAL;
  runnable->aid = aid;
  runnable->internal.internal = internal;
  runq_push (runq, runnable);
}

runnable_t*
runq_pop (runq_t* runq)
{
  assert (runq != NULL);
  pthread_mutex_lock (&runq->mutex);
  while (runq->front == NULL) {
    pthread_cond_wait (&runq->cond, &runq->mutex);
  }
  runnable_t* retval = runq->front;
  runq->front = runq->front->next;
  if (runq->front == NULL) {
    runq->back = &runq->front;
  }
  --runq->size;
  pthread_mutex_unlock (&runq->mutex);
  return retval;
}

void
runq_purge (runq_t* runq, aid_t aid)
{
  assert (runq != NULL);
  pthread_mutex_lock (&runq->mutex);

  runnable_t** ptr = &runq->front;
  while (*ptr != NULL) {
    if ((*ptr)->aid == aid) {
      runnable_t* temp = *ptr;
      *ptr = temp->next;
      xfree (temp);
      --runq->size;
    }
    else {
      ptr = &(*ptr)->next;
    }
  }

  if (runq->front == NULL) {
    runq->back = &runq->front;
  }

  pthread_mutex_unlock (&runq->mutex);
}

runnable_type_t
runnable_type (runnable_t* runnable)
{
  assert (runnable != NULL);
  return runnable->type;
}

aid_t
runnable_aid (runnable_t* runnable)
{
  assert (runnable != NULL);
  return runnable->aid;
}

output_t
runnable_output_output (runnable_t* runnable)
{
  assert (runnable != NULL);
  assert (runnable->type == OUTPUT);
  return runnable->output.output;
}

internal_t
runnable_internal_internal (runnable_t* runnable)
{
  assert (runnable != NULL);
  assert (runnable->type == INTERNAL);
  return runnable->internal.internal;
}

void
runnable_destroy (runnable_t* runnable)
{
  assert (runnable != NULL);
  xfree (runnable);
}

void
runq_dump (runq_t* runq)
{
  assert (runq != NULL);
  pthread_mutex_lock (&runq->mutex);
  runnable_t* runnable;
  for (runnable = runq->front;
       runnable != NULL;
       runnable = runnable->next) {
    printf ("(%p, %d, %d) ", runnable, runnable->type, runnable->aid);
  }
  printf ("\n");
  pthread_mutex_unlock (&runq->mutex);
}
