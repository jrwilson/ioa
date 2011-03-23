#include <stdlib.h>
#include <assert.h>
#include <ueioa.hh>

static void
child_system_input (void* state, void* param, bid_t bid)
{
  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));
  const receipt_t* receipt = (const receipt_t*)buffer_read_ptr (bid);

  if (receipt->type == SELF_CREATED) {
    /* Good. */
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

static output_t child_outputs[] = { child_output, NULL };

descriptor_t child_descriptor = {
  NULL,
  child_system_input,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  child_outputs,
  NULL,
};

typedef enum {
  START,
  CREATE1_UNSENT,
  CREATE1_SENT,
  CREATE2_UNSENT,
  CREATE2_SENT,
  COMPOSE_UNSENT,
  COMPOSE_SENT,
} input_dne_state_t;

typedef struct {
  input_dne_state_t state;
  aid_t child1;
  aid_t child2;
} input_dne_t;

static void*
input_dne_create (const void* arg)
{
  input_dne_t* input_dne = (input_dne_t*)malloc (sizeof (input_dne_t));
  input_dne->state = START;

  return input_dne;
}

static void
input_dne_system_input (void* state, void* param, bid_t bid)
{
  input_dne_t* input_dne = (input_dne_t*)state;
  assert (input_dne != NULL);

  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));
  const receipt_t* receipt = (const receipt_t*)buffer_read_ptr (bid);

  switch (input_dne->state) {
  case START:
    if (receipt->type == SELF_CREATED) {
      input_dne->state = CREATE1_UNSENT;
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
      input_dne->state = CREATE2_UNSENT;
      input_dne->child1 = receipt->child_created.child;
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
      input_dne->state = COMPOSE_UNSENT;
      input_dne->child2 = receipt->child_created.child;
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
    if (receipt->type == INPUT_DNE) {
      exit (EXIT_SUCCESS);
    }
    else {
      assert (0);
    }
    break;
  }
}

static bid_t
input_dne_system_output (void* state, void* param)
{
  input_dne_t* input_dne = (input_dne_t*)state;
  assert (input_dne != NULL);

  bid_t bid = buffer_alloc (sizeof (order_t));
  order_t* order = (order_t*)buffer_write_ptr (bid);

  switch (input_dne->state) {
  case START:
    assert (0);
    break;
  case CREATE1_UNSENT:
    /* Send a create order. */
    order_create_init (order, &child_descriptor, NULL);
    input_dne->state = CREATE1_SENT;
    break;
  case CREATE1_SENT:
    assert (0);
    break;
  case CREATE2_UNSENT:
    /* Send a create order. */
    order_create_init (order, &child_descriptor, NULL);
    input_dne->state = CREATE2_SENT;
    break;
  case CREATE2_SENT:
    assert (0);
    break;
  case COMPOSE_UNSENT:
    /* Send a compose order. */
    order_compose_init (order, input_dne->child1, child_output, NULL, input_dne->child2, child_input, NULL);
    input_dne->state = COMPOSE_SENT;
    break;
  case COMPOSE_SENT:
    assert (0);
    break;
  }

  return bid;
}

descriptor_t input_dne_descriptor = {
  input_dne_create,
  input_dne_system_input,
  input_dne_system_output,
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
  ueioa_run (&input_dne_descriptor, NULL, 1);
  exit (EXIT_SUCCESS);
}
