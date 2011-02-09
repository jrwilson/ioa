#ifndef __automata_h__
#define __automata_h__

#include <ueioa.h>

typedef struct automata_struct automata_t;

automata_t* automata_create (void);
void automata_destroy (automata_t*);

aid_t automata_create_automaton (automata_t*, descriptor_t*, aid_t);

#define COMPOSED 0
#define OUTPUT_DNE -1
#define INPUT_DNE -2
#define OUTPUT_UNAVAILABLE -3
#define INPUT_UNAVAILABLE -4
int automata_compose (automata_t*, aid_t, aid_t, output_t, aid_t, input_t);

void automata_system_input_exec (automata_t*, aid_t, const receipt_t*);
#define GOOD_ORDER 1
#define NO_ORDER 0
#define BAD_ORDER -1
int automata_system_output_exec (automata_t*, aid_t, order_t*);
void automata_output_exec (automata_t*, aid_t, output_t);
void automata_internal_exec (automata_t*, aid_t, internal_t);

aid_t automata_get_current_aid (automata_t*);
bool automata_output_exists (automata_t*, aid_t, output_t);
bool automata_internal_exists (automata_t*, aid_t, internal_t);

bid_t automata_buffer_alloc (automata_t*, size_t);
void* automata_buffer_write_ptr (automata_t*, bid_t);
const void* automata_buffer_read_ptr (automata_t*, bid_t);
size_t automata_buffer_size (automata_t*, bid_t);

#endif /* __automata_h__ */
