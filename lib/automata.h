#ifndef __automata_h__
#define __automata_h__

#include <ueioa.h>

#include "runq.h"
#include "ioq.h"
#include "receipts.h"
#include "buffers.h"

typedef struct automata_struct automata_t;

automata_t* automata_create (void);
void automata_destroy (automata_t*);

void automata_create_automaton (automata_t*, receipts_t*, runq_t*, descriptor_t*);

void automata_system_input_exec (automata_t*, receipts_t*, runq_t*, buffers_t*, aid_t);
void automata_system_output_exec (automata_t*, receipts_t*, runq_t*, ioq_t*, buffers_t*, aid_t);
void automata_alarm_input_exec (automata_t*, buffers_t*, aid_t);
void automata_read_input_exec (automata_t*, buffers_t*, aid_t);
void automata_write_input_exec (automata_t*, buffers_t*, aid_t);
void automata_free_input_exec (automata_t*, buffers_t*, aid_t, input_t, bid_t);
void automata_output_exec (automata_t*, buffers_t*, aid_t, output_t, void*);
void automata_internal_exec (automata_t*, aid_t, internal_t, void*);

aid_t automata_get_current_aid (automata_t*);
bool automata_system_output_exists (automata_t*, aid_t);
bool automata_alarm_input_exists (automata_t*, aid_t);
bool automata_read_input_exists (automata_t*, aid_t);
bool automata_write_input_exists (automata_t*, aid_t);
bool automata_free_input_exists (automata_t*, aid_t, input_t);
bool automata_output_exists (automata_t*, aid_t, output_t, void*);
bool automata_internal_exists (automata_t*, aid_t, internal_t, void*);

#endif /* __automata_h__ */
