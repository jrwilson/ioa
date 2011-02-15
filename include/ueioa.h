#ifndef __ueioa_h__
#define __ueioa_h__

#include <stddef.h>
#include <stdbool.h>
#include <time.h>

typedef int aid_t;
typedef int bid_t;

typedef void* (*constructor_t) (void);
typedef void (*input_t) (void*, void*, bid_t);
typedef bid_t (*output_t) (void*, void*);
typedef void (*internal_t) (void*, void*);

typedef struct {
  constructor_t constructor;
  input_t system_input;
  output_t system_output;
  input_t* inputs;
  output_t* outputs;
  internal_t* internals;
} descriptor_t;

typedef enum {
  CREATE,
  DECLARE,
  COMPOSE,
  DECOMPOSE,
  RESCIND,
  DESTROY,
} order_type_t;

typedef struct {
  order_type_t type;
  union {
    struct {
      descriptor_t* descriptor;
    } create;
    struct {
      void* param;
    } declare;
    struct {
      aid_t out_aid;
      output_t output;
      void* out_param;
      aid_t in_aid;
      input_t input;
      void* in_param;
    } compose;
    struct {
      aid_t out_aid;
      output_t output;
      void* out_param;
      aid_t in_aid;
      input_t input;
      void* in_param;
    } decompose;
    struct {
      void* param;
    } rescind;
    struct {
      aid_t aid;
    } destroy;
  };
} order_t;

typedef enum {
  BAD_ORDER,

  SELF_CREATED,
  CHILD_CREATED,
  BAD_DESCRIPTOR,

  DECLARED,

  OUTPUT_DNE,
  INPUT_DNE,
  OUTPUT_UNAVAILABLE,
  INPUT_UNAVAILABLE,
  COMPOSED,
  INPUT_COMPOSED,
  OUTPUT_COMPOSED,

  NOT_COMPOSED,
  DECOMPOSED,
  INPUT_DECOMPOSED,
  OUTPUT_DECOMPOSED,

  RESCINDED,

  AUTOMATON_DNE,
  NOT_OWNER,
  CHILD_DESTROYED,

  ALARM,
  WRITE_READY,
  READ_READY
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
      input_t input;
      void* in_param;
    } input_composed;
    struct {
      output_t output;
      void* out_param;
    } output_composed;
    struct {
      aid_t in_aid;
      input_t input;
      void* in_param;
    } decomposed;
    struct {
      input_t input;
      void* in_param;
    } input_decomposed;
    struct {
      output_t output;
      void* out_param;
    } output_decomposed;
    struct {
      aid_t child;
    } child_destroyed;
  };
} receipt_t;

bool descriptor_check (descriptor_t*);

void order_create_init (order_t*, descriptor_t*);
void order_declare_init (order_t*, void*);
void order_compose_init (order_t*, aid_t, output_t, void*, aid_t, input_t, void*);
void order_decompose_init (order_t*, aid_t, output_t, void*, aid_t, input_t, void*);
void order_rescind_init (order_t*, void*);
void order_destroy_init (order_t*, aid_t);

void ueioa_run (descriptor_t*, int);

void schedule_system_output (void);
int schedule_output (output_t, void*);
int schedule_internal (internal_t, void*);

void schedule_alarm (time_t, long);
void schedule_write_ready (int);
void schedule_read_ready (int);

bid_t buffer_alloc (size_t);
void* buffer_write_ptr (bid_t);
const void* buffer_read_ptr (bid_t);
size_t buffer_size (bid_t);
void buffer_incref (bid_t);
void buffer_decref (bid_t);
void buffer_add_child (bid_t, bid_t);
void buffer_remove_child (bid_t, bid_t);
bid_t buffer_dup (bid_t);

typedef struct manager_struct manager_t;

manager_t* manager_create (void);

void manager_self_set (manager_t*, aid_t*);
void manager_parent_set (manager_t*, aid_t*);
void manager_automaton_add (manager_t*, aid_t*, descriptor_t*);
void manager_composition_add (manager_t*, aid_t*, output_t, void*, aid_t*, input_t, void*);

bool manager_apply (manager_t*, const receipt_t*);
bid_t manager_action (manager_t*);

#endif /* __ueioa_h__ */
