#include <stdlib.h>
#include <assert.h>
#include <ueioa.h>

static void
child_system_input (void* state, void* param, bid_t bid)
{
  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));
  const receipt_t* receipt = buffer_read_ptr (bid);

  if (receipt->type == SELF_CREATED ||
      receipt->type == INPUT_COMPOSED) {
    /* Good. */
  }
  else if (receipt->type == OUTPUT_COMPOSED) {
    exit (EXIT_SUCCESS);
  }
  else {
    assert (0);
  }
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

descriptor_t child_descriptor = {
  .system_input = child_system_input,
  .inputs = child_inputs,
  .outputs = child_outputs,
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
input_composed_create (void* arg)
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
    if (receipt->type == SELF_CREATED) {
      input_composed->state = CREATE1_UNSENT;
      assert (schedule_system_output () == 0);
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
      input_composed->state = CREATE2_UNSENT;
      input_composed->child1 = receipt->child_created.child;
      assert (schedule_system_output () == 0);
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
      input_composed->state = COMPOSE_UNSENT;
      input_composed->child2 = receipt->child_created.child;
      assert (schedule_system_output () == 0);
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
      /* Good. */
    }
    else {
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
    order_create_init (order, &child_descriptor, NULL);
    input_composed->state = CREATE1_SENT;
    break;
  case CREATE1_SENT:
    assert (0);
    break;
  case CREATE2_UNSENT:
    /* Send a create order. */
    order_create_init (order, &child_descriptor, NULL);
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

descriptor_t input_composed_descriptor = {
  .constructor = input_composed_create,
  .system_input = input_composed_system_input,
  .system_output = input_composed_system_output,
};

int
main (int argc, char** argv)
{
  ueioa_run (&input_composed_descriptor, NULL, 1);
  exit (EXIT_SUCCESS);
}
