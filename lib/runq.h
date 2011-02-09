#ifndef __runq_h__
#define __runq_h__

#include <ueioa.h>

typedef enum {
  SYSTEM_INPUT,
  SYSTEM_OUTPUT,
  OUTPUT,
  INTERNAL,
} runnable_type_t;

typedef struct {
  runnable_type_t type;
  aid_t aid;
  union {
    struct {
      output_t output;
    } output;
    struct {
      internal_t internal;
    } internal;
  };
} runnable_t;

typedef struct runq_struct runq_t;

runq_t* runq_create (void);
void runq_destroy (runq_t*);

size_t runq_size (runq_t*);
bool runq_empty (runq_t*);

void runq_insert_system_input (runq_t*, aid_t);
void runq_insert_system_output (runq_t*, aid_t);
void runq_insert_output (runq_t*, aid_t, output_t);
void runq_insert_internal (runq_t*, aid_t, internal_t);

void runq_pop (runq_t*, runnable_t*);
void runq_purge (runq_t*, aid_t);

#endif /* __runq_h__ */
