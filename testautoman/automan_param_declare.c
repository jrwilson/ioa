#include <stdlib.h>
#include <assert.h>
#include <automan.h>

static void automaton_param_declared (void* state, void* param);

typedef struct {
  aid_t self;
  void* param;
  automan_t* automan;
} automaton_t;

static void*
automaton_create (const void* arg)
{
  automaton_t* automaton = malloc (sizeof (automaton_t));

  automaton->param = malloc (64);
  automaton->automan = automan_create (&automaton->self);
  assert (automan_param_declare (automaton->automan,
				 automaton->param,
				 automaton_param_declared) == 0);
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
automaton_param_declared (void* state, void* param)
{
  automaton_t* automaton = state;
  assert (automaton != NULL);

  if (automaton->param == param) {
    exit (EXIT_SUCCESS);
  }
  else {
    exit (EXIT_FAILURE);
  }
}

static const internal_t automaton_internals[] = {
  automaton_param_declared,
  NULL
};

static const descriptor_t automaton_descriptor = {
  .constructor = automaton_create,
  .system_input = automaton_system_input,
  .system_output = automaton_system_output,
  .internals = automaton_internals,
};

int
main (int argc, char** argv)
{
  ueioa_run (&automaton_descriptor, NULL, 1);
  exit (EXIT_SUCCESS);
}
