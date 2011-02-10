#include "composer.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include <ueioa.h>

#include "manager.h"
#include "trigger.h"
#include "counter.h"

typedef struct {
  manager_t* manager;
  aid_t self;
  aid_t parent;

  aid_t trigger;
  aid_t counter1;
  aid_t counter2;
} composer_t;

static void*
composer_create (void)
{
  printf ("composer_create\n");
  composer_t* composer = malloc (sizeof (composer_t));

  composer->manager = manager_create ();
  manager_self_set (composer->manager, &composer->self);
  manager_parent_set (composer->manager, &composer->parent);
  manager_automaton_add (composer->manager, &composer->trigger, &trigger_descriptor);
  manager_automaton_add (composer->manager, &composer->counter1, &counter_descriptor);
  manager_automaton_add (composer->manager, &composer->counter2, &counter_descriptor);
  manager_composition_add (composer->manager, &composer->trigger, trigger_output, &composer->counter1, counter_input);
  manager_composition_add (composer->manager, &composer->counter1, counter_output, &composer->self, composer_input1);
  manager_composition_add (composer->manager, &composer->trigger, trigger_output, &composer->counter2, counter_input);
  manager_composition_add (composer->manager, &composer->counter2, counter_output, &composer->self, composer_input2);

  return composer;
}

static bid_t composer_system_output (void* state);

static void
composer_system_input (void* state, bid_t bid)
{
  printf ("composer_system_input\n");
  assert (state != NULL);
  composer_t* composer = state;

  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));
  const receipt_t* receipt = buffer_read_ptr (bid);

  if (manager_apply (composer->manager, receipt)) {
    schedule_system_output ();
  }
}

static bid_t
composer_system_output (void* state)
{
  printf ("composer_system_output\n");
  assert (state != NULL);
  composer_t* composer = state;

  return manager_action (composer->manager);
}

void
composer_input1 (void* state, bid_t bid)
{
  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (counter_output_t));

  const counter_output_t* output = buffer_read_ptr (bid);
  printf ("count(1) = %d\n", output->count);
}

void
composer_input2 (void* state, bid_t bid)
{
  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (counter_output_t));

  const counter_output_t* output = buffer_read_ptr (bid);
  printf ("count(2) = %d\n", output->count);
}

static input_t composer_inputs[] = { composer_input1, composer_input2, NULL };
static output_t composer_outputs[] = { NULL };
static internal_t composer_internals[] = { NULL };

descriptor_t composer_descriptor = {
  .constructor = composer_create,
  .system_input = composer_system_input,
  .system_output = composer_system_output,
  .inputs = composer_inputs,
  .outputs = composer_outputs,
  .internals = composer_internals,
};
