#ifndef __ueioa_h__
#define __ueioa_h__

#include "types.h"

bool automaton_descriptor_check (automaton_descriptor_t*);

void system_order_create (system_order_t*, automaton_descriptor_t*);
void system_order_compose (system_order_t*, aid_t, output_t, aid_t, input_t);
void system_order_decompose (system_order_t*, aid_t, output_t, aid_t, input_t);
void system_order_destroy (system_order_t*, aid_t);

void ueioa_run (automaton_descriptor_t*);

void ueioa_schedule_system_output (void);
int ueioa_schedule_output (output_t);
int ueioa_schedule_internal (internal_t);

bid_t ueioa_buffer_alloc (size_t);
void* ueioa_buffer_write_ptr (bid_t);
const void* ueioa_buffer_read_ptr (bid_t);
size_t ueioa_buffer_size (bid_t);

#endif /* __ueioa_h__ */
