#include <assert.h>
#include <stdlib.h>
#include <unistd.h>

#include <ueioa.h>

typedef enum {
  START,
  WRITE_SENT,
  READ_SENT
} read_state_t;

typedef struct {
  read_state_t state;
  int pipes[2];
} read_t;

static void*
read_create (void)
{
  read_t* read = malloc (sizeof (read_t));
  read->state = START;
  pipe (read->pipes);
  return read;
}

static bid_t read_system_output (void* state, void* param);

static void
read_system_input (void* state, void* param, bid_t bid)
{
  read_t* rd = state;
  assert (rd != NULL);
  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));
  const receipt_t* receipt = buffer_read_ptr (bid);

  switch (rd->state) {
  case START:
    if (receipt->type == SELF_CREATED) {
      schedule_write_ready (rd->pipes[1]);
      rd->state = WRITE_SENT;
    }
    else {
      assert (0);
    }
    break;
  case WRITE_SENT:
    if (receipt->type == WRITE_READY) {
      char c = 'A';
      write (rd->pipes[1], &c, 1);
      schedule_read_ready (rd->pipes[0]);
      rd->state = READ_SENT;
    }
    else {
      assert (0);
    }
    break;
  case READ_SENT:
    if (receipt->type == READ_READY) {
      char c;
      read (rd->pipes[0], &c, 1);
      assert (c == 'A');
      exit (EXIT_SUCCESS);
    }
    else {
      assert (0);
    }
    break;
  }

}

static bid_t
read_system_output (void* state, void* param)
{
  return -1;
}

static input_t read_free_inputs[] = { NULL };
static input_t read_inputs[] = { NULL };
static output_t read_outputs[] = { NULL };
static internal_t read_internals[] = { NULL };

descriptor_t read_descriptor = {
  .constructor = read_create,
  .system_input = read_system_input,
  .system_output = read_system_output,
  .free_inputs = read_free_inputs,
  .inputs = read_inputs,
  .outputs = read_outputs,
  .internals = read_internals,
};


int
main (int argc, char** argv)
{
  ueioa_run (&read_descriptor, 1);

  exit (EXIT_SUCCESS);
}
