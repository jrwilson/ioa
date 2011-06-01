#include <stdlib.h>
#include <assert.h>
#include <ueioa.hh>

typedef enum {
  UNSENT,
  SENT
} bad_order_state_t;

typedef struct {
  bad_order_state_t state;
} bad_order_t;

static void*
bad_order_create (const void* arg)
{
  bad_order_t* bad_order = (bad_order_t*)malloc (sizeof (bad_order_t));
  bad_order->state = UNSENT;

  return bad_order;
}

static void
bad_order_system_input (void* state, void* param, bid_t bid)
{
  bad_order_t* bad_order = (bad_order_t*)state;
  assert (bad_order != NULL);

  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));
  const receipt_t* receipt = (const receipt_t*)buffer_read_ptr (bid);

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
  bad_order_t* bad_order = (bad_order_t*)state;
  assert (bad_order != NULL);

  bid_t bid = buffer_alloc (sizeof (order_t));
  order_t* order = (order_t*)buffer_write_ptr (bid);
  /* Send a bad order. */
  order->type = order_type_t(-1);
  bad_order->state = SENT;

  return bid;
}

descriptor_t bad_order_descriptor = {
  bad_order_create,
  bad_order_system_input,
  bad_order_system_output,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL
};

int
main (int argc, char** argv)
{
  ueioa_run (&bad_order_descriptor, NULL, 1);
  exit (EXIT_SUCCESS);
}
