#ifndef __automata_h__
#define __automata_h__

#include <ueioa.h>

#include "buffers.h"
#include "receipts.h"
#include "runq.h"

typedef struct automata_struct automata_t;

automata_t* automata_create (void);
void automata_destroy (automata_t*);

void automata_create_automaton (automata_t*, receipts_t*, runq_t*, descriptor_t*);

void automata_system_input_exec (automata_t*, receipts_t*, runq_t*, buffers_t*, aid_t);
void automata_system_output_exec (automata_t*, receipts_t*, runq_t*, buffers_t*, aid_t);
void automata_output_exec (automata_t*, buffers_t*, aid_t, output_t);
void automata_internal_exec (automata_t*, aid_t, internal_t);

aid_t automata_get_current_aid (automata_t*);
bool automata_output_exists (automata_t*, aid_t, output_t);
bool automata_internal_exists (automata_t*, aid_t, internal_t);

#endif /* __automata_h__ */
