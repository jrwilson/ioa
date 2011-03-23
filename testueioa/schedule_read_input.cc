#include <assert.h>
#include <stdlib.h>
#include <unistd.h>

#include <ueioa.hh>

typedef struct {
  int pipes[2];
} read_t;

static void*
read_create (const void* arg)
{
  read_t* read = (read_t*)malloc (sizeof (read_t));
  assert (pipe (read->pipes) == 0);
  return read;
}

static void
read_system_input (void* state, void* param, bid_t bid)
{
  read_t* rd = (read_t*)state;
  assert (rd != NULL);
  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));
  const receipt_t* receipt = (const receipt_t*)buffer_read_ptr (bid);

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
  read_t* rd = (read_t*)state;
  assert (rd != NULL);

  char c = 'A';
  assert (write (rd->pipes[1], &c, 1) == 1);
  assert (schedule_read_input (rd->pipes[0]) == 0);
}

static void
read_read_input (void* state, void* param, bid_t bid)
{
  read_t* rd = (read_t*)state;
  assert (rd != NULL);

  char c;
  assert (read (rd->pipes[0], &c, 1) == 1);
  assert (c == 'A');
  exit (EXIT_SUCCESS);
}

descriptor_t read_descriptor = {
  read_create,
  read_system_input,
  NULL,
  NULL,
  read_read_input,
  read_write_input,
  NULL,
  NULL,
  NULL,
  NULL,
};


int
main (int argc, char** argv)
{
  ueioa_run (&read_descriptor, NULL, 1);

  exit (EXIT_SUCCESS);
}
