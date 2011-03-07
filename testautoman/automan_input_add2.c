#include <stdlib.h>
#include <assert.h>
#include <automan.h>

static void child_input (void* state, void* param, bid_t bid);
static void child_composed (void* state, void* param, receipt_type_t receipt);

typedef struct {
  aid_t self;
  automan_t* automan;
  bool composed;
} child_t;

static void*
child_create (const void* arg)
{
  child_t* child = malloc (sizeof (child_t));

  child->automan = automan_creat (child, &child->self);
  assert (automan_input_add (child->automan,
			     &child->composed,
			     child_input,
			     NULL,
			     child_composed,
			     NULL) == 0);

  return child;
}

static void
child_system_input (void* state, void* param, bid_t bid)
{
  child_t* child = state;
  assert (child != NULL);

  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));
  const receipt_t* receipt = buffer_read_ptr (bid);

  automan_apply (child->automan, receipt);
}

static bid_t
child_system_output (void* state, void* param)
{
  child_t* child = state;
  assert (child != NULL);

  return automan_action (child->automan);
}

static void
child_input (void* state, void* param, bid_t bid)
{
}

static bid_t
child_output (void* state, void* param)
{
  return -1;
}

static void
child_composed (void* state, void* param, receipt_type_t receipt)
{
  child_t* child = state;
  assert (child != NULL);

  if (receipt == INPUT_COMPOSED) {
    assert (child->composed);
  }
  else if (receipt == INPUT_DECOMPOSED) {
    assert (!child->composed);
    exit (EXIT_SUCCESS);
  }
  else {
    exit (EXIT_FAILURE);
  }
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
  .constructor = child_create,
  .system_input = child_system_input,
  .system_output = child_system_output,
  .inputs = child_inputs,
  .outputs = child_outputs,
};

static void automaton_composed (void* state, void* param, receipt_type_t receipt);

typedef struct {
  aid_t self;
  automan_t* automan;
  bool composed;
  aid_t child1;
  aid_t child2;
} automaton_t;

static void*
automaton_create (const void* arg)
{
  automaton_t* automaton = malloc (sizeof (automaton_t));

  automaton->automan = automan_creat (automaton, &automaton->self);
  automaton->composed = false;
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

  if (receipt == COMPOSED) {
    assert (automaton->composed);
    assert (automan_decompose (automaton->automan,
			       &automaton->composed) == 0);
  }
  else if (receipt == DECOMPOSED) {
    assert (!automaton->composed);
  }
  else {
    exit (EXIT_FAILURE);
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
