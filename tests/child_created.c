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

static input_t child_free_inputs[] = { NULL };
static input_t child_inputs[] = { NULL };
static output_t child_outputs[] = { NULL };
static internal_t child_internals[] = { NULL };

descriptor_t child_descriptor = {
  .constructor = child_create,
  .system_input = child_system_input,
  .system_output = child_system_output,
  .free_inputs = child_free_inputs,
  .inputs = child_inputs,
  .outputs = child_outputs,
  .internals = child_internals,
};


typedef enum {
  UNSENT,
  SENT
} child_created_state_t;

typedef struct {
  child_created_state_t state;
} child_created_t;

static void*
child_created_create (void)
{
  child_created_t* child_created = malloc (sizeof (child_created_t));
  child_created->state = UNSENT;

  return child_created;
}

static void
child_created_system_input (void* state, void* param, bid_t bid)
{
  child_created_t* child_created = state;
  assert (child_created != NULL);

  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));
  const receipt_t* receipt = buffer_read_ptr (bid);

  switch (child_created->state) {
  case UNSENT:
    if (receipt->type == SELF_CREATED) {
      schedule_system_output ();
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
  child_created_t* child_created = state;
  assert (child_created != NULL);

  bid_t bid = buffer_alloc (sizeof (order_t));
  order_t* order = buffer_write_ptr (bid);
  /* Send a create order. */
  order_create_init (order, &child_descriptor);
  child_created->state = SENT;

  return bid;
}

static input_t child_created_inputs[] = { NULL };
static input_t child_created_free_inputs[] = { NULL };
static output_t child_created_outputs[] = { NULL };
static internal_t child_created_internals[] = { NULL };

descriptor_t child_created_descriptor = {
  .constructor = child_created_create,
  .system_input = child_created_system_input,
  .system_output = child_created_system_output,
  .free_inputs = child_created_free_inputs,
  .inputs = child_created_inputs,
  .outputs = child_created_outputs,
  .internals = child_created_internals,
};

int
main (int argc, char** argv)
{
  ueioa_run (&child_created_descriptor, 1);
  exit (EXIT_SUCCESS);
}
