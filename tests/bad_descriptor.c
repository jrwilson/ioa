#include <stdlib.h>
#include <assert.h>
#include <ueioa.h>

typedef enum {
  UNSENT,
  SENT
} bad_descriptor_state_t;

typedef struct {
  bad_descriptor_state_t state;
} bad_descriptor_t;

static void*
bad_descriptor_create (void)
{
  bad_descriptor_t* bad_descriptor = malloc (sizeof (bad_descriptor_t));
  bad_descriptor->state = UNSENT;

  return bad_descriptor;
}

static void
bad_descriptor_system_input (void* state, void* param, bid_t bid)
{
  bad_descriptor_t* bad_descriptor = state;
  assert (bad_descriptor != NULL);

  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));
  const receipt_t* receipt = buffer_read_ptr (bid);

  switch (bad_descriptor->state) {
  case UNSENT:
    switch (receipt->type) {
    case SELF_CREATED:
      schedule_system_output ();
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
    break;
  case SENT:
    switch (receipt->type) {
    case BAD_DESCRIPTOR:
      exit (EXIT_SUCCESS);
    case BAD_ORDER:
    case SELF_CREATED:
    case CHILD_CREATED:
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
    break;
  }
}

static bid_t
bad_descriptor_system_output (void* state, void* param)
{
  bad_descriptor_t* bad_descriptor = state;
  assert (bad_descriptor != NULL);

  bid_t bid = buffer_alloc (sizeof (order_t));
  order_t* order = buffer_write_ptr (bid);
  /* Send a create order. */
  order_create_init (order, NULL);
  bad_descriptor->state = SENT;

  return bid;
}

static input_t bad_descriptor_inputs[] = { NULL };
static output_t bad_descriptor_outputs[] = { NULL };
static internal_t bad_descriptor_internals[] = { NULL };

descriptor_t bad_descriptor_descriptor = {
  .constructor = bad_descriptor_create,
  .system_input = bad_descriptor_system_input,
  .system_output = bad_descriptor_system_output,
  .inputs = bad_descriptor_inputs,
  .outputs = bad_descriptor_outputs,
  .internals = bad_descriptor_internals,
};

int
main (int argc, char** argv)
{
  ueioa_run (&bad_descriptor_descriptor, 1);
  exit (EXIT_SUCCESS);
}
