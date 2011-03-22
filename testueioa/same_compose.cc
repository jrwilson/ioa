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
  CREATE_UNSENT,
  CREATE_SENT,
  COMPOSE_UNSENT,
  COMPOSE_SENT,
} same_compose_state_t;

typedef struct {
  same_compose_state_t state;
  aid_t child;
} same_compose_t;

static void*
same_compose_create (const void* arg)
{
  same_compose_t* same_compose = (same_compose_t*)malloc (sizeof (same_compose_t));
  same_compose->state = START;

  return same_compose;
}

static void
same_compose_system_input (void* state, void* param, bid_t bid)
{
  same_compose_t* same_compose = (same_compose_t*)state;
  assert (same_compose != NULL);

  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));
  const receipt_t* receipt = (const receipt_t*)buffer_read_ptr (bid);

  switch (same_compose->state) {
  case START:
    if (receipt->type == SELF_CREATED) {
      same_compose->state = CREATE_UNSENT;
      assert (schedule_system_output () == 0);
    }
    else {
      assert (0);
    }
    break;
  case CREATE_UNSENT:
    assert (0);
    break;
  case CREATE_SENT:
    if (receipt->type == CHILD_CREATED) {
      same_compose->state = COMPOSE_UNSENT;
      same_compose->child = receipt->child_created.child;
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
same_compose_system_output (void* state, void* param)
{
  same_compose_t* same_compose = (same_compose_t*)state;
  assert (same_compose != NULL);

  bid_t bid = buffer_alloc (sizeof (order_t));
  order_t* order = (order_t*)buffer_write_ptr (bid);

  switch (same_compose->state) {
  case START:
    assert (0);
    break;
  case CREATE_UNSENT:
    /* Send a create order. */
    order_create_init (order, &child_descriptor, NULL);
    same_compose->state = CREATE_SENT;
    break;
  case CREATE_SENT:
    assert (0);
    break;
  case COMPOSE_UNSENT:
    /* Send a compose order. */
    order_compose_init (order, same_compose->child, child_output, NULL, same_compose->child, child_input, NULL);
    same_compose->state = COMPOSE_SENT;
    break;
  case COMPOSE_SENT:
    assert (0);
    break;
  }

  return bid;
}

descriptor_t same_compose_descriptor = {
  same_compose_create,
  same_compose_system_input,
  same_compose_system_output,
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
  ueioa_run (&same_compose_descriptor, NULL, 1);
  exit (EXIT_SUCCESS);
}
