#include "counter.h"

#include <assert.h>
#include "xstdlib.h"
#include "ueioa.h"

#include <stdio.h>

typedef struct {
  int count;
  bool flag;
} counter_t;

static void counter_internal (state_ptr_t state);

static state_ptr_t
counter_create (void)
{
  printf ("counter_create\n");
  counter_t* counter = xmalloc (sizeof (counter_t));
  counter->count = 0;
  counter->flag = false;
  return counter;
}

static void
counter_system_input (state_ptr_t state, bid_t bid)
{
  printf ("counter_system_input\n");
  assert (state != NULL);
  assert (bid != -1);
  assert (ueioa_buffer_size (bid) == sizeof (system_receipt_t));

  const system_receipt_t* receipt = ueioa_buffer_read_ptr (bid);
  if (receipt->type == SYS_SELF_CREATED) {
    assert (ueioa_schedule_internal (counter_internal) == 0);
  }
}

static bid_t
counter_system_output (state_ptr_t state)
{
  printf ("counter_system_output\n");
  return -1;
}

void
counter_input (state_ptr_t state, bid_t bid)
{
  printf ("counter_input\n");
  assert (state != NULL);
  counter_t* counter = state;
  counter->flag = true;
  assert (ueioa_schedule_output (counter_output) == 0);
}

static void
counter_internal (state_ptr_t state)
{
  printf ("counter_internal\n");
  assert (state != NULL);
  counter_t* counter = state;
  ++counter->count;
  assert (ueioa_schedule_internal (counter_internal) == 0);
}

bid_t
counter_output (state_ptr_t state)
{
  printf ("counter_output\n");
  assert (state != NULL);
  counter_t* counter = state;
  if (counter->flag) {
    counter->flag = false;
    bid_t bid = ueioa_buffer_alloc (sizeof (counter_output_t));
    counter_output_t* output = ueioa_buffer_write_ptr (bid);
    output->count = counter->count;
    return bid;
  }
  else {
    return -1;
  }
}

static input_t counter_inputs[] = { counter_input };
static output_t counter_outputs[] = { counter_output };
static internal_t counter_internals[] = { counter_internal };

automaton_descriptor_t counter_descriptor = {
  .constructor = counter_create,
  .system_input = counter_system_input,
  .system_output = counter_system_output,
  .inputs = counter_inputs,
  .input_count = 1,
  .outputs = counter_outputs,
  .output_count = 1,
  .internals = counter_internals,
  .internal_count = 1,
};
