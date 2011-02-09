#ifndef __automata_h__
#define __automata_h__

#include "types.h"

#define COMPOSED 0
#define OUTPUT_DNE -1
#define INPUT_DNE -2
#define OUTPUT_UNAVAILABLE -3
#define INPUT_UNAVAILABLE -4

typedef struct automata_struct automata_t;

automata_t* automata_create (void);
void automata_destroy (automata_t*);

aid_t automaton_create (automata_t*, automaton_descriptor_t*, aid_t);
int automaton_compose (automata_t*, aid_t, aid_t, output_t, aid_t, input_t);

void automaton_system_input_exec (automata_t*, aid_t, const system_receipt_t*);
int automaton_system_output_exec (automata_t*, aid_t, system_order_t*);
void automaton_output_exec (automata_t*, aid_t, output_t);
void automaton_internal_exec (automata_t*, aid_t, internal_t);

aid_t automaton_get_current_aid (automata_t*);
bool automaton_output_exists (automata_t*, aid_t, output_t);
bool automaton_internal_exists (automata_t*, aid_t, internal_t);

bid_t automaton_buffer_alloc (automata_t*, size_t);
void* automaton_buffer_write_ptr (automata_t*, bid_t);
const void* automaton_buffer_read_ptr (automata_t*, bid_t);
size_t automaton_buffer_size (automata_t*, bid_t);

#endif /* __automata_h__ */
