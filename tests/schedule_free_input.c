#include <stdlib.h>
#include <assert.h>
#include <ueioa.h>

static void
free_input (void* state, void* param, bid_t bid)
{
  assert (state == NULL);
  assert (param == NULL);
  assert (bid != -1);
  assert (buffer_size (bid) == 0);

  exit (EXIT_SUCCESS);
}

static void
send_message_system_input (void* state, void* param, bid_t bid)
{
  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));
  const receipt_t* receipt = buffer_read_ptr (bid);

  if (receipt->type == SELF_CREATED) {
    assert (schedule_free_input (receipt->self_created.self, free_input, buffer_alloc (0)) == 0);
  }
  else {
    assert (0);
  }
}

static input_t send_message_free_inputs[] = { free_input, NULL };

descriptor_t send_message_descriptor = {
  .system_input = send_message_system_input,
  .free_inputs = send_message_free_inputs,
};

int
main (int argc, char** argv)
{
  ueioa_run (&send_message_descriptor, 1);
  exit (EXIT_SUCCESS);
}
