#include <stdlib.h>
#include <assert.h>
#include <ueioa.hh>

static void
schedule_internal_internal (void* state, void* param)
{
  exit (EXIT_SUCCESS);
}

static void
schedule_internal_system_input (void* state, void* param, bid_t bid)
{
  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));
  const receipt_t* receipt = (const receipt_t*)buffer_read_ptr (bid);

  if (receipt->type == SELF_CREATED) {
    assert (schedule_internal (schedule_internal_internal, NULL) == 0);
  }
  else {
    assert (0);
  }

}

static internal_t schedule_internal_internals[] = { schedule_internal_internal, NULL };

descriptor_t schedule_internal_descriptor = {
  NULL,
  schedule_internal_system_input,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  schedule_internal_internals,
};

int
main (int argc, char** argv)
{
  ueioa_run (&schedule_internal_descriptor, NULL, 1);
  exit (EXIT_SUCCESS);
}
