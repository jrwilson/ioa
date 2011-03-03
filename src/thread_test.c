#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include <ueioa.h>

typedef struct {
  int count;
} counter_t;

static void counter_internal (void* state, void* param);

static void*
counter_create (const void* arg)
{
  counter_t* counter = malloc (sizeof (counter_t));
  counter->count = 0;
  return counter;
}

static void
counter_system_input (void* state, void* param, bid_t bid)
{
  assert (state != NULL);
  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));

  const receipt_t* receipt = buffer_read_ptr (bid);
  if (receipt->type == SELF_CREATED) {
    assert (schedule_internal (counter_internal, NULL) == 0);
  }
}

static bid_t
counter_system_output (void* state, void* param)
{
  return -1;
}

static bid_t counter_output (void* state, void* param);

static void
counter_internal (void* state, void* param)
{
  assert (state != NULL);
  counter_t* counter = state;
  if (counter->count < 100000) {
    ++counter->count;
    assert (schedule_internal (counter_internal, NULL) == 0);
  }
  else {
    assert (schedule_output (counter_output, NULL) == 0);
  }
}

static bid_t
counter_output (void* state, void* param)
{
  assert (state != NULL);

  return buffer_alloc (0);
}

static input_t counter_inputs[] = { NULL };
static output_t counter_outputs[] = { counter_output, NULL };
static internal_t counter_internals[] = { counter_internal, NULL };

descriptor_t counter_descriptor = {
  .constructor = counter_create,
  .system_input = counter_system_input,
  .system_output = counter_system_output,
  .inputs = counter_inputs,
  .outputs = counter_outputs,
  .internals = counter_internals,
};

typedef struct {
  bool in1;
  bool in2;
} receiver_t;

static void*
receiver_create (const void* arg)
{
  receiver_t* receiver = malloc (sizeof (receiver_t));
  receiver->in1 = false;
  receiver->in2 = false;
  return receiver;
}

static void
receiver_system_input (void* state, void* param, bid_t bid)
{
  assert (state != NULL);
  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));
}

static bid_t
receiver_system_output (void* state, void* param)
{
  return -1;
}

static void
receiver_input1 (void* state, void* param, bid_t bid)
{
  receiver_t* receiver = state;
  assert (receiver != NULL);

  receiver->in1 = true;
  if (receiver->in1 && receiver->in2) {
    exit (EXIT_SUCCESS);
  }
}

static void
receiver_input2 (void* state, void* param, bid_t bid)
{
  receiver_t* receiver = state;
  assert (receiver != NULL);

  receiver->in2 = true;
  if (receiver->in1 && receiver->in2) {
    exit (EXIT_SUCCESS);
  }
}

static input_t receiver_inputs[] = { receiver_input1, receiver_input2, NULL };
static output_t receiver_outputs[] = { NULL };
static internal_t receiver_internals[] = { NULL };

descriptor_t receiver_descriptor = {
  .constructor = receiver_create,
  .system_input = receiver_system_input,
  .system_output = receiver_system_output,
  .inputs = receiver_inputs,
  .outputs = receiver_outputs,
  .internals = receiver_internals,
};

typedef struct {
  manager_t* manager;

  aid_t counter1;
  aid_t counter2;
  aid_t receiver;
} composer_t;

static void*
composer_create (const void* arg)
{
  composer_t* composer = malloc (sizeof (composer_t));

  composer->manager = manager_create ();
  manager_child_add (composer->manager, &composer->counter1, &counter_descriptor, NULL, NULL, NULL);
  manager_child_add (composer->manager, &composer->counter2, &counter_descriptor, NULL, NULL, NULL);
  manager_child_add (composer->manager, &composer->receiver, &receiver_descriptor, NULL, NULL, NULL);
  manager_composition_add (composer->manager, &composer->counter1, counter_output, NULL, &composer->receiver, receiver_input1, NULL);
  manager_composition_add (composer->manager, &composer->counter2, counter_output, NULL, &composer->receiver, receiver_input2, NULL);

  return composer;
}

static bid_t composer_system_output (void* state, void* param);

static void
composer_system_input (void* state, void* param, bid_t bid)
{
  assert (state != NULL);
  composer_t* composer = state;

  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));
  const receipt_t* receipt = buffer_read_ptr (bid);

  manager_apply (composer->manager, receipt);
}

static bid_t
composer_system_output (void* state, void* param)
{
  assert (state != NULL);
  composer_t* composer = state;

  return manager_action (composer->manager);
}

static input_t composer_inputs[] = { NULL };
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


int
main (int argc, char** argv)
{
  if (argc != 2) {
    fprintf (stderr, "Usage: %s COUNT\n", argv[0]);
    exit (EXIT_FAILURE);
  }

  int count = atoi (argv[1]);

  ueioa_run (&composer_descriptor, NULL, count);

  exit (EXIT_SUCCESS);
}
