#ifndef __integer_reflector_h__
#define __integer_reflector_h__

#include <ccm.h>

extern const descriptor_t integer_reflector_descriptor;

void
integer_reflector_request_proxy (void* state,
				 void* param,
				 bid_t bid);

extern const port_type_descriptor_t integer_reflector_port_type_descriptors[];

#endif
