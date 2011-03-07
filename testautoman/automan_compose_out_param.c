#include <stdlib.h>
#include <assert.h>
#include <automan.h>

static void
child_input (void* state, void* param, bid_t bid)
{
}

static bid_t
child_output (void* state, void* param)
{
  return -1;
}

static const input_t child_inputs[] = {
  child_input,
  NULL
};

static const output_t child_outputs[] = {
  child_output,
  NULL
};

static const descriptor_t child_descriptor = {
  .inputs = child_inputs,
  .outputs = child_outputs,
};

static bid_t automaton_output (void* state, void* param);
static void automaton_composed (void* state, void* param, receipt_type_t receipt);

typedef struct {
  aid_t self;
  bool declared;
  void* param;
  automan_t* automan;
  bool composed;
  aid_t child;
} automaton_t;

static void*
automaton_create (const void* arg)
{
  automaton_t* automaton = malloc (sizeof (automaton_t));

  automaton->param = malloc (64);

  automaton->automan = automan_creat (automaton, &automaton->self);
  assert (automan_declare (automaton->automan,
			   &automaton->declared,
			   automaton->param,
			   NULL,
			   NULL) == 0);
  assert (automan_create (automaton->automan,
			  &automaton->child,
			  &child_descriptor,
			  NULL,
			  NULL,
			  NULL) == 0);
  assert (automan_compose (automaton->automan,
  			   &automaton->composed,
  			   &automaton->self,
  			   automaton_output,
  			   automaton->param,
  			   &automaton->child,
  			   child_input,
  			   NULL,
  			   automaton_composed,
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

static bid_t
automaton_output (void* state, void* param)
{
  return -1;
}

static void
automaton_composed (void* state, void* param, receipt_type_t receipt)
{
  automaton_t* automaton = state;
  assert (automaton != NULL);
  assert (receipt == COMPOSED);

  if (automaton->composed) {
    exit (EXIT_SUCCESS);
  }
  else {
    exit (EXIT_FAILURE);
  }
}

static const output_t automaton_outputs[] = {
  automaton_output,
  NULL
};

static const descriptor_t automaton_descriptor = {
  .constructor = automaton_create,
  .system_input = automaton_system_input,
  .system_output = automaton_system_output,
  .outputs = automaton_outputs,
};

int
main (int argc, char** argv)
{
  ueioa_run (&automaton_descriptor, NULL, 1);
  exit (EXIT_SUCCESS);
}
