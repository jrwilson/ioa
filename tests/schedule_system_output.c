#include <stdlib.h>
#include <assert.h>
#include <ueioa.h>

static void
schedule_system_output_system_input (void* state, void* param, bid_t bid)
{
  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));
  const receipt_t* receipt = buffer_read_ptr (bid);

  if (receipt->type == SELF_CREATED) {
    assert (schedule_system_output () == 0);
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

descriptor_t schedule_system_output_descriptor = {
  .constructor = NULL,
  .system_input = schedule_system_output_system_input,
  .system_output = schedule_system_output_system_output,
  .alarm_input = NULL,
  .read_input = NULL,
  .write_input = NULL,
  .free_inputs = NULL,
  .inputs = NULL,
  .outputs = NULL,
  .internals = NULL,
};

int
main (int argc, char** argv)
{
  ueioa_run (&schedule_system_output_descriptor, 1);
  exit (EXIT_SUCCESS);
}
