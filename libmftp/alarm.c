#include <mftp.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>

typedef struct {
  bool set;
  bool expired;
  manager_t* manager;
  bool composed;
} alarm_t;

static void*
alarm_create (void* arg)
{
  alarm_t* alarm = malloc (sizeof (alarm_t));
  alarm->set = false;
  alarm->expired = false;

  alarm->manager = manager_create ();

  manager_output_add (alarm->manager, &alarm->composed, alarm_alarm_out, NULL);

  return alarm;
}

static void
alarm_system_input (void* state, void* param, bid_t bid)
{
  assert (state != NULL);
  alarm_t* alarm = state;

  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));
  const receipt_t* receipt = buffer_read_ptr (bid);

  manager_apply (alarm->manager, receipt);
}

static bid_t
alarm_system_output (void* state, void* param)
{
  alarm_t* alarm = state;
  assert (alarm != NULL);

  return manager_action (alarm->manager);
}

void
alarm_set_in (void* state, void* param, bid_t bid)
{
  alarm_t* alarm = state;
  assert (alarm != NULL);
  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (alarm_set_in_t));
  const alarm_set_in_t* in = buffer_read_ptr (bid);

  if (!alarm->set) {
    assert (schedule_alarm_input (in->secs, in->usecs) == 0);
    alarm->set = true;
  }
}

static void
alarm_alarm_input (void* state, void* param, bid_t bid)
{
  alarm_t* alarm = state;
  assert (alarm != NULL);

  assert (alarm->set);
  alarm->expired = true;
  assert (schedule_output (alarm_alarm_out, NULL) == 0);
}

bid_t
alarm_alarm_out (void* state, void* param)
{
  alarm_t* alarm = state;
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
  .constructor = alarm_create,
  .system_input = alarm_system_input,
  .system_output = alarm_system_output,
  .alarm_input = alarm_alarm_input,
  .inputs = alarm_inputs,
  .outputs = alarm_outputs,
};
