#include <stdlib.h>
#include <assert.h>
#include <ueioa.h>

static aid_t parent_aid;

typedef enum {
  CUNSENT,
  CSENT
} child_state_t;

typedef struct {
  child_state_t state;
} child_t;

static void*
child_create (void)
{
  child_t* child = malloc (sizeof (child_t));
  child->state = CUNSENT;

  return child;
}

static void
child_system_input (void* state, void* param, bid_t bid)
{
  child_t* child = state;
  assert (child != NULL);

  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));
  const receipt_t* receipt = buffer_read_ptr (bid);
  
  switch (child->state) {
  case CUNSENT:
    if (receipt->type == SELF_CREATED) {
      assert (schedule_system_output () == 0);
    }
    else {
      assert (0);
    }
    break;
  case CSENT:
    if (receipt->type == NOT_OWNER) {
      exit (EXIT_SUCCESS);
    }
    else {
      assert (0);
    }
    break;
  }

}

static bid_t
child_system_output (void* state, void* param)
{
  child_t* child = state;
  assert (child != NULL);

  bid_t bid = buffer_alloc (sizeof (order_t));
  order_t* order = buffer_write_ptr (bid);
  /* Try to destroy the parent. */
  order_destroy_init (order, parent_aid);
  child->state = CSENT;

  return bid;
}

descriptor_t child_descriptor = {
  .constructor = child_create,
  .system_input = child_system_input,
  .system_output = child_system_output,
};

typedef enum {
  UNSENT,
  SENT
} not_owner_state_t;

typedef struct {
  not_owner_state_t state;
} not_owner_t;

static void*
not_owner_create (void)
{
  not_owner_t* not_owner = malloc (sizeof (not_owner_t));
  not_owner->state = UNSENT;

  return not_owner;
}

static void
not_owner_system_input (void* state, void* param, bid_t bid)
{
  not_owner_t* not_owner = state;
  assert (not_owner != NULL);

  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));
  const receipt_t* receipt = buffer_read_ptr (bid);

  switch (not_owner->state) {
  case UNSENT:
    if (receipt->type == SELF_CREATED) {
      parent_aid = receipt->self_created.self;
      assert (schedule_system_output () == 0);
    }
    else {
      assert (0);
    }
    break;
  case SENT:
    if (receipt->type == CHILD_CREATED) {
      /* Good. */
    }
    else {
      assert (0);
    }
    break;
  }
}

static bid_t
not_owner_system_output (void* state, void* param)
{
  not_owner_t* not_owner = state;
  assert (not_owner != NULL);

  bid_t bid = buffer_alloc (sizeof (order_t));
  order_t* order = buffer_write_ptr (bid);
  /* Send a create order. */
  order_create_init (order, &child_descriptor);
  not_owner->state = SENT;

  return bid;
}

descriptor_t not_owner_descriptor = {
  .constructor = not_owner_create,
  .system_input = not_owner_system_input,
  .system_output = not_owner_system_output,
};

int
main (int argc, char** argv)
{
  ueioa_run (&not_owner_descriptor, 1);
  exit (EXIT_SUCCESS);
}
