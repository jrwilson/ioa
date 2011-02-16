#include <stdlib.h>
#include <assert.h>
#include <ueioa.h>

#include "test.h"

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
      schedule_system_output ();
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

static input_t bad_order_free_inputs[] = { NULL };
static input_t bad_order_inputs[] = { NULL };
static output_t bad_order_outputs[] = { NULL };
static internal_t bad_order_internals[] = { NULL };

descriptor_t bad_order_descriptor = {
  .constructor = bad_order_create,
  .system_input = bad_order_system_input,
  .system_output = bad_order_system_output,
  .alarm_input = test_alarm_input,
  .read_input = test_read_input,
  .write_input = test_write_input,
  .free_inputs = bad_order_free_inputs,
  .inputs = bad_order_inputs,
  .outputs = bad_order_outputs,
  .internals = bad_order_internals,
};

int
main (int argc, char** argv)
{
  ueioa_run (&bad_order_descriptor, 1);
  exit (EXIT_SUCCESS);
}
