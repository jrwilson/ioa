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
    /* Good. */
    break;
  case INPUT_COMPOSED:
    exit (EXIT_SUCCESS);
  case BAD_ORDER:
  case CHILD_CREATED:
  case BAD_DESCRIPTOR:
  case DECLARED:
  case OUTPUT_DNE:
  case INPUT_DNE:
  case OUTPUT_UNAVAILABLE:
  case INPUT_UNAVAILABLE:
  case COMPOSED:
  case NOT_COMPOSER:
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
} input_composed_state_t;

typedef struct {
  input_composed_state_t state;
  aid_t child1;
  aid_t child2;
} input_composed_t;

static void*
input_composed_create (void)
{
  input_composed_t* input_composed = malloc (sizeof (input_composed_t));
  input_composed->state = START;

  return input_composed;
}

static void
input_composed_system_input (void* state, void* param, bid_t bid)
{
  input_composed_t* input_composed = state;
  assert (input_composed != NULL);

  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));
  const receipt_t* receipt = buffer_read_ptr (bid);

  switch (input_composed->state) {
  case START:
    switch (receipt->type) {
    case SELF_CREATED:
      input_composed->state = CREATE1_UNSENT;
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
    case NOT_COMPOSER:
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
  case CREATE1_UNSENT:
    assert (0);
    break;
  case CREATE1_SENT:
    switch (receipt->type) {
    case CHILD_CREATED:
      input_composed->state = CREATE2_UNSENT;
      input_composed->child1 = receipt->child_created.child;
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
    case NOT_COMPOSER:
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
  case CREATE2_UNSENT:
    assert (0);
    break;
  case CREATE2_SENT:
    switch (receipt->type) {
    case CHILD_CREATED:
      input_composed->state = COMPOSE_UNSENT;
      input_composed->child2 = receipt->child_created.child;
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
    case NOT_COMPOSER:
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
  case COMPOSE_UNSENT:
    assert (0);
    break;
  case COMPOSE_SENT:
    switch (receipt->type) {
    case COMPOSED:
      /* Good. */
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
    case NOT_COMPOSER:
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
input_composed_system_output (void* state, void* param)
{
  input_composed_t* input_composed = state;
  assert (input_composed != NULL);

  bid_t bid = buffer_alloc (sizeof (order_t));
  order_t* order = buffer_write_ptr (bid);

  switch (input_composed->state) {
  case START:
    assert (0);
    break;
  case CREATE1_UNSENT:
    /* Send a create order. */
    order_create_init (order, &child_descriptor);
    input_composed->state = CREATE1_SENT;
    break;
  case CREATE1_SENT:
    assert (0);
    break;
  case CREATE2_UNSENT:
    /* Send a create order. */
    order_create_init (order, &child_descriptor);
    input_composed->state = CREATE2_SENT;
    break;
  case CREATE2_SENT:
    assert (0);
    break;
  case COMPOSE_UNSENT:
    /* Send a compose order. */
    order_compose_init (order, input_composed->child1, child_output, NULL, input_composed->child2, child_input, NULL);
    input_composed->state = COMPOSE_SENT;
    break;
  case COMPOSE_SENT:
    assert (0);
    break;
  }

  return bid;
}

static input_t input_composed_inputs[] = { NULL };
static output_t input_composed_outputs[] = { NULL };
static internal_t input_composed_internals[] = { NULL };

descriptor_t input_composed_descriptor = {
  .constructor = input_composed_create,
  .system_input = input_composed_system_input,
  .system_output = input_composed_system_output,
  .inputs = input_composed_inputs,
  .outputs = input_composed_outputs,
  .internals = input_composed_internals,
};

int
main (int argc, char** argv)
{
  ueioa_run (&input_composed_descriptor);
  exit (EXIT_SUCCESS);
}
