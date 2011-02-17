#include <assert.h>
#include <stdlib.h>
#include <unistd.h>

#include <ueioa.h>

typedef struct {
  int pipes[2];
} read_t;

static void*
read_create (void)
{
  read_t* read = malloc (sizeof (read_t));
  pipe (read->pipes);
  return read;
}

static void
read_system_input (void* state, void* param, bid_t bid)
{
  read_t* rd = state;
  assert (rd != NULL);
  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));
  const receipt_t* receipt = buffer_read_ptr (bid);

  if (receipt->type == SELF_CREATED) {
    assert (schedule_write_input (rd->pipes[1]) == 0);
  }
  else {
    assert (0);
  }

}

static void
read_write_input (void* state, void* param, bid_t bid)
{
  read_t* rd = state;
  assert (rd != NULL);

  char c = 'A';
  write (rd->pipes[1], &c, 1);
  assert (schedule_read_input (rd->pipes[0]) == 0);
}

static void
read_read_input (void* state, void* param, bid_t bid)
{
  read_t* rd = state;
  assert (rd != NULL);

  char c;
  read (rd->pipes[0], &c, 1);
  assert (c == 'A');
  exit (EXIT_SUCCESS);
}

descriptor_t read_descriptor = {
  .constructor = read_create,
  .system_input = read_system_input,
  .read_input = read_read_input,
  .write_input = read_write_input,
};


int
main (int argc, char** argv)
{
  ueioa_run (&read_descriptor, 1);

  exit (EXIT_SUCCESS);
}
