#ifndef __runq_h__
#define __runq_h__

#include <stddef.h>
#include <stdbool.h>

#include "types.h"

typedef enum {
  SYSTEM_INPUT,
  SYSTEM_OUTPUT,
  OUTPUT,
  INTERNAL,
} runnable_type_t;

typedef struct runq_struct runq_t;
typedef struct runnable_struct runnable_t;

runq_t* runq_create (void);
void runq_destroy (runq_t*);

size_t runq_size (runq_t*);
bool runq_empty (runq_t*);

void runq_insert_system_input (runq_t*, aid_t);
void runq_insert_system_output (runq_t*, aid_t);
void runq_insert_output (runq_t*, aid_t, output_t);
void runq_insert_internal (runq_t*, aid_t, internal_t);

runnable_t* runq_pop (runq_t*);
void runq_purge (runq_t*, aid_t);

runnable_type_t runnable_type (runnable_t*);
aid_t runnable_aid (runnable_t*);

output_t runnable_output_output (runnable_t*);

internal_t runnable_internal_internal (runnable_t*);

void runnable_destroy (runnable_t*);

void runq_dump (runq_t*);

#endif /* __runq_h__ */
