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

  if (receipt->type == SELF_CREATED ||
      receipt->type == OUTPUT_COMPOSED ||
      receipt->type == INPUT_COMPOSED ||
      receipt->type == INPUT_DECOMPOSED) {
    /* Good. */
  }
  else if (receipt->type == OUTPUT_DECOMPOSED) {
    exit (EXIT_SUCCESS);
  }
  else {
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

static input_t child_free_inputs[] = { NULL };
static input_t child_inputs[] = { child_input, NULL };
static output_t child_outputs[] = { child_output, NULL };
static internal_t child_internals[] = { NULL };

descriptor_t child_descriptor = {
  .constructor = child_create,
  .system_input = child_system_input,
  .system_output = child_system_output,
  .free_inputs = child_free_inputs,
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
} output_decomposed_state_t;

typedef struct {
  output_decomposed_state_t state;
  aid_t child1;
  aid_t child2;
} output_decomposed_t;

static void*
output_decomposed_create (void)
{
  output_decomposed_t* output_decomposed = malloc (sizeof (output_decomposed_t));
  output_decomposed->state = START;

  return output_decomposed;
}

static void
output_decomposed_system_input (void* state, void* param, bid_t bid)
{
  output_decomposed_t* output_decomposed = state;
  assert (output_decomposed != NULL);

  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));
  const receipt_t* receipt = buffer_read_ptr (bid);

  switch (output_decomposed->state) {
  case START:
    if (receipt->type == SELF_CREATED) {
      output_decomposed->state = CREATE1_UNSENT;
      schedule_system_output ();
    }
    else {
      assert (0);
    }
    break;
  case CREATE1_UNSENT:
    assert (0);
    break;
  case CREATE1_SENT:
    if (receipt->type == CHILD_CREATED) {
      output_decomposed->state = CREATE2_UNSENT;
      output_decomposed->child1 = receipt->child_created.child;
      schedule_system_output ();
    }
    else {
      assert (0);
    }
    break;
  case CREATE2_UNSENT:
    assert (0);
    break;
  case CREATE2_SENT:
    if (receipt->type == CHILD_CREATED) {
      output_decomposed->state = COMPOSE_UNSENT;
      output_decomposed->child2 = receipt->child_created.child;
      schedule_system_output ();
    }
    else {
      assert (0);
    }
    break;
  case COMPOSE_UNSENT:
    assert (0);
    break;
  case COMPOSE_SENT:
    if (receipt->type == COMPOSED) {
      output_decomposed->state = DECOMPOSE_UNSENT;
      schedule_system_output ();
    }
    else {
      assert (0);
    }
    break;
  case DECOMPOSE_UNSENT:
    assert (0);
    break;
  case DECOMPOSE_SENT:
    if (receipt->type == DECOMPOSED) {
      /* Good. */
    }
    else {
      assert (0);
    }
    break;
  }
}

static bid_t
output_decomposed_system_output (void* state, void* param)
{
  output_decomposed_t* output_decomposed = state;
  assert (output_decomposed != NULL);

  bid_t bid = buffer_alloc (sizeof (order_t));
  order_t* order = buffer_write_ptr (bid);

  switch (output_decomposed->state) {
  case START:
    assert (0);
    break;
  case CREATE1_UNSENT:
    /* Send a create order. */
    order_create_init (order, &child_descriptor);
    output_decomposed->state = CREATE1_SENT;
    break;
  case CREATE1_SENT:
    assert (0);
    break;
  case CREATE2_UNSENT:
    /* Send a create order. */
    order_create_init (order, &child_descriptor);
    output_decomposed->state = CREATE2_SENT;
    break;
  case CREATE2_SENT:
    assert (0);
    break;
  case COMPOSE_UNSENT:
    /* Send a compose order. */
    order_compose_init (order, output_decomposed->child1, child_output, NULL, output_decomposed->child2, child_input, NULL);
    output_decomposed->state = COMPOSE_SENT;
    break;
  case COMPOSE_SENT:
    assert (0);
    break;
  case DECOMPOSE_UNSENT:
    /* Send a decompose order. */
    order_decompose_init (order, output_decomposed->child1, child_output, NULL, output_decomposed->child2, child_input, NULL);
    output_decomposed->state = DECOMPOSE_SENT;
    break;
  case DECOMPOSE_SENT:
    assert (0);
    break;
  }

  return bid;
}

static input_t output_decomposed_free_inputs[] = { NULL };
static input_t output_decomposed_inputs[] = { NULL };
static output_t output_decomposed_outputs[] = { NULL };
static internal_t output_decomposed_internals[] = { NULL };

descriptor_t output_decomposed_descriptor = {
  .constructor = output_decomposed_create,
  .system_input = output_decomposed_system_input,
  .system_output = output_decomposed_system_output,
  .free_inputs = output_decomposed_free_inputs,
  .inputs = output_decomposed_inputs,
  .outputs = output_decomposed_outputs,
  .internals = output_decomposed_internals,
};

int
main (int argc, char** argv)
{
  ueioa_run (&output_decomposed_descriptor, 1);
  exit (EXIT_SUCCESS);
}
