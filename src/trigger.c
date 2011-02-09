#include "trigger.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include <ueioa.h>

typedef struct {
  bool composed;
} trigger_t;

static state_ptr_t
trigger_create (void)
{
  printf ("trigger_create\n");
  trigger_t* trigger = malloc (sizeof (trigger_t));
  trigger->composed = false;
  return trigger;
}

static void
trigger_system_input (state_ptr_t state, bid_t bid)
{
  printf ("trigger_system_input\n");
  assert (state != NULL);
  assert (bid != -1);
  assert (ueioa_buffer_size (bid) == sizeof (system_receipt_t));

  trigger_t* trigger = state;
  const system_receipt_t* receipt = ueioa_buffer_read_ptr (bid);

  if (receipt->type == SYS_OUTPUT_COMPOSED &&
      receipt->output_composed.output == trigger_output) {
    trigger->composed = true;
    assert (ueioa_schedule_output (trigger_output) == 0);
  }
  else if (receipt->type == SYS_OUTPUT_DECOMPOSED &&
	   receipt->output_decomposed.output == trigger_output) {
    trigger->composed = false;
  }

}

static bid_t
trigger_system_output (state_ptr_t state)
{
  printf ("trigger_system_output\n");
  return -1;
}

bid_t
trigger_output (state_ptr_t state)
{
  printf ("trigger_output\n");
  assert (state != NULL);

  trigger_t* trigger = state;
  if (trigger->composed) {
    assert (ueioa_schedule_output (trigger_output) == 0);
    return ueioa_buffer_alloc (0);
  }
  else {
    return -1;
  }
}

static output_t trigger_outputs[] = { trigger_output };

automaton_descriptor_t trigger_descriptor = {
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
