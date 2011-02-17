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
    assert (schedule_write_input (write->pipes[1]) == 0);
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

static void
write_write_input (void* state, void* param, bid_t bid)
{
  exit (EXIT_SUCCESS);
}

descriptor_t write_descriptor = {
  .constructor = write_create,
  .system_input = write_system_input,
  .system_output = write_system_output,
  .write_input = write_write_input,
};


int
main (int argc, char** argv)
{
  ueioa_run (&write_descriptor, 1);

  exit (EXIT_SUCCESS);
}
