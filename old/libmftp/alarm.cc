#include <mftp.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>

typedef struct {
  bool set;
  bool expired;
} alarm_t;

static void*
alarm_create (const void* arg)
{
  alarm_t* alarm = (alarm_t*)malloc (sizeof (alarm_t));
  alarm->set = false;
  alarm->expired = false;

  return alarm;
}

void
alarm_set_in (void* state, void* param, bid_t bid)
{
  alarm_t* alarm = (alarm_t*)state;
  assert (alarm != NULL);
  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (alarm_set_in_t));
  const alarm_set_in_t* in = (const alarm_set_in_t*)buffer_read_ptr (bid);

  if (!alarm->set) {
    assert (schedule_alarm_input (in->secs, in->usecs) == 0);
    alarm->set = true;
  }
}

static void
alarm_alarm_input (void* state, void* param, bid_t bid)
{
  alarm_t* alarm = (alarm_t*)state;
  assert (alarm != NULL);

  assert (alarm->set);
  alarm->expired = true;
  assert (schedule_output (alarm_alarm_out, NULL) == 0);
}

bid_t
alarm_alarm_out (void* state, void* param)
{
  alarm_t* alarm = (alarm_t*)state;
  assert (alarm != NULL);

  if (alarm->expired) {
    alarm->set = false;
    alarm->expired = false;
    return buffer_alloc (0);
  }
  else {
    return -1;
  }
}

static input_t alarm_inputs[] = {
  alarm_set_in,
  NULL
};
static output_t alarm_outputs[] = {
  alarm_alarm_out,
  NULL
};

descriptor_t alarm_descriptor = {
  alarm_create,
  NULL,
  NULL,
  alarm_alarm_input,
  NULL,
  NULL,
  NULL,
  alarm_inputs,
  alarm_outputs,
  NULL
};
