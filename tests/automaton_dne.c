#include <stdlib.h>
#include <assert.h>
#include <ueioa.h>

typedef enum {
  UNSENT,
  SENT
} automaton_dne_state_t;

typedef struct {
  automaton_dne_state_t state;
} automaton_dne_t;

static void*
automaton_dne_create (void)
{
  automaton_dne_t* automaton_dne = malloc (sizeof (automaton_dne_t));
  automaton_dne->state = UNSENT;

  return automaton_dne;
}

static void
automaton_dne_system_input (void* state, void* param, bid_t bid)
{
  automaton_dne_t* automaton_dne = state;
  assert (automaton_dne != NULL);

  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));
  const receipt_t* receipt = buffer_read_ptr (bid);

  switch (automaton_dne->state) {
  case UNSENT:
    switch (receipt->type) {
    case SELF_CREATED:
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
  case SENT:
    switch (receipt->type) {
    case AUTOMATON_DNE:
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
    case NOT_COMPOSED:
    case DECOMPOSED:
    case INPUT_DECOMPOSED:
    case OUTPUT_DECOMPOSED:
    case RESCINDED:
    case NOT_OWNER:
    case CHILD_DESTROYED:
      assert (0);
    }
    break;
  }
}

static bid_t
automaton_dne_system_output (void* state, void* param)
{
  automaton_dne_t* automaton_dne = state;
  assert (automaton_dne != NULL);

  bid_t bid = buffer_alloc (sizeof (order_t));
  order_t* order = buffer_write_ptr (bid);
  /* Try to destroy an automaton that doesn't exist. */
  order_destroy_init (order, -1);
  automaton_dne->state = SENT;

  return bid;
}

static input_t automaton_dne_inputs[] = { NULL };
static output_t automaton_dne_outputs[] = { NULL };
static internal_t automaton_dne_internals[] = { NULL };

descriptor_t automaton_dne_descriptor = {
  .constructor = automaton_dne_create,
  .system_input = automaton_dne_system_input,
  .system_output = automaton_dne_system_output,
  .inputs = automaton_dne_inputs,
  .outputs = automaton_dne_outputs,
  .internals = automaton_dne_internals,
};

int
main (int argc, char** argv)
{
  ueioa_run (&automaton_dne_descriptor, 1);
  exit (EXIT_SUCCESS);
}
