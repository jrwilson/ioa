#include "integer_reflector_proxy.h"

#include <assert.h>
#include <stdlib.h>

typedef struct {
  int x;
} integer_reflector_proxy_t;

static void*
integer_reflector_proxy_create (const void* arg)
{
  return malloc (sizeof (integer_reflector_proxy_t));
}

void
integer_reflector_proxy_integer_in (void* state,
				    void* param,
				    bid_t bid)
{
  integer_reflector_proxy_t* integer_reflector_proxy = state;
  assert (integer_reflector_proxy != NULL);

  assert (buffer_size (bid) == sizeof (int));
  const int* x = buffer_read_ptr (bid);
  integer_reflector_proxy->x = *x;
  assert (schedule_output (integer_reflector_proxy_integer_out, NULL) == 0);
}

bid_t
integer_reflector_proxy_integer_out (void* state,
				     void* param)
{
  integer_reflector_proxy_t* integer_reflector_proxy = state;
  assert (integer_reflector_proxy != NULL);

  bid_t bid = buffer_alloc (sizeof (int));
  int* x = buffer_write_ptr (bid);
  *x = integer_reflector_proxy->x;

  return bid;
}

static const input_t integer_reflector_proxy_inputs[] = {
  integer_reflector_proxy_integer_in,
  NULL,
};

static const output_t integer_reflector_proxy_outputs[] = {
  integer_reflector_proxy_integer_out,
  NULL,
};


const descriptor_t integer_reflector_proxy_descriptor = {
  .constructor = integer_reflector_proxy_create,
  .inputs = integer_reflector_proxy_inputs,
  .outputs = integer_reflector_proxy_outputs,
};
