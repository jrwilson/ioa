#include <stdlib.h>
#include <assert.h>
#include <automan.h>

static void automaton_param_declared (void* state, void* param, receipt_type_t receipt);

typedef enum {
  DECLARE_RECV,
  RESCIND_RECV,
} state_t;

typedef struct {
  state_t state;
  aid_t self;
  bool declared;
  void* param;
  automan_t* automan;
} automaton_t;

static void*
automaton_create (const void* arg)
{
  automaton_t* automaton = malloc (sizeof (automaton_t));

  automaton->state = DECLARE_RECV;
  automaton->declared = false;
  automaton->param = malloc (64);
  automaton->automan = automan_creat (automaton, &automaton->self);
  assert (automan_declare (automaton->automan,
			   &automaton->declared,
			   automaton->param,
			   automaton_param_declared,
			   automaton->param) == 0);
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
automaton_param_declared (void* state, void* param, receipt_type_t receipt)
{
  automaton_t* automaton = state;
  assert (automaton != NULL);
  assert (automaton->param == param);

  switch (automaton->state) {
  case DECLARE_RECV:
    assert (receipt == DECLARED);
    assert (automaton->declared);
    assert (automan_rescind (automaton->automan,
			     &automaton->declared) == 0);
    assert (schedule_system_output () == 0);
    automaton->state = RESCIND_RECV;
    break;
  case RESCIND_RECV:
    assert (receipt == RESCINDED);
    assert (!automaton->declared);
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
