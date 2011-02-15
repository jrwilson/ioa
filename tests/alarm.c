#include <assert.h>
#include <stdlib.h>

#include <ueioa.h>

static void*
alarm_create (void)
{
  return NULL;
}

static bid_t alarm_system_output (void* state, void* param);

static void
alarm_system_input (void* state, void* param, bid_t bid)
{
  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));
  const receipt_t* receipt = buffer_read_ptr (bid);

  if (receipt->type == SELF_CREATED) {
    schedule_alarm (1, 0);
  }
  else if (receipt->type == ALARM) {
    exit (EXIT_SUCCESS);
  }
  else {
    assert (0);
  }
}

static bid_t
alarm_system_output (void* state, void* param)
{
  return -1;
}

static input_t alarm_inputs[] = { NULL };
static output_t alarm_outputs[] = { NULL };
static internal_t alarm_internals[] = { NULL };

descriptor_t alarm_descriptor = {
  .constructor = alarm_create,
  .system_input = alarm_system_input,
  .system_output = alarm_system_output,
  .inputs = alarm_inputs,
  .outputs = alarm_outputs,
  .internals = alarm_internals,
};


int
main (int argc, char** argv)
{
  ueioa_run (&alarm_descriptor, 1);

  exit (EXIT_SUCCESS);
}
