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
  START,
  CREATE_UNSENT,
  CREATE_SENT,
  DESTROY_UNSENT,
  DESTROY_SENT
} child_destroyed_state_t;

typedef struct {
  child_destroyed_state_t state;
  aid_t child_aid;
} child_destroyed_t;

static void*
child_destroyed_create (void)
{
  child_destroyed_t* child_destroyed = malloc (sizeof (child_destroyed_t));
  child_destroyed->state = START;

  return child_destroyed;
}

static void
child_destroyed_system_input (void* state, void* param, bid_t bid)
{
  child_destroyed_t* child_destroyed = state;
  assert (child_destroyed != NULL);

  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));
  const receipt_t* receipt = buffer_read_ptr (bid);

  switch (child_destroyed->state) {
  case START:
    switch (receipt->type) {
    case SELF_CREATED:
      child_destroyed->state = CREATE_UNSENT;
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
  case CREATE_UNSENT:
    assert (0);
    break;
  case CREATE_SENT:
    switch (receipt->type) {
    case CHILD_CREATED:
      child_destroyed->child_aid = receipt->child_created.child;
      child_destroyed->state = DESTROY_UNSENT;
      schedule_system_output ();
      break;
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
      assert (0);
    }
    break;
  case DESTROY_UNSENT:
    assert (0);
    break;
  case DESTROY_SENT:
    switch (receipt->type) {
    case CHILD_DESTROYED:
      exit (EXIT_SUCCESS);
    case CHILD_CREATED:
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
      assert (0);
    }
    break;
  }
}

static bid_t
child_destroyed_system_output (void* state, void* param)
{
  child_destroyed_t* child_destroyed = state;
  assert (child_destroyed != NULL);

  bid_t bid = buffer_alloc (sizeof (order_t));
  order_t* order = buffer_write_ptr (bid);

  switch (child_destroyed->state) {
  case START:
    assert (0);
    break;
  case CREATE_UNSENT:
    /* Send a create order. */
    order_create_init (order, &child_descriptor);
    child_destroyed->state = CREATE_SENT;
    break;
  case CREATE_SENT:
    assert (0);
    break;
  case DESTROY_UNSENT:
    /* Send a destroy order. */
    order_destroy_init (order, child_destroyed->child_aid);
    child_destroyed->state = DESTROY_SENT;
    break;
  case DESTROY_SENT:
    assert (0);
    break;
  }

  return bid;
}

static input_t child_destroyed_inputs[] = { NULL };
static output_t child_destroyed_outputs[] = { NULL };
static internal_t child_destroyed_internals[] = { NULL };

descriptor_t child_destroyed_descriptor = {
  .constructor = child_destroyed_create,
  .system_input = child_destroyed_system_input,
  .system_output = child_destroyed_system_output,
  .inputs = child_destroyed_inputs,
  .outputs = child_destroyed_outputs,
  .internals = child_destroyed_internals,
};

int
main (int argc, char** argv)
{
  ueioa_run (&child_destroyed_descriptor, 1);
  exit (EXIT_SUCCESS);
}
