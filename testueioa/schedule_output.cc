#include <stdlib.h>
#include <assert.h>
#include <ueioa.h>

static bid_t
schedule_output_output (void* state, void* param)
{
  exit (EXIT_SUCCESS);
  return -1;
}

static void
schedule_output_system_input (void* state, void* param, bid_t bid)
{
  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));
  const receipt_t* receipt = (const receipt_t*)buffer_read_ptr (bid);

  if (receipt->type == SELF_CREATED) {
    assert (schedule_output (schedule_output_output, NULL) == 0);
  }
  else {
    assert (0);
  }

}

static output_t schedule_output_outputs[] = { schedule_output_output, NULL };

descriptor_t schedule_output_descriptor = {
  NULL,
  schedule_output_system_input,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  schedule_output_outputs,
  NULL,
};

int
main (int argc, char** argv)
{
  ueioa_run (&schedule_output_descriptor, NULL, 1);
  exit (EXIT_SUCCESS);
}
