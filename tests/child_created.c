#include <stdlib.h>
#include <assert.h>
#include <ueioa.h>

static void*
child_create (void)
{
  return NULL;
}

static void
child_system_input (void* state, void* param, bid_t bid)
{
  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));
  const receipt_t* receipt = buffer_read_ptr (bid);

  switch (receipt->type) {
  case SELF_CREATED:
    /* Good. */
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
  case OUTPUT_COMPOSED:
  case INPUT_COMPOSED:
  case NOT_COMPOSED:
  case DECOMPOSED:
  case INPUT_DECOMPOSED:
  case OUTPUT_DECOMPOSED:
  case RESCINDED:
  case AUTOMATON_DNE:
  case NOT_OWNER:
  case CHILD_DESTROYED:
  case WAKEUP:
    assert (0);
  }
}

static bid_t
child_system_output (void* state, void* param)
{
  return -1;
}

static input_t child_inputs[] = { NULL };
static output_t child_outputs[] = { NULL };
static internal_t child_internals[] = { NULL };

descriptor_t child_descriptor = {
  .constructor = child_create,
  .system_input = child_system_input,
  .system_output = child_system_output,
  .inputs = child_inputs,
  .outputs = child_outputs,
  .internals = child_internals,
};


typedef enum {
  UNSENT,
  SENT
} child_created_state_t;

typedef struct {
  child_created_state_t state;
} child_created_t;

static void*
child_created_create (void)
{
  child_created_t* child_created = malloc (sizeof (child_created_t));
  child_created->state = UNSENT;

  return child_created;
}

static void
child_created_system_input (void* state, void* param, bid_t bid)
{
  child_created_t* child_created = state;
  assert (child_created != NULL);

  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));
  const receipt_t* receipt = buffer_read_ptr (bid);

  switch (child_created->state) {
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
    case WAKEUP:
      assert (0);
    }
    break;
  case SENT:
    switch (receipt->type) {
    case CHILD_CREATED:
      exit (EXIT_SUCCESS);
    case BAD_ORDER:
    case SELF_CREATED:
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
    case WAKEUP:
      assert (0);
    }
    break;
  }
}

static bid_t
child_created_system_output (void* state, void* param)
{
  child_created_t* child_created = state;
  assert (child_created != NULL);

  bid_t bid = buffer_alloc (sizeof (order_t));
  order_t* order = buffer_write_ptr (bid);
  /* Send a create order. */
  order_create_init (order, &child_descriptor);
  child_created->state = SENT;

  return bid;
}

static input_t child_created_inputs[] = { NULL };
static output_t child_created_outputs[] = { NULL };
static internal_t child_created_internals[] = { NULL };

descriptor_t child_created_descriptor = {
  .constructor = child_created_create,
  .system_input = child_created_system_input,
  .system_output = child_created_system_output,
  .inputs = child_created_inputs,
  .outputs = child_created_outputs,
  .internals = child_created_internals,
};

int
main (int argc, char** argv)
{
  ueioa_run (&child_created_descriptor, 1);
  exit (EXIT_SUCCESS);
}
