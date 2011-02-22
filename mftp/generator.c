#include "generator.h"

#include <stdlib.h>
#include <assert.h>

static void
generator_system_input (void* state, void* param, bid_t bid)
{
  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));

  const receipt_t* receipt = buffer_read_ptr (bid);
  if (receipt->type == SELF_CREATED) {
    assert (schedule_alarm_input (1, 0) == 0);
  }
}

bid_t
generator_output (void* state, void* param)
{
  return buffer_alloc (0);
}

static void
generator_alarm_input (void* state, void* param, bid_t bid)
{
  assert (schedule_output (generator_output, NULL) == 0);
  assert (schedule_alarm_input (1, 0) == 0);
}

static output_t generator_outputs[] = { generator_output, NULL };

descriptor_t generator_descriptor = {
  .system_input = generator_system_input,
  .alarm_input = generator_alarm_input,
  .outputs = generator_outputs,
};
