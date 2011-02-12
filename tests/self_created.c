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

  switch (receipt->type) {
  case SELF_CREATED:
    exit (EXIT_SUCCESS);
    break;
  case BAD_ORDER:
  case CHILD_CREATED:
  case BAD_DESCRIPTOR:
  case DECLARED:
  case OUTPUT_DNE:
  case INPUT_DNE:
  case OUTPUT_UNAVAILABLE:
  case INPUT_UNAVAILABLE:
  case COMPOSED:
  case INPUT_COMPOSED:
  case OUTPUT_COMPOSED:
  case NOT_COMPOSED:
  case DECOMPOSED:
  case INPUT_DECOMPOSED:
  case OUTPUT_DECOMPOSED:
  case RESCINDED:
  case AUTOMATON_DNE:
  case NOT_OWNER:
  case CHILD_DESTROYED:
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
  ueioa_run (&self_created_descriptor);
  exit (EXIT_SUCCESS);
}
