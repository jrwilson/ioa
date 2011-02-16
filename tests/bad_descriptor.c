#include <stdlib.h>
#include <assert.h>
#include <ueioa.h>

#include "test.h"

typedef enum {
  UNSENT,
  SENT
} bad_descriptor_state_t;

typedef struct {
  bad_descriptor_state_t state;
} bad_descriptor_t;

static void*
bad_descriptor_create (void)
{
  bad_descriptor_t* bad_descriptor = malloc (sizeof (bad_descriptor_t));
  bad_descriptor->state = UNSENT;

  return bad_descriptor;
}

static void
bad_descriptor_system_input (void* state, void* param, bid_t bid)
{
  bad_descriptor_t* bad_descriptor = state;
  assert (bad_descriptor != NULL);

  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));
  const receipt_t* receipt = buffer_read_ptr (bid);

  switch (bad_descriptor->state) {
  case UNSENT:
    if (receipt->type == SELF_CREATED) {
      schedule_system_output ();
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
  bad_descriptor_t* bad_descriptor = state;
  assert (bad_descriptor != NULL);

  bid_t bid = buffer_alloc (sizeof (order_t));
  order_t* order = buffer_write_ptr (bid);
  /* Send a create order. */
  order_create_init (order, NULL);
  bad_descriptor->state = SENT;

  return bid;
}

static input_t bad_descriptor_free_inputs[] = { NULL };
static input_t bad_descriptor_inputs[] = { NULL };
static output_t bad_descriptor_outputs[] = { NULL };
static internal_t bad_descriptor_internals[] = { NULL };

descriptor_t bad_descriptor_descriptor = {
  .constructor = bad_descriptor_create,
  .system_input = bad_descriptor_system_input,
  .system_output = bad_descriptor_system_output,
  .alarm_input = test_alarm_input,
  .read_input = test_read_input,
  .write_input = test_write_input,
  .free_inputs = bad_descriptor_free_inputs,
  .inputs = bad_descriptor_inputs,
  .outputs = bad_descriptor_outputs,
  .internals = bad_descriptor_internals,
};

int
main (int argc, char** argv)
{
  ueioa_run (&bad_descriptor_descriptor, 1);
  exit (EXIT_SUCCESS);
}
