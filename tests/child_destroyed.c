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

static input_t child_free_inputs[] = { NULL };
static input_t child_inputs[] = { NULL };
static output_t child_outputs[] = { NULL };
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
  CREATE_UNSENT,
  CREATE_SENT,
  DESTROY_UNSENT,
  DESTROY_SENT
} child_destroyed_state_t;

typedef struct {
  child_destroyed_state_t state;
  aid_t child_aid;
} child_destroyed_t;

static void*
child_destroyed_create (void)
{
  child_destroyed_t* child_destroyed = malloc (sizeof (child_destroyed_t));
  child_destroyed->state = START;

  return child_destroyed;
}

static void
child_destroyed_system_input (void* state, void* param, bid_t bid)
{
  child_destroyed_t* child_destroyed = state;
  assert (child_destroyed != NULL);

  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));
  const receipt_t* receipt = buffer_read_ptr (bid);

  switch (child_destroyed->state) {
  case START:
    if (receipt->type == SELF_CREATED) {
      child_destroyed->state = CREATE_UNSENT;
      schedule_system_output ();
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
      child_destroyed->child_aid = receipt->child_created.child;
      child_destroyed->state = DESTROY_UNSENT;
      schedule_system_output ();
    }
    else {
      assert (0);
    }
    break;
  case DESTROY_UNSENT:
    assert (0);
    break;
  case DESTROY_SENT:
    if (receipt->type == CHILD_DESTROYED) {
      exit (EXIT_SUCCESS);
    }
    else {
      assert (0);
    }
    break;
  }
}

static bid_t
child_destroyed_system_output (void* state, void* param)
{
  child_destroyed_t* child_destroyed = state;
  assert (child_destroyed != NULL);

  bid_t bid = buffer_alloc (sizeof (order_t));
  order_t* order = buffer_write_ptr (bid);

  switch (child_destroyed->state) {
  case START:
    assert (0);
    break;
  case CREATE_UNSENT:
    /* Send a create order. */
    order_create_init (order, &child_descriptor);
    child_destroyed->state = CREATE_SENT;
    break;
  case CREATE_SENT:
    assert (0);
    break;
  case DESTROY_UNSENT:
    /* Send a destroy order. */
    order_destroy_init (order, child_destroyed->child_aid);
    child_destroyed->state = DESTROY_SENT;
    break;
  case DESTROY_SENT:
    assert (0);
    break;
  }

  return bid;
}

static input_t child_destroyed_free_inputs[] = { NULL };
static input_t child_destroyed_inputs[] = { NULL };
static output_t child_destroyed_outputs[] = { NULL };
static internal_t child_destroyed_internals[] = { NULL };

descriptor_t child_destroyed_descriptor = {
  .constructor = child_destroyed_create,
  .system_input = child_destroyed_system_input,
  .system_output = child_destroyed_system_output,
  .alarm_input = test_alarm_input,
  .read_input = test_read_input,
  .write_input = test_write_input,
  .free_inputs = child_destroyed_free_inputs,
  .inputs = child_destroyed_inputs,
  .outputs = child_destroyed_outputs,
  .internals = child_destroyed_internals,
};

int
main (int argc, char** argv)
{
  ueioa_run (&child_destroyed_descriptor, 1);
  exit (EXIT_SUCCESS);
}
