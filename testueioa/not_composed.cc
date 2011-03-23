#include <stdlib.h>
#include <assert.h>
#include <ueioa.hh>

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
  DECOMPOSE_UNSENT,
  DECOMPOSE_SENT,
} not_composed_state_t;

typedef struct {
  not_composed_state_t state;
  aid_t child1;
  aid_t child2;
} not_composed_t;

static void*
not_composed_create (const void* arg)
{
  not_composed_t* not_composed = (not_composed_t*)malloc (sizeof (not_composed_t));
  not_composed->state = START;

  return not_composed;
}

static void
not_composed_system_input (void* state, void* param, bid_t bid)
{
  not_composed_t* not_composed = (not_composed_t*)state;
  assert (not_composed != NULL);

  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));
  const receipt_t* receipt = (const receipt_t*)buffer_read_ptr (bid);

  switch (not_composed->state) {
  case START:
    if (receipt->type == SELF_CREATED) {
      not_composed->state = CREATE1_UNSENT;
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
      not_composed->state = CREATE2_UNSENT;
      not_composed->child1 = receipt->child_created.child;
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
      not_composed->state = DECOMPOSE_UNSENT;
      not_composed->child2 = receipt->child_created.child;
      assert (schedule_system_output () == 0);
    }
    else {
      assert (0);
    }
    break;
  case DECOMPOSE_UNSENT:
    assert (0);
    break;
  case DECOMPOSE_SENT:
    if (receipt->type == NOT_COMPOSED) {
      exit (EXIT_SUCCESS);
    }
    else {
      assert (0);
    }
    break;
  }
}

static bid_t
not_composed_system_output (void* state, void* param)
{
  not_composed_t* not_composed = (not_composed_t*)state;
  assert (not_composed != NULL);

  bid_t bid = buffer_alloc (sizeof (order_t));
  order_t* order = (order_t*)buffer_write_ptr (bid);

  switch (not_composed->state) {
  case START:
    assert (0);
    break;
  case CREATE1_UNSENT:
    /* Send a create order. */
    order_create_init (order, &child_descriptor, NULL);
    not_composed->state = CREATE1_SENT;
    break;
  case CREATE1_SENT:
    assert (0);
    break;
  case CREATE2_UNSENT:
    /* Send a create order. */
    order_create_init (order, &child_descriptor, NULL);
    not_composed->state = CREATE2_SENT;
    break;
  case CREATE2_SENT:
    assert (0);
    break;
  case DECOMPOSE_UNSENT:
    /* Send a decompose order. */
    order_decompose_init (order, not_composed->child1, child_output, NULL, not_composed->child2, child_input, NULL);
    not_composed->state = DECOMPOSE_SENT;
    break;
  case DECOMPOSE_SENT:
    assert (0);
    break;
  }

  return bid;
}

descriptor_t not_composed_descriptor = {
  not_composed_create,
  not_composed_system_input,
  not_composed_system_output,
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
  ueioa_run (&not_composed_descriptor, NULL, 1);
  exit (EXIT_SUCCESS);
}
