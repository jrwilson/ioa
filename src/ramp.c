#include "ramp.h"

#include <stdlib.h>
#include <assert.h>

#include "component_manager.h"
#include "ramp_proxy.h"

static bid_t ramp_integer_out (void*, void*);

typedef struct {
  aid_t aid;
  aid_t callback_aid;
  input_t callback_free_input;
} ramp_proxy_t;

static void ramp_proxy_created (void*, void*);

typedef struct {
  int x;
  manager_t* manager;
  aid_t self;
} ramp_t;

#define LIMIT 10

static void*
ramp_create (void* arg)
{
  ramp_t* ramp = malloc (sizeof (ramp_t));

  ramp->x = LIMIT - 1;

  ramp->manager = manager_create ();
  manager_self_set (ramp->manager, &ramp->self);

  return ramp;
}

static void
ramp_system_input (void* state, void* param, bid_t bid)
{
  ramp_t* ramp = state;
  assert (ramp != NULL);

  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));
  const receipt_t* receipt = buffer_read_ptr (bid);

  manager_apply (ramp->manager, receipt);

  if (receipt->type == SELF_CREATED) {
    /* Schedule output. */
    assert (schedule_output (ramp_integer_out, NULL) == 0);
  }
}

static bid_t
ramp_system_output (void* state, void* param)
{
  ramp_t* ramp = state;
  assert (ramp != NULL);

  return manager_action (ramp->manager);
}

bid_t
ramp_integer_out (void* state, void* param)
{
  ramp_t* ramp = state;
  assert (ramp != NULL);

  ++ramp->x;
  if (ramp->x >= LIMIT) {
    ramp->x = 0;
  }

  bid_t bid = buffer_alloc (sizeof (int));
  int* ptr = buffer_write_ptr (bid);
  *ptr = htonl (ramp->x);

  assert (schedule_output (ramp_integer_out, NULL) == 0);
  return bid;
}

void
ramp_request_proxy (void* state, void* param, bid_t bid)
{
  ramp_t* ramp = state;
  assert (ramp != NULL);

  assert (buffer_size (bid) == sizeof (proxy_request_t));

  const proxy_request_t* request = buffer_read_ptr (bid);
  ramp_proxy_t* ramp_proxy = malloc (sizeof (ramp_proxy_t));
  ramp_proxy->callback_aid = request->callback_aid;
  ramp_proxy->callback_free_input = request->callback_free_input;
  manager_param_add (ramp->manager,
		     ramp_proxy);
  manager_child_add (ramp->manager,
		     &ramp_proxy->aid,
		     &ramp_proxy_descriptor,
		     NULL,
		     ramp_proxy_created,
		     ramp_proxy);
  manager_composition_add (ramp->manager,
			   &ramp->self, ramp_integer_out, ramp_proxy,
			   &ramp_proxy->aid, ramp_proxy_integer_in, NULL);
  assert (schedule_system_output () == 0);
}

static void
ramp_proxy_created (void* state, void* param)
{
  ramp_t* ramp = state;
  assert (ramp != NULL);
  
  ramp_proxy_t* ramp_proxy = param;
  assert (ramp_proxy != NULL);
  
  assert (schedule_free_input (ramp_proxy->callback_aid, ramp_proxy->callback_free_input, proxy_receipt_create (ramp_proxy->aid, -1)) == 0);
}

static input_t ramp_free_inputs[] = {
  ramp_request_proxy,
  NULL
};

static output_t ramp_outputs[] = {
  ramp_integer_out,
  NULL
};

static internal_t ramp_internals[] = {
  ramp_proxy_created,
  NULL
};

descriptor_t ramp_descriptor = {
  .constructor = ramp_create,
  .system_input = ramp_system_input,
  .system_output = ramp_system_output,
  .free_inputs = ramp_free_inputs,
  .outputs = ramp_outputs,
  .internals = ramp_internals,
};
