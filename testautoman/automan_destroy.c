#include <stdlib.h>
#include <assert.h>
#include <automan.h>

static const descriptor_t child_descriptor = {
};

static void automaton_created (void* state, void* param, receipt_type_t receipt);

typedef enum {
  CREATE_RECV,
  DESTROY_RECV
} state_t;

typedef struct {
  state_t state;
  aid_t self;
  automan_t* automan;
  aid_t child;
} automaton_t;

static void*
automaton_create (const void* arg)
{
  automaton_t* automaton = malloc (sizeof (automaton_t));

  automaton->state = CREATE_RECV;
  automaton->automan = automan_creat (automaton, &automaton->self);
  automaton->child = -1;
  assert (automan_create (automaton->automan,
			  &automaton->child,
			  &child_descriptor,
			  NULL,
			  automaton_created,
			  NULL) == 0);
  return automaton;
}

static void
automaton_system_input (void* state, void* param, bid_t bid)
{
  automaton_t* automaton = state;
  assert (automaton != NULL);

  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));
  const receipt_t* receipt = buffer_read_ptr (bid);

  automan_apply (automaton->automan, receipt);
}

static bid_t
automaton_system_output (void* state, void* param)
{
  automaton_t* automaton = state;
  assert (automaton != NULL);

  return automan_action (automaton->automan);
}

static void
automaton_created (void* state, void* param, receipt_type_t receipt)
{
  automaton_t* automaton = state;
  assert (automaton != NULL);

  switch (automaton->state) {
  case CREATE_RECV:
    assert (receipt == CHILD_CREATED);
    assert (automaton->child != -1);
    assert (automan_destroy (automaton->automan,
			     &automaton->child) == 0);
    assert (schedule_system_output () == 0);
    automaton->state = DESTROY_RECV;
    break;
  case DESTROY_RECV:
    assert (receipt == CHILD_DESTROYED);
    assert (automaton->child == -1);
    exit (EXIT_SUCCESS);
    break;
  }
}

static const descriptor_t automaton_descriptor = {
  .constructor = automaton_create,
  .system_input = automaton_system_input,
  .system_output = automaton_system_output,
};

int
main (int argc, char** argv)
{
  ueioa_run (&automaton_descriptor, NULL, 1);
  exit (EXIT_SUCCESS);
}
