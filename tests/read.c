#include <assert.h>
#include <stdlib.h>
#include <unistd.h>

#include <ueioa.h>

typedef enum {
  START,
  WRITE_UNSENT,
  WRITE_SENT,
  READ_UNSENT,
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
      rd->state = WRITE_UNSENT;
      schedule_system_output ();
    }
    else {
      assert (0);
    }
    break;
  case WRITE_UNSENT:
    assert (0);
    break;
  case WRITE_SENT:
    if (receipt->type == WRITE_WAKEUP) {
      char c = 'A';
      write (rd->pipes[1], &c, 1);
      rd->state = READ_UNSENT;
      schedule_system_output ();
    }
    else {
      assert (0);
    }
    break;
  case READ_UNSENT:
    assert (0);
    break;
  case READ_SENT:
    if (receipt->type == READ_WAKEUP) {
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
  read_t* read = state;
  assert (read != NULL);

  bid_t bid = buffer_alloc (sizeof (order_t));
  order_t* order = buffer_write_ptr (bid);

  switch (read->state) {
  case START:
    assert (0);
    break;
  case WRITE_UNSENT:
    order_set_write_alarm_init (order, read->pipes[1]);
    read->state = WRITE_SENT;
    break;
  case WRITE_SENT:
    assert (0);
    break;
  case READ_UNSENT:
    order_set_read_alarm_init (order, read->pipes[0]);
    read->state = READ_SENT;
    break;
  case READ_SENT:
    assert (0);
    break;
  }

  return bid;
}

static input_t read_inputs[] = { NULL };
static output_t read_outputs[] = { NULL };
static internal_t read_internals[] = { NULL };

descriptor_t read_descriptor = {
  .constructor = read_create,
  .system_input = read_system_input,
  .system_output = read_system_output,
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
