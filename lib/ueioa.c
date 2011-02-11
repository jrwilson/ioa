#include "ueioa.h"

#include <assert.h>
#include <pthread.h>

#include "runq.h"
#include "automata.h"
#include "receipts.h"

static runq_t* runq;
static automata_t* automata;
static buffers_t* buffers;
static receipts_t* receipts;

void
ueioa_run (descriptor_t* descriptor)
{
  assert (descriptor != NULL);
  assert (descriptor_check (descriptor));

  runq = runq_create ();
  automata = automata_create ();
  buffers = buffers_create ();
  receipts = receipts_create ();
  
  automata_create_automaton (automata, receipts, runq, descriptor);

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
