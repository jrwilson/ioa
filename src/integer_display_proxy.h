#ifndef __integer_display_proxy_h__
#define __integer_display_proxy_h__

#include <ueioa.h>

typedef struct {
  aid_t aid;
  input_t free_input;
} integer_display_proxy_create_arg_t;

void integer_display_proxy_integer_in (void* state, void* param, bid_t bid);

extern descriptor_t integer_display_proxy_descriptor;

#endif
