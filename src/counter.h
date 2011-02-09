#ifndef __counter_h__
#define __counter_h__

#include "ueioa.h"

void counter_input (state_ptr_t, bid_t);

typedef struct {
  int count;
} counter_output_t;
bid_t counter_output (state_ptr_t);

extern automaton_descriptor_t counter_descriptor;

#endif /* __counter_h__ */
