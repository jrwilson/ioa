#include "ramp_proxy.h"

#include <assert.h>
#include <stdlib.h>
#include <automan.h>

typedef struct {
  bidq_t* bidq;
  aid_t self;
  automan_t* automan;
  bool out_composed;
} ramp_proxy_t;

static void
composed (void* state,
	  void* param,
	  receipt_type_t receipt)
{
  ramp_proxy_t* ramp_proxy = state;
  assert (ramp_proxy != NULL);

  if (receipt == INPUT_DECOMPOSED ||
      receipt == OUTPUT_DECOMPOSED) {
    if (!ramp_proxy->out_composed) {
      automan_self_destruct (ramp_proxy->automan);
    }
  }
}

static void*
ramp_proxy_create (const void* a)
{
  ramp_proxy_t* ramp_proxy = malloc (sizeof (ramp_proxy_t));

  ramp_proxy->bidq = bidq_create ();
  ramp_proxy->automan = automan_creat (ramp_proxy,
				       &ramp_proxy->self);
  assert (automan_output_add (ramp_proxy->automan,
			      &ramp_proxy->out_composed,
			      ramp_proxy_integer_out,
			      NULL,
			      composed,
			      NULL) == 0);

  return ramp_proxy;
}

static void
ramp_proxy_system_input (void* state,
		       void* param,
		       bid_t bid)
{
  ramp_proxy_t* ramp_proxy = state;
  assert (ramp_proxy != NULL);

  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));
  const receipt_t* receipt = buffer_read_ptr (bid);

  automan_apply (ramp_proxy->automan, receipt);
}

static bid_t
ramp_proxy_system_output (void* state,
			void* param)
{
  ramp_proxy_t* ramp_proxy = state;
  assert (ramp_proxy != NULL);

  return automan_action (ramp_proxy->automan);
}

void
ramp_proxy_integer_in (void* state, void* param, bid_t bid)
{
  ramp_proxy_t* ramp_proxy = state;
  assert (ramp_proxy != NULL);

  if (ramp_proxy->out_composed) {
    /* Enqueue the item. */
    buffer_incref (bid);
    bidq_push_back (ramp_proxy->bidq, bid);
    
    assert (schedule_output (ramp_proxy_integer_out, NULL) == 0);
  }
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
  .system_output = ramp_proxy_system_output,
  .inputs = ramp_proxy_inputs,
  .outputs = ramp_proxy_outputs,
};
