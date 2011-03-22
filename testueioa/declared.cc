#include <stdlib.h>
#include <assert.h>
#include <ueioa.h>

typedef enum {
  UNSENT,
  SENT
} declared_state_t;

typedef struct {
  declared_state_t state;
} declared_t;

static void*
declared_create (const void* arg)
{
  declared_t* declared = (declared_t*)malloc (sizeof (declared_t));
  declared->state = UNSENT;

  return declared;
}

static void
declared_system_input (void* state, void* param, bid_t bid)
{
  declared_t* declared = (declared_t*)state;
  assert (declared != NULL);

  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));
  const receipt_t* receipt = (const receipt_t*)buffer_read_ptr (bid);

  switch (declared->state) {
  case UNSENT:
    if (receipt->type == SELF_CREATED) {
      assert (schedule_system_output () == 0);
    }
    else {
      assert (0);
    }
    break;
  case SENT:
    if (receipt->type == DECLARED) {
      exit (EXIT_SUCCESS);
    }
    else {
      assert (0);
    }
    break;
  }
}

static bid_t
declared_system_output (void* state, void* param)
{
  declared_t* declared = (declared_t*)state;
  assert (declared != NULL);

  bid_t bid = buffer_alloc (sizeof (order_t));
  order_t* order = (order_t*)buffer_write_ptr (bid);
  /* Declare a parameter. */
  order_declare_init (order, (void*)567);
  declared->state = SENT;

  return bid;
}

descriptor_t declared_descriptor = {
  declared_create,
  declared_system_input,
  declared_system_output,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
};

int
main (int argc, char** argv)
{
  ueioa_run (&declared_descriptor, NULL, 1);
  exit (EXIT_SUCCESS);
}
