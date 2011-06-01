#include "integer_reflector_proxy.h"

#include <assert.h>
#include <stdlib.h>
#include <automan.h>

typedef struct {
  int x;
  bool have_x;
  bool in_composed;
  bool out_composed;
  aid_t self;
  automan_t* automan;
} integer_reflector_proxy_t;

static void
composed (void* state,
	  void* param,
	  receipt_type_t receipt)
{
  integer_reflector_proxy_t* integer_reflector_proxy = state;
  assert (integer_reflector_proxy != NULL);

  /* We might need to output. */
  assert (schedule_output (integer_reflector_proxy_integer_out, NULL) == 0);
 
  if (receipt == INPUT_DECOMPOSED ||
      receipt == OUTPUT_DECOMPOSED) {
    if (!integer_reflector_proxy->in_composed &&
	!integer_reflector_proxy->out_composed) {
      automan_self_destruct (integer_reflector_proxy->automan);
    }
  }
}

static void*
integer_reflector_proxy_create (const void* arg)
{
  integer_reflector_proxy_t* integer_reflector_proxy = malloc (sizeof (integer_reflector_proxy_t));
  integer_reflector_proxy->have_x = false;
  integer_reflector_proxy->automan = automan_creat (integer_reflector_proxy,
						    &integer_reflector_proxy->self);
  assert (automan_input_add (integer_reflector_proxy->automan,
			      &integer_reflector_proxy->in_composed,
			      integer_reflector_proxy_integer_in,
			      NULL,
			      composed,
			      NULL) == 0);
  assert (automan_output_add (integer_reflector_proxy->automan,
			      &integer_reflector_proxy->out_composed,
			      integer_reflector_proxy_integer_out,
			      NULL,
			      composed,
			      NULL) == 0);

  return integer_reflector_proxy;
}

static void
integer_reflector_proxy_system_input (void* state,
		       void* param,
		       bid_t bid)
{
  integer_reflector_proxy_t* integer_reflector_proxy = state;
  assert (integer_reflector_proxy != NULL);

  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));
  const receipt_t* receipt = buffer_read_ptr (bid);

  automan_apply (integer_reflector_proxy->automan, receipt);
}

static bid_t
integer_reflector_proxy_system_output (void* state,
			void* param)
{
  integer_reflector_proxy_t* integer_reflector_proxy = state;
  assert (integer_reflector_proxy != NULL);

  return automan_action (integer_reflector_proxy->automan);
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
  integer_reflector_proxy->have_x = true;
  assert (schedule_output (integer_reflector_proxy_integer_out, NULL) == 0);
}

bid_t
integer_reflector_proxy_integer_out (void* state,
				     void* param)
{
  integer_reflector_proxy_t* integer_reflector_proxy = state;
  assert (integer_reflector_proxy != NULL);

  if (integer_reflector_proxy->have_x &&
      integer_reflector_proxy->out_composed) {
    bid_t bid = buffer_alloc (sizeof (int));
    int* x = buffer_write_ptr (bid);
    *x = integer_reflector_proxy->x;
    return bid;
  }
  else {
    return -1;
  }
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
  .system_input = integer_reflector_proxy_system_input,
  .system_output = integer_reflector_proxy_system_output,
  .inputs = integer_reflector_proxy_inputs,
  .outputs = integer_reflector_proxy_outputs,
};
