#include "integer_display_proxy.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "component_manager.h"

typedef struct {
  aid_t aid;
  input_t free_input;
} integer_display_proxy_t;

static void*
integer_display_proxy_create (void* a)
{
  integer_display_proxy_create_arg_t* arg = a;
  assert (arg != NULL);

  integer_display_proxy_t* integer_display_proxy = malloc (sizeof (integer_display_proxy_t));

  integer_display_proxy->aid = arg->aid;
  integer_display_proxy->free_input = arg->free_input;

  return integer_display_proxy;
}

static void
integer_display_proxy_system_input (void* state, void* param, bid_t bid)
{
  integer_display_proxy_t* integer_display_proxy = state;
  assert (integer_display_proxy != NULL);
  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));
  const receipt_t* receipt = buffer_read_ptr (bid);

  if (receipt->type == SELF_CREATED) {
    bid_t bid = proxy_receipt_create (receipt->self_created.self, -1);
    assert (schedule_free_input (integer_display_proxy->aid, integer_display_proxy->free_input, bid) == 0);
  }
}

void
integer_display_proxy_integer_in (void* state, void* param, bid_t bid)
{
  assert (buffer_size (bid) == sizeof (int));
  const int* ptr = buffer_read_ptr (bid);
  printf (" in: %d\n", ntohl(*ptr));
}

static input_t integer_display_proxy_inputs[] = {
  integer_display_proxy_integer_in,
  NULL
};

descriptor_t integer_display_proxy_descriptor = {
  .constructor = integer_display_proxy_create,
  .system_input = integer_display_proxy_system_input,
  .inputs = integer_display_proxy_inputs,
};
