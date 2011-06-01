#ifndef __ramp_h__
#define __ramp_h__

#include <ccm.h>

extern descriptor_t ramp_descriptor;

void ramp_request_proxy (void* state, void* param, bid_t bid);

extern const port_descriptor_t ramp_port_descriptors[];

#endif
