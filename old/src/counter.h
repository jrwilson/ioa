#ifndef __counter_h__
#define __counter_h__

#include <ueioa.h>

void counter_input (void*, void*, bid_t);

typedef struct {
  int count;
} counter_output_t;
bid_t counter_output (void*, void*);

extern descriptor_t counter_descriptor;

#endif /* __counter_h__ */
