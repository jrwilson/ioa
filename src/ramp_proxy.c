#include "ramp_proxy.h"

#include <stdlib.h>
#include <assert.h>

#include "port_manager.h"

typedef struct {
  aid_t aid;
  input_t free_input;
  bidq_t* bidq;
} ramp_proxy_t;

static void*
ramp_proxy_create (void* a)
{
  ramp_proxy_create_arg_t* arg = a;
  assert (arg != NULL);

  ramp_proxy_t* ramp_proxy = malloc (sizeof (ramp_proxy_t));

  ramp_proxy->aid = arg->aid;
  ramp_proxy->free_input = arg->free_input;
  ramp_proxy->bidq = bidq_create ();

  return ramp_proxy;
}

static void
ramp_proxy_system_input (void* state, void* param, bid_t bid)
{
  ramp_proxy_t* ramp_proxy = state;
  assert (ramp_proxy != NULL);
  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));
  const receipt_t* receipt = buffer_read_ptr (bid);

  if (receipt->type == SELF_CREATED) {
    bid_t bid = proxy_receipt_create (receipt->self_created.self, -1);
    assert (schedule_free_input (ramp_proxy->aid, ramp_proxy->free_input, bid) == 0);
  }
}

void
ramp_proxy_integer_in (void* state, void* param, bid_t bid)
{
  ramp_proxy_t* ramp_proxy = state;
  assert (ramp_proxy != NULL);

  /* Enqueue the item. */
  buffer_incref (bid);
  bidq_push_back (ramp_proxy->bidq, bid);
  
  assert (schedule_output (ramp_proxy_integer_out, NULL) == 0);
}

bid_t
ramp_proxy_integer_out (void* state, void* param)
{
  ramp_proxy_t* ramp_proxy = state;
  assert (ramp_proxy != NULL);

  if (!bidq_empty (ramp_proxy->bidq)) {
    bid_t bid = bidq_front (ramp_proxy->bidq);
    bidq_pop_front (ramp_proxy->bidq);

    /* Go again. */
    assert (schedule_output (ramp_proxy_integer_out, NULL) == 0);

    return bid;
  }
  else {
    return -1;
  }
}

static input_t ramp_proxy_inputs[] = { ramp_proxy_integer_in, NULL };
static output_t ramp_proxy_outputs[] = { ramp_proxy_integer_out, NULL };

descriptor_t ramp_proxy_descriptor = {
  .constructor = ramp_proxy_create,
  .system_input = ramp_proxy_system_input,
  .inputs = ramp_proxy_inputs,
  .outputs = ramp_proxy_outputs,
};
