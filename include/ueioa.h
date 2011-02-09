#ifndef __ueioa_h__
#define __ueioa_h__

#include <stddef.h>
#include <stdbool.h>

typedef int aid_t;
typedef int bid_t;

typedef void* (*constructor_t) (void);
typedef void (*input_t) (void*, bid_t);
typedef bid_t (*output_t) (void*);
typedef void (*internal_t) (void*);

typedef struct {
  constructor_t constructor;
  input_t system_input;
  output_t system_output;
  size_t input_count;
  input_t* inputs;
  size_t output_count;
  output_t* outputs;
  size_t internal_count;
  internal_t* internals;
} descriptor_t;

typedef enum {
  CREATE,
  COMPOSE,
  DECOMPOSE,
  DESTROY
} order_type_t;

typedef struct {
  order_type_t type;
  union {
    struct {
      descriptor_t* descriptor;
    } create;
    struct {
      aid_t out_automaton;
      output_t output;
      aid_t in_automaton;
      input_t input;
    } compose;
    struct {
      aid_t out_automaton;
      output_t output;
      aid_t in_automaton;
      input_t input;
    } decompose;
    struct {
      aid_t automaton;
    } destroy;
  };
} order_t;

typedef enum {
  BAD_ORDER,

  SELF_CREATED,
  CHILD_CREATED,
  BAD_DESCRIPTOR,

  OUTPUT_DNE,
  INPUT_DNE,
  OUTPUT_UNAVAILABLE,
  INPUT_UNAVAILABLE,
  COMPOSED,
  OUTPUT_COMPOSED,

  DECOMPOSED,
  OUTPUT_DECOMPOSED,
  NOT_COMPOSED,
  AUTOMATON_DNE,
  NOT_OWNER,
  CHILD_DESTROYED
} receipt_type_t;

typedef struct {
  receipt_type_t type;
  union {
    struct {
      aid_t self;
      aid_t parent;
    } self_created;
    struct {
      aid_t child;
    } child_created;
    struct {
      output_t output;
    } output_composed;
    struct {
      output_t output;
    } output_decomposed;
    struct {
      aid_t child;
    } child_destroyed;
  };
} receipt_t;

bool descriptor_check (descriptor_t*);

void order_create_init (order_t*, descriptor_t*);
void order_compose_init (order_t*, aid_t, output_t, aid_t, input_t);
void order_decompose_init (order_t*, aid_t, output_t, aid_t, input_t);
void order_destroy_init (order_t*, aid_t);

void ueioa_run (descriptor_t*);

void schedule_system_output (void);
int schedule_output (output_t);
int schedule_internal (internal_t);

bid_t buffer_alloc (size_t);
void* buffer_write_ptr (bid_t);
const void* buffer_read_ptr (bid_t);
size_t buffer_size (bid_t);
void buffer_incref (bid_t);
void buffer_decref (bid_t);
void buffer_add_child (bid_t, bid_t);
void buffer_remove_child (bid_t, bid_t);
bid_t buffer_dup (bid_t);

#endif /* __ueioa_h__ */
