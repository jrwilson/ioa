#include "ueioa.h"

#include <assert.h>
#include <pthread.h>

#include "runq.h"
#include "automata.h"
#include "receipts.h"

#include <stdio.h>
#include <stdlib.h>

#define MAX_THREADS 128

static runq_t* runq;
static automata_t* automata;
static buffers_t* buffers;
static receipts_t* receipts;

static void*
thread_func (void* arg)
{
  for (;;) {
    runnable_t runnable;
    runq_pop (runq, &runnable);
    switch (runnable.type) {
    case SYSTEM_INPUT:
      automata_system_input_exec (automata, receipts, runq, buffers, runnable.aid);
      break;
    case SYSTEM_OUTPUT:
      automata_system_output_exec (automata, receipts, runq, buffers, runnable.aid);
      break;
    case OUTPUT:
      automata_output_exec (automata, buffers, runnable.aid, runnable.output.output, runnable.param);
      break;
    case INTERNAL:
      automata_internal_exec (automata, runnable.aid, runnable.internal.internal, runnable.param);
      break;
    }
  }

  pthread_exit (NULL);
}

void
ueioa_run (descriptor_t* descriptor, int thread_count)
{
  assert (descriptor != NULL);
  assert (descriptor_check (descriptor));
  assert (thread_count > 0 && thread_count <= MAX_THREADS);

  pthread_t a_thread[MAX_THREADS];

  runq = runq_create ();
  automata = automata_create ();
  buffers = buffers_create ();
  receipts = receipts_create ();
  
  automata_create_automaton (automata, receipts, runq, descriptor);

  int idx;
  for (idx = 0; idx < thread_count; ++idx) {
    if (pthread_create (&a_thread[idx], NULL, thread_func, NULL) != 0) {
      perror ("pthread_create");
      exit (EXIT_FAILURE);
    }
  }

  for (idx = 0; idx < thread_count; ++idx) {
    pthread_join (a_thread[idx], NULL);
  }

  receipts_destroy (receipts);
  buffers_destroy (buffers);
  automata_destroy (automata);
  runq_destroy (runq);
}

void
schedule_system_output (void)
{
  runq_insert_system_output (runq, automata_get_current_aid (automata));
}

int
schedule_output (output_t output, void* param)
{
  aid_t aid = automata_get_current_aid (automata);
  if (automata_output_exists (automata, aid, output, param)) {
    runq_insert_output (runq, aid, output, param);
    return 0;
  }
  else {
    return -1;
  }
}

int
schedule_internal (internal_t internal, void* param)
{
  aid_t aid = automata_get_current_aid (automata);
  if (automata_internal_exists (automata, aid, internal, param)) {
    runq_insert_internal (runq, aid, internal, param);
    return 0;
  }
  else {
    return -1;
  }
}

bid_t
buffer_alloc (size_t size)
{
  return buffers_alloc (buffers, automata_get_current_aid (automata), size);
}

void*
buffer_write_ptr (bid_t bid)
{
  return buffers_write_ptr (buffers, automata_get_current_aid (automata), bid);
}

const void*
buffer_read_ptr (bid_t bid)
{
  return buffers_read_ptr (buffers, automata_get_current_aid (automata), bid);
}

size_t
buffer_size (bid_t bid)
{
  return buffers_size (buffers, automata_get_current_aid (automata), bid);
}

void
buffer_incref (bid_t bid)
{
  buffers_incref (buffers, automata_get_current_aid (automata), bid);
}

void
buffer_decref (bid_t bid)
{
  buffers_decref (buffers, automata_get_current_aid (automata), bid);
}

void
buffer_add_child (bid_t parent, bid_t child)
{
  buffers_add_child (buffers, automata_get_current_aid (automata), parent, child);
}

void
buffer_remove_child (bid_t parent, bid_t child)
{
  buffers_remove_child (buffers, automata_get_current_aid (automata), parent, child);
}

bid_t
buffer_dup (bid_t bid)
{
  return buffers_dup (buffers, automata_get_current_aid (automata), bid);
}
