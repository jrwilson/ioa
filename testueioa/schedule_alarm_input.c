#include <assert.h>
#include <stdlib.h>

#include <ueioa.h>

static void
alarm_system_input (void* state, void* param, bid_t bid)
{
  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));
  const receipt_t* receipt = buffer_read_ptr (bid);

  if (receipt->type == SELF_CREATED) {
    assert (schedule_alarm_input (1, 0) == 0);
  }
  else {
    assert (0);
  }
}

static void
alarm_alarm_input (void* state, void* param, bid_t bid)
{
  exit (EXIT_SUCCESS);
}

descriptor_t alarm_descriptor = {
  .system_input = alarm_system_input,
  .alarm_input = alarm_alarm_input,
};


int
main (int argc, char** argv)
{
  ueioa_run (&alarm_descriptor, NULL, 1);

  exit (EXIT_SUCCESS);
}
