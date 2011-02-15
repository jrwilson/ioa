#include <assert.h>
#include <stdlib.h>
#include <unistd.h>

#include <ueioa.h>

typedef struct {
  int pipes[2];
} write_t;

static void*
write_create (void)
{
  write_t* write = malloc (sizeof (write_t));
  pipe (write->pipes);
  return write;
}

static bid_t write_system_output (void* state, void* param);

static void
write_system_input (void* state, void* param, bid_t bid)
{
  write_t* write = state;
  assert (write != NULL);
  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));
  const receipt_t* receipt = buffer_read_ptr (bid);

  if (receipt->type == SELF_CREATED) {
    schedule_write_ready (write->pipes[1]);
  }
  else if (receipt->type == WRITE_READY) {
    exit (EXIT_SUCCESS);
  }
  else {
    assert (0);
  }
}

static bid_t
write_system_output (void* state, void* param)
{
  return -1;
}

static input_t write_inputs[] = { NULL };
static output_t write_outputs[] = { NULL };
static internal_t write_internals[] = { NULL };

descriptor_t write_descriptor = {
  .constructor = write_create,
  .system_input = write_system_input,
  .system_output = write_system_output,
  .inputs = write_inputs,
  .outputs = write_outputs,
  .internals = write_internals,
};


int
main (int argc, char** argv)
{
  ueioa_run (&write_descriptor, 1);

  exit (EXIT_SUCCESS);
}
