#include <stdlib.h>
#include <assert.h>
#include <ueioa.h>

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

descriptor_t child_descriptor = {
  NULL,
  child_system_input,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
};

typedef enum {
  UNSENT,
  SENT
} child_created_state_t;

typedef struct {
  child_created_state_t state;
} child_created_t;

static void*
child_created_create (const void* arg)
{
  child_created_t* child_created = (child_created_t*)malloc (sizeof (child_created_t));
  child_created->state = UNSENT;

  return child_created;
}

static void
child_created_system_input (void* state, void* param, bid_t bid)
{
  child_created_t* child_created = (child_created_t*)state;
  assert (child_created != NULL);

  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));
  const receipt_t* receipt = (const receipt_t*)buffer_read_ptr (bid);

  switch (child_created->state) {
  case UNSENT:
    if (receipt->type == SELF_CREATED) {
      assert (schedule_system_output () == 0);
    }
    else {
      assert (0);
    }
    break;
  case SENT:
    if (receipt->type == CHILD_CREATED) {
      exit (EXIT_SUCCESS);
    }
    else {
      assert (0);
    }
    break;
  }
}

static bid_t
child_created_system_output (void* state, void* param)
{
  child_created_t* child_created = (child_created_t*)state;
  assert (child_created != NULL);

  bid_t bid = buffer_alloc (sizeof (order_t));
  order_t* order = (order_t*)buffer_write_ptr (bid);
  /* Send a create order. */
  order_create_init (order, &child_descriptor, NULL);
  child_created->state = SENT;

  return bid;
}

descriptor_t child_created_descriptor = {
  child_created_create,
  child_created_system_input,
  child_created_system_output,
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
  ueioa_run (&child_created_descriptor, NULL, 1);
  exit (EXIT_SUCCESS);
}
