#ifndef __runq_h__
#define __runq_h__

#include <ueioa.h>

typedef enum {
  SYSTEM_INPUT,
  SYSTEM_OUTPUT,
  ALARM_INPUT,
  READ_INPUT,
  WRITE_INPUT,
  FREE_INPUT,
  OUTPUT,
  INTERNAL,
} runnable_type_t;

typedef struct {
  runnable_type_t type;
  aid_t aid;
  void* param;
  union {
    struct {
      input_t free_input;
      bid_t bid;
    } free_input;
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
void runq_insert_alarm_input (runq_t*, aid_t);
void runq_insert_read_input (runq_t*, aid_t);
void runq_insert_write_input (runq_t*, aid_t);
void runq_insert_free_input (runq_t*, aid_t, input_t, bid_t);
void runq_insert_output (runq_t*, aid_t, output_t, void*);
void runq_insert_internal (runq_t*, aid_t, internal_t, void*);

void runq_pop (runq_t*, runnable_t*);
void runq_purge_aid (runq_t*, aid_t);
void runq_purge_aid_param (runq_t*, aid_t, void*);

#endif /* __runq_h__ */
