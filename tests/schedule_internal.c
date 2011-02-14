#include <stdlib.h>
#include <assert.h>
#include <ueioa.h>

static void*
schedule_internal_create (void)
{
  return NULL;
}

static void
schedule_internal_internal (void* state, void* param)
{
  exit (EXIT_SUCCESS);
}

static void
schedule_internal_system_input (void* state, void* param, bid_t bid)
{
  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));
  const receipt_t* receipt = buffer_read_ptr (bid);

  switch (receipt->type) {
  case SELF_CREATED:
    assert (schedule_internal (schedule_internal_internal, NULL) == 0);
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
schedule_internal_system_output (void* state, void* param)
{
  return -1;
}

static input_t schedule_internal_inputs[] = { NULL };
static output_t schedule_internal_outputs[] = { NULL };
static internal_t schedule_internal_internals[] = { schedule_internal_internal, NULL };

descriptor_t schedule_internal_descriptor = {
  .constructor = schedule_internal_create,
  .system_input = schedule_internal_system_input,
  .system_output = schedule_internal_system_output,
  .inputs = schedule_internal_inputs,
  .outputs = schedule_internal_outputs,
  .internals = schedule_internal_internals,
};

int
main (int argc, char** argv)
{
  ueioa_run (&schedule_internal_descriptor, 1);
  exit (EXIT_SUCCESS);
}
