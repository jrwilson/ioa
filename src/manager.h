#ifndef __manager_h__
#define __manager_h__

#include <ueioa.h>

typedef struct manager_struct manager_t;

manager_t* manager_create (void);

void manager_self_set (manager_t*, aid_t*);
void manager_parent_set (manager_t*, aid_t*);
void manager_automaton_add (manager_t*, aid_t*, automaton_descriptor_t*);
void manager_composition_add (manager_t*, aid_t*, output_t, aid_t*, input_t);

bool manager_apply (manager_t*, const system_receipt_t*);
bid_t manager_action (manager_t*);

#endif /* __manager_h__ */
