#include <assert.h>
#include <stdlib.h>

#include <ueioa.h>

#include "test.h"

static void*
alarm_create (void)
{
  return NULL;
}

static bid_t alarm_system_output (void* state, void* param);

static void
alarm_system_input (void* state, void* param, bid_t bid)
{
  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));
  const receipt_t* receipt = buffer_read_ptr (bid);

  if (receipt->type == SELF_CREATED) {
    schedule_alarm_input (1, 0);
  }
  else {
    assert (0);
  }
}

static bid_t
alarm_system_output (void* state, void* param)
{
  return -1;
}

static void
alarm_alarm_input (void* state, void* param, bid_t bid)
{
  exit (EXIT_SUCCESS);
}

static input_t alarm_free_inputs[] = { NULL };
static input_t alarm_inputs[] = { NULL };
static output_t alarm_outputs[] = { NULL };
static internal_t alarm_internals[] = { NULL };

descriptor_t alarm_descriptor = {
  .constructor = alarm_create,
  .system_input = alarm_system_input,
  .system_output = alarm_system_output,
  .alarm_input = alarm_alarm_input,
  .read_input = test_read_input,
  .write_input = test_write_input,
  .free_inputs = alarm_free_inputs,
  .inputs = alarm_inputs,
  .outputs = alarm_outputs,
  .internals = alarm_internals,
};


int
main (int argc, char** argv)
{
  ueioa_run (&alarm_descriptor, 1);

  exit (EXIT_SUCCESS);
}
