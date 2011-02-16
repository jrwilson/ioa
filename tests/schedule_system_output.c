#include <stdlib.h>
#include <assert.h>
#include <ueioa.h>

#include "test.h"

static void*
schedule_system_output_create (void)
{
  return NULL;
}

static void
schedule_system_output_system_input (void* state, void* param, bid_t bid)
{
  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));
  const receipt_t* receipt = buffer_read_ptr (bid);

  if (receipt->type == SELF_CREATED) {
    schedule_system_output ();
  }
  else {
    assert (0);
  }

}

static bid_t
schedule_system_output_system_output (void* state, void* param)
{
  exit (EXIT_SUCCESS);
  return -1;
}

static input_t schedule_system_output_inputs[] = { NULL };
static input_t schedule_system_output_free_inputs[] = { NULL };
static output_t schedule_system_output_outputs[] = { NULL };
static internal_t schedule_system_output_internals[] = { NULL };

descriptor_t schedule_system_output_descriptor = {
  .constructor = schedule_system_output_create,
  .system_input = schedule_system_output_system_input,
  .system_output = schedule_system_output_system_output,
  .alarm_input = test_alarm_input,
  .read_input = test_read_input,
  .write_input = test_write_input,
  .free_inputs = schedule_system_output_free_inputs,
  .inputs = schedule_system_output_inputs,
  .outputs = schedule_system_output_outputs,
  .internals = schedule_system_output_internals,
};

int
main (int argc, char** argv)
{
  ueioa_run (&schedule_system_output_descriptor, 1);
  exit (EXIT_SUCCESS);
}
