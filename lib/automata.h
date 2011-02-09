#ifndef __automata_h__
#define __automata_h__

#include <ueioa.h>

#include "buffers.h"

typedef struct automata_struct automata_t;

automata_t* automata_create (void);
void automata_destroy (automata_t*);

aid_t automata_create_automaton (automata_t*, descriptor_t*, aid_t);

#define A_OUTPUT_DNE 0
#define A_INPUT_DNE 1
#define A_OUTPUT_UNAVAILABLE 2
#define A_INPUT_UNAVAILABLE 3
#define A_COMPOSED 4
int automata_compose (automata_t*, aid_t, aid_t, output_t, aid_t, input_t);
#define A_NOT_COMPOSER 0
#define A_NOT_COMPOSED 1
#define A_DECOMPOSED 2
int automata_decompose (automata_t*, aid_t, aid_t, output_t, aid_t, input_t);

void automata_system_input_exec (automata_t*, buffers_t*, aid_t, const receipt_t*);
#define A_GOOD_ORDER 1
#define A_NO_ORDER 0
#define A_BAD_ORDER -1
int automata_system_output_exec (automata_t*, buffers_t*, aid_t, order_t*);
void automata_output_exec (automata_t*, buffers_t*, aid_t, output_t);
void automata_internal_exec (automata_t*, aid_t, internal_t);

aid_t automata_get_current_aid (automata_t*);
bool automata_output_exists (automata_t*, aid_t, output_t);
bool automata_internal_exists (automata_t*, aid_t, internal_t);

#endif /* __automata_h__ */
