#include <assert.h>
#include <stdlib.h>

#include <ueioa.h>

static void*
timer_create (void)
{
  return NULL;
}

static bid_t timer_system_output (void* state, void* param);

static void
timer_system_input (void* state, void* param, bid_t bid)
{
  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));
  const receipt_t* receipt = buffer_read_ptr (bid);

  switch (receipt->type) {
  case SELF_CREATED:
    schedule_system_output ();
    break;
  case WAKEUP:
    exit (EXIT_SUCCESS);
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
}

static bid_t
timer_system_output (void* state, void* param)
{
  bid_t bid = buffer_alloc (sizeof (order_t));
  order_t* order = buffer_write_ptr (bid);
  order_set_alarm_init (order, 1, 0);
  return bid;
}

static input_t timer_inputs[] = { NULL };
static output_t timer_outputs[] = { NULL };
static internal_t timer_internals[] = { NULL };

descriptor_t timer_descriptor = {
  .constructor = timer_create,
  .system_input = timer_system_input,
  .system_output = timer_system_output,
  .inputs = timer_inputs,
  .outputs = timer_outputs,
  .internals = timer_internals,
};


int
main (int argc, char** argv)
{
  ueioa_run (&timer_descriptor, 1);

  exit (EXIT_SUCCESS);
}
