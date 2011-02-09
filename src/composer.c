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

static state_ptr_t
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

static bid_t composer_system_output (state_ptr_t state);

static void
composer_system_input (state_ptr_t state, bid_t bid)
{
  printf ("composer_system_input\n");
  assert (state != NULL);
  composer_t* composer = state;

  assert (bid != -1);
  assert (ueioa_buffer_size (bid) == sizeof (system_receipt_t));
  const system_receipt_t* receipt = ueioa_buffer_read_ptr (bid);

  if (manager_apply (composer->manager, receipt)) {
    ueioa_schedule_system_output ();
  }
}

static bid_t
composer_system_output (state_ptr_t state)
{
  printf ("composer_system_output\n");
  assert (state != NULL);
  composer_t* composer = state;

  return manager_action (composer->manager);
}

void
composer_input1 (state_ptr_t state, bid_t bid)
{
  assert (bid != -1);
  assert (ueioa_buffer_size (bid) == sizeof (counter_output_t));

  const counter_output_t* output = ueioa_buffer_read_ptr (bid);
  printf ("count(1) = %d\n", output->count);
}

void
composer_input2 (state_ptr_t state, bid_t bid)
{
  assert (bid != -1);
  assert (ueioa_buffer_size (bid) == sizeof (counter_output_t));

  const counter_output_t* output = ueioa_buffer_read_ptr (bid);
  printf ("count(2) = %d\n", output->count);
}

static input_t composer_inputs[] = { composer_input1, composer_input2 };

automaton_descriptor_t composer_descriptor = {
  .constructor = composer_create,
  .system_input = composer_system_input,
  .system_output = composer_system_output,
  .input_count = 2,
  .inputs = composer_inputs,
  .output_count = 0,
  .outputs = NULL,
  .internal_count = 0,
  .internals = NULL
};
