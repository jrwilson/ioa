#include <stdlib.h>
#include <assert.h>
#include <ueioa.h>

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

descriptor_t child_descriptor = {
  .system_input = child_system_input,
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
child_destroyed_create (const void* arg)
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
      child_destroyed->child_aid = receipt->child_created.child;
      child_destroyed->state = DESTROY_UNSENT;
      assert (schedule_system_output () == 0);
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
    order_create_init (order, &child_descriptor, NULL);
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

descriptor_t child_destroyed_descriptor = {
  .constructor = child_destroyed_create,
  .system_input = child_destroyed_system_input,
  .system_output = child_destroyed_system_output,
};

int
main (int argc, char** argv)
{
  ueioa_run (&child_destroyed_descriptor, NULL, 1);
  exit (EXIT_SUCCESS);
}
