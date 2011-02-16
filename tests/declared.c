#include <stdlib.h>
#include <assert.h>
#include <ueioa.h>

typedef enum {
  UNSENT,
  SENT
} declared_state_t;

typedef struct {
  declared_state_t state;
} declared_t;

static void*
declared_create (void)
{
  declared_t* declared = malloc (sizeof (declared_t));
  declared->state = UNSENT;

  return declared;
}

static void
declared_system_input (void* state, void* param, bid_t bid)
{
  declared_t* declared = state;
  assert (declared != NULL);

  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));
  const receipt_t* receipt = buffer_read_ptr (bid);

  switch (declared->state) {
  case UNSENT:
    if (receipt->type == SELF_CREATED) {
      schedule_system_output ();
    }
    else {
      assert (0);
    }
    break;
  case SENT:
    if (receipt->type == DECLARED) {
      exit (EXIT_SUCCESS);
    }
    else {
      assert (0);
    }
    break;
  }
}

static bid_t
declared_system_output (void* state, void* param)
{
  declared_t* declared = state;
  assert (declared != NULL);

  bid_t bid = buffer_alloc (sizeof (order_t));
  order_t* order = buffer_write_ptr (bid);
  /* Declare a paramter. */
  order_declare_init (order, (void*)567);
  declared->state = SENT;

  return bid;
}

static input_t declared_free_inputs[] = { NULL };
static input_t declared_inputs[] = { NULL };
static output_t declared_outputs[] = { NULL };
static internal_t declared_internals[] = { NULL };

descriptor_t declared_descriptor = {
  .constructor = declared_create,
  .system_input = declared_system_input,
  .system_output = declared_system_output,
  .free_inputs = declared_free_inputs,
  .inputs = declared_inputs,
  .outputs = declared_outputs,
  .internals = declared_internals,
};

int
main (int argc, char** argv)
{
  ueioa_run (&declared_descriptor, 1);
  exit (EXIT_SUCCESS);
}
