#include <stdlib.h>
#include <assert.h>
#include <ueioa.h>

static void
child_system_input (void* state, void* param, bid_t bid)
{
  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));
  const receipt_t* receipt = (const receipt_t*)buffer_read_ptr (bid);

  if (receipt->type == SELF_CREATED ||
      receipt->type == OUTPUT_COMPOSED ||
      receipt->type == INPUT_COMPOSED) {
    /* Good. */
  }
  else {
    assert (0);
  }
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

descriptor_t child_descriptor = {
  NULL,
  child_system_input,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  child_inputs,
  child_outputs,
  NULL,
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
output_unavailable_create (const void* arg)
{
  output_unavailable_t* output_unavailable = (output_unavailable_t*)malloc (sizeof (output_unavailable_t));
  output_unavailable->state = START;

  return output_unavailable;
}

static void
output_unavailable_system_input (void* state, void* param, bid_t bid)
{
  output_unavailable_t* output_unavailable = (output_unavailable_t*)state;
  assert (output_unavailable != NULL);

  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));
  const receipt_t* receipt = (const receipt_t*)buffer_read_ptr (bid);

  switch (output_unavailable->state) {
  case START:
    if (receipt->type == SELF_CREATED) {
      output_unavailable->state = CREATE1_UNSENT;
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
      output_unavailable->state = CREATE2_UNSENT;
      output_unavailable->child1 = receipt->child_created.child;
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
      output_unavailable->state = COMPOSE1_UNSENT;
      output_unavailable->child2 = receipt->child_created.child;
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
      output_unavailable->state = COMPOSE2_UNSENT;
      assert (schedule_system_output () == 0);
    }
    else {
      assert (0);
    }
    break;
  case COMPOSE2_UNSENT:
    assert (0);
    break;
  case COMPOSE2_SENT:
    if (receipt->type == OUTPUT_UNAVAILABLE) {
      exit (EXIT_SUCCESS);
    }
    else {
      assert (0);
    }
    break;
  }
}

static bid_t
output_unavailable_system_output (void* state, void* param)
{
  output_unavailable_t* output_unavailable = (output_unavailable_t*)state;
  assert (output_unavailable != NULL);

  bid_t bid = buffer_alloc (sizeof (order_t));
  order_t* order = (order_t*)buffer_write_ptr (bid);

  switch (output_unavailable->state) {
  case START:
    assert (0);
    break;
  case CREATE1_UNSENT:
    /* Send a create order. */
    order_create_init (order, &child_descriptor, NULL);
    output_unavailable->state = CREATE1_SENT;
    break;
  case CREATE1_SENT:
    assert (0);
    break;
  case CREATE2_UNSENT:
    /* Send a create order. */
    order_create_init (order, &child_descriptor, NULL);
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

descriptor_t output_unavailable_descriptor = {
  output_unavailable_create,
  output_unavailable_system_input,
  output_unavailable_system_output,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
};

int
main (int argc, char** argv)
{
  ueioa_run (&output_unavailable_descriptor, NULL, 1);
  exit (EXIT_SUCCESS);
}
