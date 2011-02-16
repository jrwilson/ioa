#include <stdlib.h>
#include <assert.h>
#include <ueioa.h>

#include "test.h"

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
      receipt->type == INPUT_COMPOSED) {
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
child_output1 (void* state, void* param)
{
  return -1;
}

static bid_t
child_output2 (void* state, void* param)
{
  return -1;
}

static input_t child_free_inputs[] = { NULL };
static input_t child_inputs[] = { child_input, NULL };
static output_t child_outputs[] = { child_output1, child_output2, NULL };
static internal_t child_internals[] = { NULL };

descriptor_t child_descriptor = {
  .constructor = child_create,
  .system_input = child_system_input,
  .system_output = child_system_output,
  .alarm_input = test_alarm_input,
  .read_input = test_read_input,
  .write_input = test_write_input,
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
  COMPOSE1_UNSENT,
  COMPOSE1_SENT,
  COMPOSE2_UNSENT,
  COMPOSE2_SENT,
} input_unavailable_state_t;

typedef struct {
  input_unavailable_state_t state;
  aid_t child1;
  aid_t child2;
} input_unavailable_t;

static void*
input_unavailable_create (void)
{
  input_unavailable_t* input_unavailable = malloc (sizeof (input_unavailable_t));
  input_unavailable->state = START;

  return input_unavailable;
}

static void
input_unavailable_system_input (void* state, void* param, bid_t bid)
{
  input_unavailable_t* input_unavailable = state;
  assert (input_unavailable != NULL);

  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));
  const receipt_t* receipt = buffer_read_ptr (bid);

  switch (input_unavailable->state) {
  case START:
    if (receipt->type == SELF_CREATED) {
      input_unavailable->state = CREATE1_UNSENT;
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
      input_unavailable->state = CREATE2_UNSENT;
      input_unavailable->child1 = receipt->child_created.child;
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
      input_unavailable->state = COMPOSE1_UNSENT;
      input_unavailable->child2 = receipt->child_created.child;
      schedule_system_output ();
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
      input_unavailable->state = COMPOSE2_UNSENT;
      schedule_system_output ();
    }
    else {
      assert (0);
    }
    break;
  case COMPOSE2_UNSENT:
    assert (0);
    break;
  case COMPOSE2_SENT:
    if (receipt->type == INPUT_UNAVAILABLE) {
      exit (EXIT_SUCCESS);
    }
    else {
      assert (0);
    }
    break;
  }
}

static bid_t
input_unavailable_system_output (void* state, void* param)
{
  input_unavailable_t* input_unavailable = state;
  assert (input_unavailable != NULL);

  bid_t bid = buffer_alloc (sizeof (order_t));
  order_t* order = buffer_write_ptr (bid);

  switch (input_unavailable->state) {
  case START:
    assert (0);
    break;
  case CREATE1_UNSENT:
    /* Send a create order. */
    order_create_init (order, &child_descriptor);
    input_unavailable->state = CREATE1_SENT;
    break;
  case CREATE1_SENT:
    assert (0);
    break;
  case CREATE2_UNSENT:
    /* Send a create order. */
    order_create_init (order, &child_descriptor);
    input_unavailable->state = CREATE2_SENT;
    break;
  case CREATE2_SENT:
    assert (0);
    break;
  case COMPOSE1_UNSENT:
    /* Send a compose order. */
    order_compose_init (order, input_unavailable->child1, child_output1, NULL, input_unavailable->child2, child_input, NULL);
    input_unavailable->state = COMPOSE1_SENT;
    break;
  case COMPOSE1_SENT:
    assert (0);
    break;
  case COMPOSE2_UNSENT:
    /* Send a compose order. */
    order_compose_init (order, input_unavailable->child1, child_output2, NULL, input_unavailable->child2, child_input, NULL);
    input_unavailable->state = COMPOSE2_SENT;
    break;
  case COMPOSE2_SENT:
    assert (0);
    break;
  }

  return bid;
}

static input_t input_unavailable_free_inputs[] = { NULL };
static input_t input_unavailable_inputs[] = { NULL };
static output_t input_unavailable_outputs[] = { NULL };
static internal_t input_unavailable_internals[] = { NULL };

descriptor_t input_unavailable_descriptor = {
  .constructor = input_unavailable_create,
  .system_input = input_unavailable_system_input,
  .system_output = input_unavailable_system_output,
  .alarm_input = test_alarm_input,
  .read_input = test_read_input,
  .write_input = test_write_input,
  .free_inputs = input_unavailable_free_inputs,
  .inputs = input_unavailable_inputs,
  .outputs = input_unavailable_outputs,
  .internals = input_unavailable_internals,
};

int
main (int argc, char** argv)
{
  ueioa_run (&input_unavailable_descriptor, 1);
  exit (EXIT_SUCCESS);
}
