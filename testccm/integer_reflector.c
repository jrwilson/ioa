#include "integer_reflector.h"

#include <assert.h>
#include <automan.h>
#include <stdlib.h>
#include "integer_reflector_proxy.h"

typedef struct {
  bool declared;
  aid_t aid;
  aid_t aid2;
  proxy_request_t request;
} integer_reflector_proxy_t;

static void integer_reflector_proxy_created (void*, void*, receipt_type_t);

typedef struct {
  aid_t self;
  automan_t* automan;
} integer_reflector_t;

static void*
integer_reflector_create (const void* arg)
{
  integer_reflector_t* integer_reflector = malloc (sizeof (integer_reflector_t));

  integer_reflector->automan = automan_creat (integer_reflector, &integer_reflector->self);

  return integer_reflector;
}

static void
integer_reflector_system_input (void* state, void* param, bid_t bid)
{
  integer_reflector_t* integer_reflector = state;
  assert (integer_reflector != NULL);

  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));
  const receipt_t* receipt = buffer_read_ptr (bid);

  automan_apply (integer_reflector->automan, receipt);
}

static bid_t
integer_reflector_system_output (void* state, void* param)
{
  integer_reflector_t* integer_reflector = state;
  assert (integer_reflector != NULL);

  return automan_action (integer_reflector->automan);
}

static void
integer_reflector_declared (void* state,
			    void* param,
			    receipt_type_t receipt)
{
  integer_reflector_t* integer_reflector = state;
  assert (integer_reflector != NULL);
  
  integer_reflector_proxy_t* integer_reflector_proxy = param;
  assert (integer_reflector_proxy != NULL);

  if (receipt == DECLARED) {
    /* Okay. */
  }
  else if (receipt == RESCINDED) {
    assert (automan_proxy_send_destroyed (integer_reflector_proxy->aid2, &integer_reflector_proxy->request) == 0);
    free (integer_reflector_proxy);
  }
  else {
    assert (0);
  }
}

void
integer_reflector_request_proxy (void* state, void* param, bid_t bid)
{
  integer_reflector_t* integer_reflector = state;
  assert (integer_reflector != NULL);

  assert (buffer_size (bid) == sizeof (proxy_request_t));

  const proxy_request_t* request = buffer_read_ptr (bid);
  integer_reflector_proxy_t* integer_reflector_proxy = malloc (sizeof (integer_reflector_proxy_t));
  integer_reflector_proxy->request = *request;
  assert (automan_declare (integer_reflector->automan,
			   &integer_reflector_proxy->declared,
			   integer_reflector_proxy,
			   integer_reflector_declared,
			   integer_reflector_proxy) == 0);
  assert (automan_create (integer_reflector->automan,
			  &integer_reflector_proxy->aid,
			  &integer_reflector_proxy_descriptor,
			  NULL,
			  integer_reflector_proxy_created,
			  integer_reflector_proxy) == 0);
}

static void
integer_reflector_proxy_created (void* state, void* param, receipt_type_t receipt)
{
  integer_reflector_t* integer_reflector = state;
  assert (integer_reflector != NULL);
  
  integer_reflector_proxy_t* integer_reflector_proxy = param;
  assert (integer_reflector_proxy != NULL);

  if (receipt == CHILD_CREATED) {
    integer_reflector_proxy->aid2 = integer_reflector_proxy->aid;
    assert (automan_proxy_send_created (integer_reflector_proxy->aid, -1, &integer_reflector_proxy->request) == 0);
  }
  else if (receipt == CHILD_DESTROYED) {
    assert (automan_rescind (integer_reflector->automan,
			     &integer_reflector_proxy->declared) == 0);
  }
  else {
    assert (0);
  }
}

static input_t integer_reflector_free_inputs[] = {
  integer_reflector_request_proxy,
  NULL
};

const descriptor_t
integer_reflector_descriptor = {
  .constructor = integer_reflector_create,
  .system_input = integer_reflector_system_input,
  .system_output = integer_reflector_system_output,
  .free_inputs = integer_reflector_free_inputs,
};

static input_message_descriptor_t integer_reflector_inputs[] = {
  {
    "integer_in", "integer", integer_reflector_proxy_integer_in
  },
  {
    NULL, NULL, NULL
  }
};

static output_message_descriptor_t integer_reflector_outputs[] = {
  {
    "integer_out", "integer", integer_reflector_proxy_integer_out
  },
  {
    NULL, NULL, NULL
  }
};

const port_descriptor_t
integer_reflector_port_descriptors[] = {
  {
    .cardinality = 1,
    .input_message_descriptors = integer_reflector_inputs,
    .output_message_descriptors = integer_reflector_outputs,
  },
  {
    0, NULL, NULL
  },
};
