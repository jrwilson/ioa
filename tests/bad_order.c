#include <stdlib.h>
#include <assert.h>
#include <ueioa.h>

typedef enum {
  UNSENT,
  SENT
} bad_order_state_t;

typedef struct {
  bad_order_state_t state;
} bad_order_t;

static void*
bad_order_create (void)
{
  bad_order_t* bad_order = malloc (sizeof (bad_order_t));
  bad_order->state = UNSENT;

  return bad_order;
}

static void
bad_order_system_input (void* state, void* param, bid_t bid)
{
  bad_order_t* bad_order = state;
  assert (bad_order != NULL);

  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));
  const receipt_t* receipt = buffer_read_ptr (bid);

  switch (bad_order->state) {
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
    case BAD_ORDER:
      exit (EXIT_SUCCESS);
      break;
    case SELF_CREATED:
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
  }
}

static bid_t
bad_order_system_output (void* state, void* param)
{
  bad_order_t* bad_order = state;
  assert (bad_order != NULL);

  bid_t bid = buffer_alloc (sizeof (order_t));
  order_t* order = buffer_write_ptr (bid);
  /* Send a bad order. */
  order->type = -1;
  bad_order->state = SENT;

  return bid;
}

static input_t bad_order_inputs[] = { NULL };
static output_t bad_order_outputs[] = { NULL };
static internal_t bad_order_internals[] = { NULL };

descriptor_t bad_order_descriptor = {
  .constructor = bad_order_create,
  .system_input = bad_order_system_input,
  .system_output = bad_order_system_output,
  .inputs = bad_order_inputs,
  .outputs = bad_order_outputs,
  .internals = bad_order_internals,
};

int
main (int argc, char** argv)
{
  ueioa_run (&bad_order_descriptor);
  exit (EXIT_SUCCESS);
}
