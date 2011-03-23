#include <stdlib.h>
#include <assert.h>
#include <automan.hh>

static const descriptor_t child_descriptor = {
};

static void automaton_created (void* state, void* param, receipt_type_t receipt);

typedef struct {
  aid_t self;
  automan* m_automan;
  aid_t child;
} automaton_t;

static void*
automaton_create (const void* arg)
{
  automaton_t* automaton = (automaton_t*)malloc (sizeof (automaton_t));

  automaton->m_automan = new automan (automaton, &automaton->self);
  automaton->child = -1;
  assert (automaton->m_automan->create (&automaton->child,
					&child_descriptor,
					NULL,
					automaton_created,
					NULL) == 0);
  return automaton;
}

static void
automaton_system_input (void* state, void* param, bid_t bid)
{
  automaton_t* automaton = (automaton_t*)state;
  assert (automaton != NULL);

  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));
  const receipt_t* receipt = (const receipt_t*)buffer_read_ptr (bid);

  automaton->m_automan->apply (*receipt);
}

static bid_t
automaton_system_output (void* state, void* param)
{
  automaton_t* automaton = (automaton_t*)state;
  assert (automaton != NULL);

  return automaton->m_automan->action ();
}

static void
automaton_created (void* state, void* param, receipt_type_t receipt)
{
  automaton_t* automaton = (automaton_t*)state;
  assert (automaton != NULL);
  assert (receipt == CHILD_CREATED);
  assert (automaton->child != -1);

  exit (EXIT_SUCCESS);
}

static const descriptor_t automaton_descriptor = {
  automaton_create,
  automaton_system_input,
  automaton_system_output,
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
  ueioa_run (&automaton_descriptor, NULL, 1);
  exit (EXIT_SUCCESS);
}
