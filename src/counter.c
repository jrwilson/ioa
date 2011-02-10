#include "counter.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include <ueioa.h>

typedef struct {
  int count;
  bool flag;
} counter_t;

static void counter_internal (void* state);

static void*
counter_create (void)
{
  printf ("counter_create\n");
  counter_t* counter = malloc (sizeof (counter_t));
  counter->count = 0;
  counter->flag = false;
  return counter;
}

static void
counter_system_input (void* state, bid_t bid)
{
  printf ("counter_system_input\n");
  assert (state != NULL);
  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));

  const receipt_t* receipt = buffer_read_ptr (bid);
  if (receipt->type == SELF_CREATED) {
    assert (schedule_internal (counter_internal) == 0);
  }
}

static bid_t
counter_system_output (void* state)
{
  printf ("counter_system_output\n");
  return -1;
}

void
counter_input (void* state, bid_t bid)
{
  printf ("counter_input\n");
  assert (state != NULL);
  counter_t* counter = state;
  counter->flag = true;
  assert (schedule_output (counter_output) == 0);
}

static void
counter_internal (void* state)
{
  printf ("counter_internal\n");
  assert (state != NULL);
  counter_t* counter = state;
  ++counter->count;
  assert (schedule_internal (counter_internal) == 0);
}

bid_t
counter_output (void* state)
{
  printf ("counter_output\n");
  assert (state != NULL);
  counter_t* counter = state;
  if (counter->flag) {
    counter->flag = false;
    bid_t bid = buffer_alloc (sizeof (counter_output_t));
    counter_output_t* output = buffer_write_ptr (bid);
    output->count = counter->count;
    return bid;
  }
  else {
    return -1;
  }
}

static input_t counter_inputs[] = { counter_input, NULL };
static output_t counter_outputs[] = { counter_output, NULL };
static internal_t counter_internals[] = { counter_internal, NULL };

descriptor_t counter_descriptor = {
  .constructor = counter_create,
  .system_input = counter_system_input,
  .system_output = counter_system_output,
  .inputs = counter_inputs,
  .outputs = counter_outputs,
  .internals = counter_internals,
};
