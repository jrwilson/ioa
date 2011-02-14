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
  case OUTPUT_COMPOSED:
  case INPUT_COMPOSED:
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

static void
child_input (void* state, void* param, bid_t bid)
{

}

static bid_t
child_output (void* state, void* param)
{
  return -1;
}

static input_t child_inputs[] = { child_input, NULL };
static output_t child_outputs[] = { child_output, NULL };
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
  CREATE1_UNSENT,
  CREATE1_SENT,
  CREATE2_UNSENT,
  CREATE2_SENT,
  COMPOSE_UNSENT,
  COMPOSE_SENT,
  DECOMPOSE_UNSENT,
  DECOMPOSE_SENT,
} decomposed_state_t;

typedef struct {
  decomposed_state_t state;
  aid_t child1;
  aid_t child2;
} decomposed_t;

static void*
decomposed_create (void)
{
  decomposed_t* decomposed = malloc (sizeof (decomposed_t));
  decomposed->state = START;

  return decomposed;
}

static void
decomposed_system_input (void* state, void* param, bid_t bid)
{
  decomposed_t* decomposed = state;
  assert (decomposed != NULL);

  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));
  const receipt_t* receipt = buffer_read_ptr (bid);

  switch (decomposed->state) {
  case START:
    switch (receipt->type) {
    case SELF_CREATED:
      decomposed->state = CREATE1_UNSENT;
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
  case CREATE1_UNSENT:
    assert (0);
    break;
  case CREATE1_SENT:
    switch (receipt->type) {
    case CHILD_CREATED:
      decomposed->state = CREATE2_UNSENT;
      decomposed->child1 = receipt->child_created.child;
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
    case WAKEUP:
      assert (0);
    }
    break;
  case CREATE2_UNSENT:
    assert (0);
    break;
  case CREATE2_SENT:
    switch (receipt->type) {
    case CHILD_CREATED:
      decomposed->state = COMPOSE_UNSENT;
      decomposed->child2 = receipt->child_created.child;
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
    case WAKEUP:
      assert (0);
    }
    break;
  case COMPOSE_UNSENT:
    assert (0);
    break;
  case COMPOSE_SENT:
    switch (receipt->type) {
    case COMPOSED:
      decomposed->state = DECOMPOSE_UNSENT;
      schedule_system_output ();
      break;
    case CHILD_CREATED:
    case BAD_ORDER:
    case SELF_CREATED:
    case BAD_DESCRIPTOR:
    case DECLARED:
    case OUTPUT_DNE:
    case INPUT_DNE:
    case OUTPUT_UNAVAILABLE:
    case INPUT_UNAVAILABLE:
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
  case DECOMPOSE_UNSENT:
    assert (0);
    break;
  case DECOMPOSE_SENT:
    switch (receipt->type) {
    case DECOMPOSED:
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
decomposed_system_output (void* state, void* param)
{
  decomposed_t* decomposed = state;
  assert (decomposed != NULL);

  bid_t bid = buffer_alloc (sizeof (order_t));
  order_t* order = buffer_write_ptr (bid);

  switch (decomposed->state) {
  case START:
    assert (0);
    break;
  case CREATE1_UNSENT:
    /* Send a create order. */
    order_create_init (order, &child_descriptor);
    decomposed->state = CREATE1_SENT;
    break;
  case CREATE1_SENT:
    assert (0);
    break;
  case CREATE2_UNSENT:
    /* Send a create order. */
    order_create_init (order, &child_descriptor);
    decomposed->state = CREATE2_SENT;
    break;
  case CREATE2_SENT:
    assert (0);
    break;
  case COMPOSE_UNSENT:
    /* Send a compose order. */
    order_compose_init (order, decomposed->child1, child_output, NULL, decomposed->child2, child_input, NULL);
    decomposed->state = COMPOSE_SENT;
    break;
  case COMPOSE_SENT:
    assert (0);
    break;
  case DECOMPOSE_UNSENT:
    /* Send a decompose order. */
    order_decompose_init (order, decomposed->child1, child_output, NULL, decomposed->child2, child_input, NULL);
    decomposed->state = DECOMPOSE_SENT;
    break;
  case DECOMPOSE_SENT:
    assert (0);
    break;
  }

  return bid;
}

static input_t decomposed_inputs[] = { NULL };
static output_t decomposed_outputs[] = { NULL };
static internal_t decomposed_internals[] = { NULL };

descriptor_t decomposed_descriptor = {
  .constructor = decomposed_create,
  .system_input = decomposed_system_input,
  .system_output = decomposed_system_output,
  .inputs = decomposed_inputs,
  .outputs = decomposed_outputs,
  .internals = decomposed_internals,
};

int
main (int argc, char** argv)
{
  ueioa_run (&decomposed_descriptor, 1);
  exit (EXIT_SUCCESS);
}
