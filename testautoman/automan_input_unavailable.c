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

static void composer_composed (void* state, void* param, receipt_type_t receipt);

typedef struct {
  aid_t* child2;
} composer_arg_t;

typedef struct {
  aid_t self;
  automan_t* automan;
  aid_t child1;
  aid_t* child2;
  bool composed;
} composer_t;

static void*
composer_create (const void* a)
{
  const composer_arg_t* arg = a;
  assert (arg != NULL);
  assert (arg->child2 != NULL);

  composer_t* composer = malloc (sizeof (composer_t));
  composer->automan = automan_creat (composer, &composer->self);

  assert (automan_create (composer->automan,
			  &composer->child1,
			  &child_descriptor,
			  NULL,
			  NULL,
			  NULL) == 0);
  composer->child2 = arg->child2;
  assert (automan_compose (composer->automan,
  			   &composer->composed,
  			   &composer->child1,
  			   child_output,
  			   NULL,
  			   composer->child2,
  			   child_input,
  			   NULL,
  			   composer_composed,
  			   NULL) == 0);

  return composer;
}

static void
composer_system_input (void* state, void* param, bid_t bid)
{
  composer_t* composer = state;
  assert (composer != NULL);

  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));
  const receipt_t* receipt = buffer_read_ptr (bid);

  automan_apply (composer->automan, receipt);
}

static bid_t
composer_system_output (void* state, void* param)
{
  composer_t* composer = state;
  assert (composer != NULL);

  return automan_action (composer->automan);
}

static void
composer_composed (void* state, void* param, receipt_type_t receipt)
{
  composer_t* composer = state;
  assert (composer != NULL);
  assert (receipt == INPUT_UNAVAILABLE);
  assert (!composer->composed);

  exit (EXIT_SUCCESS);
}

static const descriptor_t composer_descriptor = {
  .constructor = composer_create,
  .system_input = composer_system_input,
  .system_output = composer_system_output,
};

static void automaton_composed (void* state, void* param, receipt_type_t receipt);

typedef struct {
  aid_t self;
  automan_t* automan;
  bool composed;
  aid_t child1;
  aid_t child2;
  composer_arg_t composer_arg;
  aid_t composer;
} automaton_t;

static void*
automaton_create (const void* arg)
{
  automaton_t* automaton = malloc (sizeof (automaton_t));

  automaton->automan = automan_creat (automaton, &automaton->self);
  assert (automan_create (automaton->automan,
			  &automaton->child1,
			  &child_descriptor,
			  NULL,
			  NULL,
			  NULL) == 0);
  assert (automan_create (automaton->automan,
			  &automaton->child2,
			  &child_descriptor,
			  NULL,
			  NULL,
			  NULL) == 0);
  assert (automan_compose (automaton->automan,
			   &automaton->composed,
			   &automaton->child1,
			   child_output,
			   NULL,
			   &automaton->child2,
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

static void
automaton_composed (void* state, void* param, receipt_type_t receipt)
{
  automaton_t* automaton = state;
  assert (automaton != NULL);
  assert (receipt == COMPOSED);
  assert (automaton->composed);

  /* Create another automaton that will attempt to compose with the input of child2. */
  automaton->composer_arg.child2 = &automaton->child2;
  assert (automan_create (automaton->automan,
			  &automaton->composer,
			  &composer_descriptor,
			  &automaton->composer_arg,
			  NULL,
			  NULL) == 0);
  assert (schedule_system_output () == 0);
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
