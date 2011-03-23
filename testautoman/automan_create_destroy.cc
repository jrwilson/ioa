#include <stdlib.h>
#include <assert.h>
#include <automan.h>

static const descriptor_t child_descriptor = {
};

static void automaton_created (void* state, void* param, receipt_type_t receipt);

typedef struct {
  int created_count;
  aid_t self;
  automan_t* automan;
  aid_t child;
} automaton_t;

static void*
automaton_create (const void* arg)
{
  automaton_t* automaton = malloc (sizeof (automaton_t));

  automaton->created_count = 0;
  automaton->automan = automan_creat (automaton, &automaton->self);
  automaton->child = 53;
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
    if (automaton->created_count == 0) {
      assert (automan_destroy (automaton->automan,
			       &automaton->child) == 0);
      assert (automan_create (automaton->automan,
			      &automaton->child,
			      &child_descriptor,
			      NULL,
			      automaton_created,
			      NULL) == 0);
      ++automaton->created_count;
    }
    else if (automaton->created_count == 1) {
      exit (EXIT_SUCCESS);
    }
    else {
      assert (0);
    }
  }
  else if (receipt == CHILD_DESTROYED) {
    /* Okay. */
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
