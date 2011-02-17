#include <stdlib.h>
#include <assert.h>
#include <ueioa.h>

static void
self_created_system_input (void* state, void* param, bid_t bid)
{
  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));
  const receipt_t* receipt = buffer_read_ptr (bid);

  if (receipt->type == SELF_CREATED) {
    exit (EXIT_SUCCESS);
  }
  else {
    assert (0);
  }
}

descriptor_t self_created_descriptor = {
  .constructor = NULL,
  .system_input = self_created_system_input,
  .system_output = NULL,
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
  ueioa_run (&self_created_descriptor, 1);
  exit (EXIT_SUCCESS);
}
