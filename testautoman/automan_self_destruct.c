#include <stdlib.h>
#include <assert.h>
#include <automan.h>

typedef struct {
  aid_t self;
  automan_t* automan;
} child_t;

static void*
child_create (const void* arg)
{
  child_t* child = malloc (sizeof (child_t));

  child->automan = automan_creat (child, &child->self);
  automan_self_destruct (child->automan);

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

static const descriptor_t child_descriptor = {
  .constructor = child_create,
  .system_input = child_system_input,
  .system_output = child_system_output,
};

static void automaton_created (void* state, void* param, receipt_type_t receipt);

typedef struct {
  aid_t self;
  automan_t* automan;
  aid_t child;
} automaton_t;

static void*
automaton_create (const void* arg)
{
  automaton_t* automaton = malloc (sizeof (automaton_t));

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

  if (receipt == CHILD_CREATED) {
    assert (automaton->child != -1);
  }
  else if (receipt == CHILD_DESTROYED) {
    exit (EXIT_SUCCESS);
  }
  else {
    assert (0);
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
