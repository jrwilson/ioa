/*
  Test the ability to compose with parameterized inputs.

  The goal configuration is:
  (child1, output, NULL) -> (self, input, NULL)
  (child2, output, NULL) -> (self, input, param1)
  (child3, output, NULL) -> (self, input, param2)
 */

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <ueioa.h>

static void
child_system_input (void* state, void* param, bid_t bid)
{
  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));
  const receipt_t* receipt = buffer_read_ptr (bid);

  if (receipt->type == SELF_CREATED ||
      receipt->type == OUTPUT_COMPOSED ||
      receipt->type == INPUT_COMPOSED) {
    /* Good. */
  }
  else {
    assert (0);
  }
}

static bid_t
child_output (void* state, void* param)
{
  return -1;
}

static output_t child_outputs[] = { child_output, NULL };

descriptor_t child_descriptor = {
  .system_input = child_system_input,
  .outputs = child_outputs,
};


typedef enum {
  START,
  CREATE1_UNSENT,
  CREATE1_SENT,
  CREATE2_UNSENT,
  CREATE2_SENT,
  CREATE3_UNSENT,
  CREATE3_SENT,
  DECLARE2_UNSENT,
  DECLARE2_SENT,
  DECLARE3_UNSENT,
  DECLARE3_SENT,
  COMPOSE1_UNSENT,
  COMPOSE1_SENT,
  COMPOSE2_UNSENT,
  COMPOSE2_SENT,
  COMPOSE3_UNSENT,
  COMPOSE3_SENT,
} compose_param_input_state_t;

typedef struct {
  compose_param_input_state_t state;
  aid_t self;
  aid_t child1;
  aid_t child2;
  aid_t child3;
  int* param2;
  int* param3;
} compose_param_input_t;

static void*
compose_param_input_create (void)
{
  compose_param_input_t* compose_param_input = malloc (sizeof (compose_param_input_t));
  compose_param_input->state = START;

  return compose_param_input;
}

static void
compose_param_input_input (void* state, void* param, bid_t bid)
{

}

static void
compose_param_input_system_input (void* state, void* param, bid_t bid)
{
  compose_param_input_t* compose_param_input = state;
  assert (compose_param_input != NULL);

  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));
  const receipt_t* receipt = buffer_read_ptr (bid);

  switch (compose_param_input->state) {
  case START:
    if (receipt->type == SELF_CREATED) {
      compose_param_input->state = CREATE1_UNSENT;
      compose_param_input->self = receipt->self_created.self;
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
      compose_param_input->child1 = receipt->child_created.child;
      compose_param_input->state = CREATE2_UNSENT;
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
      compose_param_input->child2 = receipt->child_created.child;
      compose_param_input->state = CREATE3_UNSENT;
      assert (schedule_system_output () == 0);
    }
    else {
      assert (0);
    }
    break;
  case CREATE3_UNSENT:
    assert (0);
    break;
  case CREATE3_SENT:
    if (receipt->type == CHILD_CREATED) {
      compose_param_input->child3 = receipt->child_created.child;
      compose_param_input->state = DECLARE2_UNSENT;
      assert (schedule_system_output () == 0);
    }
    else {
      assert (0);
    }
    break;
  case DECLARE2_UNSENT:
    assert (0);
    break;
  case DECLARE2_SENT:
    if (receipt->type == DECLARED) {
      compose_param_input->state = DECLARE3_UNSENT;
      assert (schedule_system_output () == 0);
    }
    else {
      assert (0);
    }
    break;
  case DECLARE3_UNSENT:
    assert (0);
    break;
  case DECLARE3_SENT:
    if (receipt->type == DECLARED) {
      compose_param_input->state = COMPOSE1_UNSENT;
      assert (schedule_system_output () == 0);
    }
    else {
      assert (0);
    }
    break;
  case COMPOSE1_UNSENT:
    assert (0);
    break;
  case COMPOSE1_SENT:
    if (receipt->type == COMPOSED) {
      compose_param_input->state = COMPOSE2_UNSENT;
      assert (schedule_system_output () == 0);
    }
    else {
      printf ("Receipt Type: %d\n", receipt->type);
      assert (0);
    }
    break;
  case COMPOSE2_UNSENT:
    assert (0);
    break;
  case COMPOSE2_SENT:
    if (receipt->type == COMPOSED) {
      compose_param_input->state = COMPOSE3_UNSENT;
      assert (schedule_system_output () == 0);
    }
    else if (receipt->type == INPUT_COMPOSED) {
      /* Good. */
    }
    else {
      printf ("Receipt Type: %d\n", receipt->type);
      assert (0);
    }
    break;
  case COMPOSE3_UNSENT:
    assert (0);
    break;
  case COMPOSE3_SENT:
    if (receipt->type == COMPOSED) {
      exit (EXIT_SUCCESS);
    }
    else if (receipt->type == INPUT_COMPOSED) {
      /* Good. */
    }
    else {
      printf ("Receipt Type: %d\n", receipt->type);
      assert (0);
    }
    break;
  }
}

static bid_t
compose_param_input_system_output (void* state, void* param)
{
  compose_param_input_t* compose_param_input = state;
  assert (compose_param_input != NULL);

  bid_t bid = buffer_alloc (sizeof (order_t));
  order_t* order = buffer_write_ptr (bid);

  switch (compose_param_input->state) {
  case START:
    assert (0);
    break;
  case CREATE1_UNSENT:
    /* Send a create order. */
    order_create_init (order, &child_descriptor);
    compose_param_input->state = CREATE1_SENT;
    break;
  case CREATE1_SENT:
    assert (0);
    break;
  case CREATE2_UNSENT:
    /* Send a create order. */
    order_create_init (order, &child_descriptor);
    compose_param_input->state = CREATE2_SENT;
    break;
  case CREATE2_SENT:
    assert (0);
    break;
  case CREATE3_UNSENT:
    /* Send a create order. */
    order_create_init (order, &child_descriptor);
    compose_param_input->state = CREATE3_SENT;
    break;
  case CREATE3_SENT:
    assert (0);
    break;
  case DECLARE2_UNSENT:
    /* Send a declare order. */
    compose_param_input->param2 = malloc (sizeof (int));
    order_declare_init (order, compose_param_input->param2);
    compose_param_input->state = DECLARE2_SENT;
    break;
  case DECLARE2_SENT:
    assert (0);
    break;
  case DECLARE3_UNSENT:
    /* Send a declare order. */
    compose_param_input->param3 = malloc (sizeof (int));
    order_declare_init (order, compose_param_input->param3);
    compose_param_input->state = DECLARE3_SENT;
    break;
  case DECLARE3_SENT:
    assert (0);
    break;
  case COMPOSE1_UNSENT:
    /* Send a compose order. */
    order_compose_init (order, compose_param_input->child1, child_output, NULL, compose_param_input->self, compose_param_input_input, NULL);
    compose_param_input->state = COMPOSE1_SENT;
    break;
  case COMPOSE1_SENT:
    assert (0);
    break;
  case COMPOSE2_UNSENT:
    /* Send a compose order. */
    order_compose_init (order, compose_param_input->child2, child_output, NULL, compose_param_input->self, compose_param_input_input, compose_param_input->param2);
    compose_param_input->state = COMPOSE2_SENT;
    break;
  case COMPOSE2_SENT:
    assert (0);
    break;
  case COMPOSE3_UNSENT:
    /* Send a compose order. */
    order_compose_init (order, compose_param_input->child3, child_output, NULL, compose_param_input->self, compose_param_input_input, compose_param_input->param3);
    compose_param_input->state = COMPOSE3_SENT;
    break;
  case COMPOSE3_SENT:
    assert (0);
    break;
  }

  return bid;
}

static input_t compose_param_input_inputs[] = { compose_param_input_input, NULL };

descriptor_t compose_param_input_descriptor = {
  .constructor = compose_param_input_create,
  .system_input = compose_param_input_system_input,
  .system_output = compose_param_input_system_output,
  .inputs = compose_param_input_inputs,
};

int
main (int argc, char** argv)
{
  ueioa_run (&compose_param_input_descriptor, 1);
  exit (EXIT_SUCCESS);
}
