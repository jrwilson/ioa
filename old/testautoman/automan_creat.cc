#include <stdlib.h>
#include <assert.h>
#include <automan.hh>

typedef struct {
  aid_t self;
  automan* m_automan;
} automaton_t;

static void*
automaton_create (const void* arg)
{
  automaton_t* automaton = (automaton_t*)malloc (sizeof (automaton_t));

  automaton->m_automan = new automan (automaton, &automaton->self);

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

  if (receipt->type == SELF_CREATED) {
    if (automaton->self == receipt->self_created.self) {
      exit (EXIT_SUCCESS);
    }
    else {
      exit (EXIT_FAILURE);
    }
  }
}

static bid_t
automaton_system_output (void* state, void* param)
{
  automaton_t* automaton = (automaton_t*)state;
  assert (automaton != NULL);

  return automaton->m_automan->action ();
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
