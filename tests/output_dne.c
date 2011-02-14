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

  if (receipt->type == SELF_CREATED) {
    /* Good. */
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
  CREATE1_UNSENT,
  CREATE1_SENT,
  CREATE2_UNSENT,
  CREATE2_SENT,
  COMPOSE_UNSENT,
  COMPOSE_SENT,
} output_dne_state_t;

typedef struct {
  output_dne_state_t state;
  aid_t child1;
  aid_t child2;
} output_dne_t;

static void*
output_dne_create (void)
{
  output_dne_t* output_dne = malloc (sizeof (output_dne_t));
  output_dne->state = START;

  return output_dne;
}

static void
output_dne_system_input (void* state, void* param, bid_t bid)
{
  output_dne_t* output_dne = state;
  assert (output_dne != NULL);

  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));
  const receipt_t* receipt = buffer_read_ptr (bid);

  switch (output_dne->state) {
  case START:
    if (receipt->type == SELF_CREATED) {
      output_dne->state = CREATE1_UNSENT;
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
      output_dne->state = CREATE2_UNSENT;
      output_dne->child1 = receipt->child_created.child;
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
      output_dne->state = COMPOSE_UNSENT;
      output_dne->child2 = receipt->child_created.child;
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
    if (receipt->type == OUTPUT_DNE) {
      exit (EXIT_SUCCESS);
    }
    else {
      assert (0);
    }
    break;
  }
}

static bid_t
output_dne_system_output (void* state, void* param)
{
  output_dne_t* output_dne = state;
  assert (output_dne != NULL);

  bid_t bid = buffer_alloc (sizeof (order_t));
  order_t* order = buffer_write_ptr (bid);

  switch (output_dne->state) {
  case START:
    assert (0);
    break;
  case CREATE1_UNSENT:
    /* Send a create order. */
    order_create_init (order, &child_descriptor);
    output_dne->state = CREATE1_SENT;
    break;
  case CREATE1_SENT:
    assert (0);
    break;
  case CREATE2_UNSENT:
    /* Send a create order. */
    order_create_init (order, &child_descriptor);
    output_dne->state = CREATE2_SENT;
    break;
  case CREATE2_SENT:
    assert (0);
    break;
  case COMPOSE_UNSENT:
    /* Send a compose order. */
    order_compose_init (order, output_dne->child1, child_output, NULL, output_dne->child2, child_input, NULL);
    output_dne->state = COMPOSE_SENT;
    break;
  case COMPOSE_SENT:
    assert (0);
    break;
  }

  return bid;
}

static input_t output_dne_inputs[] = { NULL };
static output_t output_dne_outputs[] = { NULL };
static internal_t output_dne_internals[] = { NULL };

descriptor_t output_dne_descriptor = {
  .constructor = output_dne_create,
  .system_input = output_dne_system_input,
  .system_output = output_dne_system_output,
  .inputs = output_dne_inputs,
  .outputs = output_dne_outputs,
  .internals = output_dne_internals,
};

int
main (int argc, char** argv)
{
  ueioa_run (&output_dne_descriptor, 1);
  exit (EXIT_SUCCESS);
}
