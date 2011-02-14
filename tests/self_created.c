#include <stdlib.h>
#include <assert.h>
#include <ueioa.h>

static void*
self_created_create (void)
{
  return NULL;
}

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

static bid_t
self_created_system_output (void* state, void* param)
{
  return -1;
}

static input_t self_created_inputs[] = { NULL };
static output_t self_created_outputs[] = { NULL };
static internal_t self_created_internals[] = { NULL };

descriptor_t self_created_descriptor = {
  .constructor = self_created_create,
  .system_input = self_created_system_input,
  .system_output = self_created_system_output,
  .inputs = self_created_inputs,
  .outputs = self_created_outputs,
  .internals = self_created_internals,
};

int
main (int argc, char** argv)
{
  ueioa_run (&self_created_descriptor, 1);
  exit (EXIT_SUCCESS);
}
