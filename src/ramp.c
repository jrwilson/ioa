#include "ramp.h"

#include <assert.h>
#include <automan.h>
#include <stdlib.h>
#include "ramp_proxy.h"

static bid_t ramp_integer_out (void*, void*);

typedef struct {
  bool declared;
  aid_t aid;
  aid_t aid2;
  bool composed;
  proxy_request_t request;
} ramp_proxy_t;

typedef struct {
  int x;
  aid_t self;
  automan_t* automan;
} ramp_t;

#define LIMIT 10

static void
ramp_proxy_created (void* state, void* param, receipt_type_t receipt)
{
  ramp_t* ramp = state;
  assert (ramp != NULL);
  
  ramp_proxy_t* ramp_proxy = param;
  assert (ramp_proxy != NULL);

  if (receipt == CHILD_CREATED) {
    ramp_proxy->aid2 = ramp_proxy->aid;
    assert (automan_proxy_send_created (ramp_proxy->aid, -1, &ramp_proxy->request) == 0);
  }
  else if (receipt == CHILD_DESTROYED) {
    assert (automan_rescind (ramp->automan,
			     &ramp_proxy->declared) == 0);
  }
  else {
    assert (0);
  }
}

static void*
ramp_create (const void* arg)
{
  ramp_t* ramp = malloc (sizeof (ramp_t));

  ramp->x = LIMIT - 1;

  ramp->automan = automan_creat (ramp, &ramp->self);

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

  automan_apply (ramp->automan, receipt);

  if (receipt->type == SELF_CREATED) {
    /* Schedule output. */
    assert (schedule_alarm_input (1, 0) == 0);
  }
}

static bid_t
ramp_system_output (void* state, void* param)
{
  ramp_t* ramp = state;
  assert (ramp != NULL);

  return automan_action (ramp->automan);
}

static void
ramp_declared (void* state,
	       void* param,
	       receipt_type_t receipt)
{
  ramp_t* ramp = state;
  assert (ramp != NULL);
  
  ramp_proxy_t* ramp_proxy = param;
  assert (ramp_proxy != NULL);

  if (receipt == DECLARED) {
    /* Okay. */
  }
  else if (receipt == RESCINDED) {
    assert (automan_proxy_send_destroyed (ramp_proxy->aid2, &ramp_proxy->request) == 0);
    free (ramp_proxy);
  }
  else {
    assert (0);
  }
}

void
ramp_request_proxy (void* state, void* param, bid_t bid)
{
  ramp_t* ramp = state;
  assert (ramp != NULL);

  assert (buffer_size (bid) == sizeof (proxy_request_t));

  const proxy_request_t* request = buffer_read_ptr (bid);
  ramp_proxy_t* ramp_proxy = malloc (sizeof (ramp_proxy_t));
  ramp_proxy->request = *request;
  assert (automan_declare (ramp->automan,
			   &ramp_proxy->composed,
			   ramp_proxy,
			   ramp_declared,
			   ramp_proxy) == 0);
  assert (automan_create (ramp->automan,
			  &ramp_proxy->aid,
			  &ramp_proxy_descriptor,
			  NULL,
			  ramp_proxy_created,
			  ramp_proxy) == 0);
  assert (automan_compose (ramp->automan,
			   &ramp_proxy->composed,
			   &ramp->self,
			   ramp_integer_out,
			   ramp_proxy,
			   &ramp_proxy->aid,
			   ramp_proxy_integer_in,
			   NULL,
			   NULL,
			   NULL) == 0);
  assert (schedule_system_output () == 0);
}

static void
ramp_alarm_input (void* state,
		  void* param,
		  bid_t bid)
{
  assert (schedule_output (ramp_integer_out, NULL) == 0);  
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

  assert (schedule_alarm_input (1, 0) == 0);
  return bid;
}

static input_t ramp_free_inputs[] = {
  ramp_request_proxy,
  NULL
};

static output_t ramp_outputs[] = {
  ramp_integer_out,
  NULL
};

descriptor_t ramp_descriptor = {
  .constructor = ramp_create,
  .system_input = ramp_system_input,
  .system_output = ramp_system_output,
  .alarm_input = ramp_alarm_input,
  .free_inputs = ramp_free_inputs,
  .outputs = ramp_outputs,
};

static input_message_descriptor_t ramp_message_inputs[] = {
  {
    NULL, NULL, NULL
  }
};

static output_message_descriptor_t ramp_message_outputs[] = {
  {
    "display_integer", "integer", ramp_proxy_integer_out
  },
  {
    NULL, NULL, NULL
  }
};

const port_descriptor_t ramp_port_descriptors[] = {
  {
    .cardinality = 0,
    .input_message_descriptors = ramp_message_inputs,
    .output_message_descriptors = ramp_message_outputs,
  },
  {
    0, NULL, NULL
  },
};
