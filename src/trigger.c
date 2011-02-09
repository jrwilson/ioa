#include "trigger.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include <ueioa.h>

typedef struct {
  bool composed;
} trigger_t;

static void*
trigger_create (void)
{
  printf ("trigger_create\n");
  trigger_t* trigger = malloc (sizeof (trigger_t));
  trigger->composed = false;
  return trigger;
}

static void
trigger_system_input (void* state, bid_t bid)
{
  printf ("trigger_system_input\n");
  assert (state != NULL);
  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));

  trigger_t* trigger = state;
  const receipt_t* receipt = buffer_read_ptr (bid);

  if (receipt->type == OUTPUT_COMPOSED &&
      receipt->output_composed.output == trigger_output) {
    trigger->composed = true;
    assert (schedule_output (trigger_output) == 0);
  }
  else if (receipt->type == OUTPUT_DECOMPOSED &&
	   receipt->output_decomposed.output == trigger_output) {
    trigger->composed = false;
  }

}

static bid_t
trigger_system_output (void* state)
{
  printf ("trigger_system_output\n");
  return -1;
}

bid_t
trigger_output (void* state)
{
  printf ("trigger_output\n");
  assert (state != NULL);

  trigger_t* trigger = state;
  if (trigger->composed) {
    assert (schedule_output (trigger_output) == 0);
    return buffer_alloc (0);
  }
  else {
    return -1;
  }
}

static output_t trigger_outputs[] = { trigger_output };

descriptor_t trigger_descriptor = {
  .constructor = trigger_create,
  .system_input = trigger_system_input,
  .system_output = trigger_system_output,
  .input_count = 0,
  .inputs = NULL,
  .output_count = 1,
  .outputs = trigger_outputs,
  .internal_count = 0,
  .internals = NULL
};
