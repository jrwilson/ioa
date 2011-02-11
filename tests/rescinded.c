#include <stdlib.h>
#include <assert.h>
#include <ueioa.h>

typedef enum {
  START,
  DECLARE_UNSENT,
  DECLARE_SENT,
  RESCIND_UNSENT,
  RESCIND_SENT
} rescinded_state_t;

typedef struct {
  rescinded_state_t state;
} rescinded_t;

static void*
rescinded_create (void)
{
  rescinded_t* rescinded = malloc (sizeof (rescinded_t));
  rescinded->state = START;

  return rescinded;
}

static void
rescinded_system_input (void* state, void* param, bid_t bid)
{
  rescinded_t* rescinded = state;
  assert (rescinded != NULL);

  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));
  const receipt_t* receipt = buffer_read_ptr (bid);

  switch (rescinded->state) {
  case START:
    switch (receipt->type) {
    case SELF_CREATED:
      rescinded->state = DECLARE_UNSENT;
      schedule_system_output ();
      break;
    case BAD_ORDER:
    case CHILD_CREATED:
    case BAD_DESCRIPTOR:
    case DECLARED:
    case OUTPUT_DNE:
    case INPUT_DNE:
    case OUTPUT_UNAVAILABLE:
    case INPUT_UNAVAILABLE:
    case COMPOSED:
    case INPUT_COMPOSED:
    case OUTPUT_COMPOSED:
    case NOT_COMPOSER:
    case NOT_COMPOSED:
    case DECOMPOSED:
    case INPUT_DECOMPOSED:
    case OUTPUT_DECOMPOSED:
    case RESCINDED:
    case AUTOMATON_DNE:
    case NOT_OWNER:
    case CHILD_DESTROYED:
      assert (0);
    }
    break;
  case DECLARE_UNSENT:
    assert (0);
    break;
  case DECLARE_SENT:
    switch (receipt->type) {
    case DECLARED:
      rescinded->state = RESCIND_UNSENT;
      schedule_system_output ();
      break;
    case BAD_ORDER:
    case SELF_CREATED:
    case CHILD_CREATED:
    case BAD_DESCRIPTOR:
    case OUTPUT_DNE:
    case INPUT_DNE:
    case OUTPUT_UNAVAILABLE:
    case INPUT_UNAVAILABLE:
    case COMPOSED:
    case INPUT_COMPOSED:
    case OUTPUT_COMPOSED:
    case NOT_COMPOSER:
    case NOT_COMPOSED:
    case DECOMPOSED:
    case INPUT_DECOMPOSED:
    case OUTPUT_DECOMPOSED:
    case RESCINDED:
    case AUTOMATON_DNE:
    case NOT_OWNER:
    case CHILD_DESTROYED:
      assert (0);
    }
    break;
  case RESCIND_UNSENT:
    assert (0);
    break;
  case RESCIND_SENT:
    switch (receipt->type) {
    case RESCINDED:
      exit (EXIT_SUCCESS);
      break;
    case BAD_ORDER:
    case SELF_CREATED:
    case CHILD_CREATED:
    case BAD_DESCRIPTOR:
    case DECLARED:
    case OUTPUT_DNE:
    case INPUT_DNE:
    case OUTPUT_UNAVAILABLE:
    case INPUT_UNAVAILABLE:
    case COMPOSED:
    case INPUT_COMPOSED:
    case OUTPUT_COMPOSED:
    case NOT_COMPOSER:
    case NOT_COMPOSED:
    case DECOMPOSED:
    case INPUT_DECOMPOSED:
    case OUTPUT_DECOMPOSED:
    case AUTOMATON_DNE:
    case NOT_OWNER:
    case CHILD_DESTROYED:
      assert (0);
    }
    break;
  }
}

static bid_t
rescinded_system_output (void* state, void* param)
{
  rescinded_t* rescinded = state;
  assert (rescinded != NULL);

  bid_t bid = buffer_alloc (sizeof (order_t));
  order_t* order = buffer_write_ptr (bid);

  switch (rescinded->state) {
  case START:
    assert (0);
    break;
  case DECLARE_UNSENT:
    /* Declare a paramter. */
    order_declare_init (order, (void*)567);
    rescinded->state = DECLARE_SENT;
    break;
  case DECLARE_SENT:
    assert (0);
    break;
  case RESCIND_UNSENT:
    /* Rescind a paramter. */
    order_rescind_init (order, (void*)567);
    rescinded->state = RESCIND_SENT;
    break;
  case RESCIND_SENT:
    assert (0);
    break;
  }

  return bid;
}

static input_t rescinded_inputs[] = { NULL };
static output_t rescinded_outputs[] = { NULL };
static internal_t rescinded_internals[] = { NULL };

descriptor_t rescinded_descriptor = {
  .constructor = rescinded_create,
  .system_input = rescinded_system_input,
  .system_output = rescinded_system_output,
  .inputs = rescinded_inputs,
  .outputs = rescinded_outputs,
  .internals = rescinded_internals,
};

int
main (int argc, char** argv)
{
  ueioa_run (&rescinded_descriptor);
  exit (EXIT_SUCCESS);
}