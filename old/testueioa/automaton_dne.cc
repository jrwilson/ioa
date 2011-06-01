#include <stdlib.h>
#include <assert.h>
#include <ueioa.hh>

typedef enum {
  UNSENT,
  SENT
} automaton_dne_state_t;

typedef struct {
  automaton_dne_state_t state;
} automaton_dne_t;

static void*
automaton_dne_create (const void* arg)
{
  automaton_dne_t* automaton_dne = (automaton_dne_t*)malloc (sizeof (automaton_dne_t));
  automaton_dne->state = UNSENT;

  return automaton_dne;
}

static void
automaton_dne_system_input (void* state, void* param, bid_t bid)
{
  automaton_dne_t* automaton_dne = (automaton_dne_t*)state;
  assert (automaton_dne != NULL);

  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));
  const receipt_t* receipt = (const receipt_t*)buffer_read_ptr (bid);

  switch (automaton_dne->state) {
  case UNSENT:
    if (receipt->type == SELF_CREATED) {
      assert (schedule_system_output () == 0);
    }
    else {
      assert (0);
    }
    break;
  case SENT:
    if (receipt->type == AUTOMATON_DNE) {
      exit (EXIT_SUCCESS);
    }
    else {
      assert (0);
    }
    break;
  }
}

static bid_t
automaton_dne_system_output (void* state, void* param)
{
  automaton_dne_t* automaton_dne = (automaton_dne_t*)state;
  assert (automaton_dne != NULL);

  bid_t bid = buffer_alloc (sizeof (order_t));
  order_t* order = (order_t*)buffer_write_ptr (bid);
  /* Try to destroy an automaton that doesn't exist. */
  order_destroy_init (order, -1);
  automaton_dne->state = SENT;

  return bid;
}

descriptor_t automaton_dne_descriptor = {
  automaton_dne_create,
  automaton_dne_system_input,
  automaton_dne_system_output,
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
  ueioa_run (&automaton_dne_descriptor, NULL, 1);
  exit (EXIT_SUCCESS);
}
