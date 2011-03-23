#include <stdlib.h>
#include <assert.h>
#include <ueioa.hh>

static void
self_created_system_input (void* state, void* param, bid_t bid)
{
  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));
  const receipt_t* receipt = (const receipt_t*)buffer_read_ptr (bid);

  if (receipt->type == SELF_CREATED) {
    exit (EXIT_SUCCESS);
  }
  else {
    assert (0);
  }
}

descriptor_t self_created_descriptor = {
  NULL,
  self_created_system_input,
  NULL,
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
  ueioa_run (&self_created_descriptor, NULL, 1);
  exit (EXIT_SUCCESS);
}
