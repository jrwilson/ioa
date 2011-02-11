#ifndef __receipts_h__
#define __receipts_h__

#include <ueioa.h>

typedef struct receipts_struct receipts_t;

receipts_t* receipts_create (void);
void receipts_destroy (receipts_t*);

void receipts_push_bad_order (receipts_t*, aid_t);

void receipts_push_self_created (receipts_t*, aid_t, aid_t, aid_t);
void receipts_push_child_created (receipts_t*, aid_t, aid_t);
void receipts_push_bad_descriptor (receipts_t*, aid_t);

void receipts_push_output_dne (receipts_t*, aid_t);
void receipts_push_input_dne (receipts_t*, aid_t);
void receipts_push_output_unavailable (receipts_t*, aid_t);
void receipts_push_input_unavailable (receipts_t*, aid_t);
void receipts_push_composed (receipts_t*, aid_t);
void receipts_push_output_composed (receipts_t*, aid_t, output_t);

void receipts_push_not_composer (receipts_t*, aid_t);
void receipts_push_not_composed (receipts_t*, aid_t);
void receipts_push_decomposed (receipts_t*, aid_t, aid_t, input_t, void*);
void receipts_push_output_decomposed (receipts_t*, aid_t, output_t);

void receipts_push_automaton_dne (receipts_t*, aid_t);
void receipts_push_not_owner (receipts_t*, aid_t);
void receipts_push_child_destroyed (receipts_t*, aid_t, aid_t);

int receipts_pop (receipts_t*, aid_t, receipt_t*);
bool receipts_empty (receipts_t*, aid_t);

void receipts_purge_aid (receipts_t*, aid_t);

#endif /* __receipts_h__ */
