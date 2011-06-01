#ifndef __integer_display_h__
#define __integer_display_h__

#include <ueioa.h>

extern descriptor_t integer_display_descriptor;

void integer_display_request_proxy (void* state, void* param, bid_t bid);

#endif
