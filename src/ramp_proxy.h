#ifndef __ramp_proxy_h__
#define __ramp_proxy_h__

#include <ueioa.h>

typedef struct {
  aid_t aid;
  input_t free_input;
} ramp_proxy_create_arg_t;

extern descriptor_t ramp_proxy_descriptor;

void ramp_proxy_integer_in (void* state, void* param, bid_t bid);
bid_t ramp_proxy_integer_out (void* state, void* param);

#endif
