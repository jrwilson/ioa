#ifndef __integer_reflector_proxy_h__
#define __integer_reflector_proxy_h__

#include <ueioa.h>

extern const descriptor_t integer_reflector_proxy_descriptor;

void
integer_reflector_proxy_integer_in (void* state,
				    void* param,
				    bid_t bid);

bid_t
integer_reflector_proxy_integer_out (void* state,
				     void* param);

#endif
