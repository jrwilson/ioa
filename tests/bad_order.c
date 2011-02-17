#include <stdlib.h>
#include <assert.h>
#include <ueioa.h>

typedef enum {
  UNSENT,
  SENT
} bad_order_state_t;

typedef struct {
  bad_order_state_t state;
} bad_order_t;

static void*
bad_order_create (void)
{
  bad_order_t* bad_order = malloc (sizeof (bad_order_t));
  bad_order->state = UNSENT;

  return bad_order;
}

static void
bad_order_system_input (void* state, void* param, bid_t bid)
{
  bad_order_t* bad_order = state;
  assert (bad_order != NULL);

  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));
  const receipt_t* receipt = buffer_read_ptr (bid);

  switch (bad_order->state) {
  case UNSENT:
    if (receipt->type == SELF_CREATED) {
      assert (schedule_system_output () == 0);
    }
    else {
      assert (0);
    }
    break;
  case SENT:
    if (receipt->type == BAD_ORDER) {
      exit (EXIT_SUCCESS);
    }
    else {
      assert (0);
    }
    break;
  }
}

static bid_t
bad_order_system_output (void* state, void* param)
{
  bad_order_t* bad_order = state;
  assert (bad_order != NULL);

  bid_t bid = buffer_alloc (sizeof (order_t));
  order_t* order = buffer_write_ptr (bid);
  /* Send a bad order. */
  order->type = -1;
  bad_order->state = SENT;

  return bid;
}

descriptor_t bad_order_descriptor = {
  .constructor = bad_order_create,
  .system_input = bad_order_system_input,
  .system_output = bad_order_system_output,
  .alarm_input = NULL,
  .read_input = NULL,
  .write_input = NULL,
  .free_inputs = NULL,
  .inputs = NULL,
  .outputs = NULL,
  .internals = NULL,
};

int
main (int argc, char** argv)
{
  ueioa_run (&bad_order_descriptor, 1);
  exit (EXIT_SUCCESS);
}
