#include "integer_display_proxy.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "component_manager.h"

void
integer_display_proxy_integer_in (void* state, void* param, bid_t bid)
{
  /* assert (buffer_size (bid) == sizeof (int)); */
  /* const int* ptr = buffer_read_ptr (bid); */
  /* printf ("in: %d\n", ntohl(*ptr)); */
}

static input_t integer_display_proxy_inputs[] = {
  integer_display_proxy_integer_in,
  NULL
};

descriptor_t integer_display_proxy_descriptor = {
  .inputs = integer_display_proxy_inputs,
};
