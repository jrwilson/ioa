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
      receipt->type == OUTPUT_DECOMPOSED) {
    /* Good. */
  }
  else if (receipt->type == INPUT_DECOMPOSED) {
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
} input_decomposed_state_t;

typedef struct {
  input_decomposed_state_t state;
  aid_t child1;
  aid_t child2;
} input_decomposed_t;

static void*
input_decomposed_create (void)
{
  input_decomposed_t* input_decomposed = malloc (sizeof (input_decomposed_t));
  input_decomposed->state = START;

  return input_decomposed;
}

static void
input_decomposed_system_input (void* state, void* param, bid_t bid)
{
  input_decomposed_t* input_decomposed = state;
  assert (input_decomposed != NULL);

  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));
  const receipt_t* receipt = buffer_read_ptr (bid);

  switch (input_decomposed->state) {
  case START:
    if (receipt->type == SELF_CREATED) {
      input_decomposed->state = CREATE1_UNSENT;
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
      input_decomposed->state = CREATE2_UNSENT;
      input_decomposed->child1 = receipt->child_created.child;
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
      input_decomposed->state = COMPOSE_UNSENT;
      input_decomposed->child2 = receipt->child_created.child;
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
      input_decomposed->state = DECOMPOSE_UNSENT;
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
input_decomposed_system_output (void* state, void* param)
{
  input_decomposed_t* input_decomposed = state;
  assert (input_decomposed != NULL);

  bid_t bid = buffer_alloc (sizeof (order_t));
  order_t* order = buffer_write_ptr (bid);

  switch (input_decomposed->state) {
  case START:
    assert (0);
    break;
  case CREATE1_UNSENT:
    /* Send a create order. */
    order_create_init (order, &child_descriptor);
    input_decomposed->state = CREATE1_SENT;
    break;
  case CREATE1_SENT:
    assert (0);
    break;
  case CREATE2_UNSENT:
    /* Send a create order. */
    order_create_init (order, &child_descriptor);
    input_decomposed->state = CREATE2_SENT;
    break;
  case CREATE2_SENT:
    assert (0);
    break;
  case COMPOSE_UNSENT:
    /* Send a compose order. */
    order_compose_init (order, input_decomposed->child1, child_output, NULL, input_decomposed->child2, child_input, NULL);
    input_decomposed->state = COMPOSE_SENT;
    break;
  case COMPOSE_SENT:
    assert (0);
    break;
  case DECOMPOSE_UNSENT:
    /* Send a decompose order. */
    order_decompose_init (order, input_decomposed->child1, child_output, NULL, input_decomposed->child2, child_input, NULL);
    input_decomposed->state = DECOMPOSE_SENT;
    break;
  case DECOMPOSE_SENT:
    assert (0);
    break;
  }

  return bid;
}

static input_t input_decomposed_inputs[] = { NULL };
static output_t input_decomposed_outputs[] = { NULL };
static internal_t input_decomposed_internals[] = { NULL };

descriptor_t input_decomposed_descriptor = {
  .constructor = input_decomposed_create,
  .system_input = input_decomposed_system_input,
  .system_output = input_decomposed_system_output,
  .inputs = input_decomposed_inputs,
  .outputs = input_decomposed_outputs,
  .internals = input_decomposed_internals,
};

int
main (int argc, char** argv)
{
  ueioa_run (&input_decomposed_descriptor, 1);
  exit (EXIT_SUCCESS);
}
