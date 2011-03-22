#include <stdlib.h>
#include <assert.h>
#include <ueioa.h>

typedef enum {
  UNSENT,
  SENT
} bad_descriptor_state_t;

typedef struct {
  bad_descriptor_state_t state;
} bad_descriptor_t;

static void*
bad_descriptor_create (const void* arg)
{
  bad_descriptor_t* bad_descriptor = (bad_descriptor_t*)malloc (sizeof (bad_descriptor_t));
  bad_descriptor->state = UNSENT;

  return bad_descriptor;
}

static void
bad_descriptor_system_input (void* state, void* param, bid_t bid)
{
  bad_descriptor_t* bad_descriptor = (bad_descriptor_t*)state;
  assert (bad_descriptor != NULL);

  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));
  const receipt_t* receipt = (const receipt_t*)buffer_read_ptr (bid);

  switch (bad_descriptor->state) {
  case UNSENT:
    if (receipt->type == SELF_CREATED) {
      assert (schedule_system_output () == 0);
    }
    else {
      assert (0);
    }
    break;
  case SENT:
    if (receipt->type == BAD_DESCRIPTOR) {
      exit (EXIT_SUCCESS);
    }
    else {
      assert (0);
    }
    break;
  }
}

static bid_t
bad_descriptor_system_output (void* state, void* param)
{
  bad_descriptor_t* bad_descriptor = (bad_descriptor_t*)state;
  assert (bad_descriptor != NULL);

  bid_t bid = buffer_alloc (sizeof (order_t));
  order_t* order = (order_t*)buffer_write_ptr (bid);
  /* Send a create order. */
  order_create_init (order, NULL, NULL);
  bad_descriptor->state = SENT;

  return bid;
}

descriptor_t bad_descriptor_descriptor = {
   bad_descriptor_create,
   bad_descriptor_system_input,
   bad_descriptor_system_output,
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
  ueioa_run (&bad_descriptor_descriptor, NULL, 1);
  exit (EXIT_SUCCESS);
}
