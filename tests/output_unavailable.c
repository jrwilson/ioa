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
    assert (0);
  }
}

static bid_t
child_system_output (void* state, void* param)
{
  return -1;
}

static void
child_input1 (void* state, void* param, bid_t bid)
{

}

static void
child_input2 (void* state, void* param, bid_t bid)
{

}

static bid_t
child_output (void* state, void* param)
{
  return -1;
}

static input_t child_inputs[] = { child_input1, child_input2, NULL };
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
  COMPOSE1_UNSENT,
  COMPOSE1_SENT,
  COMPOSE2_UNSENT,
  COMPOSE2_SENT,
} output_unavailable_state_t;

typedef struct {
  output_unavailable_state_t state;
  aid_t child1;
  aid_t child2;
} output_unavailable_t;

static void*
output_unavailable_create (void)
{
  output_unavailable_t* output_unavailable = malloc (sizeof (output_unavailable_t));
  output_unavailable->state = START;

  return output_unavailable;
}

static void
output_unavailable_system_input (void* state, void* param, bid_t bid)
{
  output_unavailable_t* output_unavailable = state;
  assert (output_unavailable != NULL);

  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));
  const receipt_t* receipt = buffer_read_ptr (bid);

  switch (output_unavailable->state) {
  case START:
    switch (receipt->type) {
    case SELF_CREATED:
      output_unavailable->state = CREATE1_UNSENT;
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
  case CREATE1_UNSENT:
    assert (0);
    break;
  case CREATE1_SENT:
    switch (receipt->type) {
    case CHILD_CREATED:
      output_unavailable->state = CREATE2_UNSENT;
      output_unavailable->child1 = receipt->child_created.child;
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
  case CREATE2_UNSENT:
    assert (0);
    break;
  case CREATE2_SENT:
    switch (receipt->type) {
    case CHILD_CREATED:
      output_unavailable->state = COMPOSE1_UNSENT;
      output_unavailable->child2 = receipt->child_created.child;
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
  case COMPOSE1_UNSENT:
    assert (0);
    break;
  case COMPOSE1_SENT:
    switch (receipt->type) {
    case COMPOSED:
      output_unavailable->state = COMPOSE2_UNSENT;
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
      assert (0);
    }
    break;
  case COMPOSE2_UNSENT:
    assert (0);
    break;
  case COMPOSE2_SENT:
    switch (receipt->type) {
    case OUTPUT_UNAVAILABLE:
      exit (EXIT_SUCCESS);
      break;
    case BAD_ORDER:
    case SELF_CREATED:
    case BAD_DESCRIPTOR:
    case CHILD_CREATED:
    case DECLARED:
    case OUTPUT_DNE:
    case INPUT_DNE:
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
output_unavailable_system_output (void* state, void* param)
{
  output_unavailable_t* output_unavailable = state;
  assert (output_unavailable != NULL);

  bid_t bid = buffer_alloc (sizeof (order_t));
  order_t* order = buffer_write_ptr (bid);

  switch (output_unavailable->state) {
  case START:
    assert (0);
    break;
  case CREATE1_UNSENT:
    /* Send a create order. */
    order_create_init (order, &child_descriptor);
    output_unavailable->state = CREATE1_SENT;
    break;
  case CREATE1_SENT:
    assert (0);
    break;
  case CREATE2_UNSENT:
    /* Send a create order. */
    order_create_init (order, &child_descriptor);
    output_unavailable->state = CREATE2_SENT;
    break;
  case CREATE2_SENT:
    assert (0);
    break;
  case COMPOSE1_UNSENT:
    /* Send a compose order. */
    order_compose_init (order, output_unavailable->child1, child_output, NULL, output_unavailable->child2, child_input1, NULL);
    output_unavailable->state = COMPOSE1_SENT;
    break;
  case COMPOSE1_SENT:
    assert (0);
    break;
  case COMPOSE2_UNSENT:
    /* Send a compose order. */
    order_compose_init (order, output_unavailable->child1, child_output, NULL, output_unavailable->child2, child_input2, NULL);
    output_unavailable->state = COMPOSE2_SENT;
    break;
  case COMPOSE2_SENT:
    assert (0);
    break;
  }

  return bid;
}

static input_t output_unavailable_inputs[] = { NULL };
static output_t output_unavailable_outputs[] = { NULL };
static internal_t output_unavailable_internals[] = { NULL };

descriptor_t output_unavailable_descriptor = {
  .constructor = output_unavailable_create,
  .system_input = output_unavailable_system_input,
  .system_output = output_unavailable_system_output,
  .inputs = output_unavailable_inputs,
  .outputs = output_unavailable_outputs,
  .internals = output_unavailable_internals,
};

int
main (int argc, char** argv)
{
  ueioa_run (&output_unavailable_descriptor);
  exit (EXIT_SUCCESS);
}
