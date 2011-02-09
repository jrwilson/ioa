#ifndef __types_h__
#define __types_h__

#include <stddef.h>
#include <stdbool.h>

typedef int aid_t;
typedef int bid_t;

typedef void* state_ptr_t;
typedef state_ptr_t (*constructor_t) (void);
typedef void (*input_t) (state_ptr_t, bid_t);
typedef bid_t (*output_t) (state_ptr_t);
typedef void (*internal_t) (state_ptr_t);

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
} automaton_descriptor_t;

typedef enum {
  SYS_CREATE,
  SYS_COMPOSE,
  SYS_DECOMPOSE,
  SYS_DESTROY
} order_type_t;

typedef struct {
  order_type_t type;
  union {
    struct {
      automaton_descriptor_t* descriptor;
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
} system_order_t;

typedef enum {
  SYS_SELF_CREATED,
  SYS_CHILD_CREATED,
  SYS_BAD_DESCRIPTOR,
  SYS_OUTPUT_DNE,
  SYS_INPUT_DNE,
  SYS_OUTPUT_UNAVAILABLE,
  SYS_INPUT_UNAVAILABLE,
  SYS_COMPOSED,
  SYS_OUTPUT_COMPOSED,
  SYS_DECOMPOSED,
  SYS_OUTPUT_DECOMPOSED,
  SYS_NOT_COMPOSED,
  SYS_AUTOMATON_DNE,
  SYS_NOT_OWNER,
  SYS_CHILD_DESTROYED
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
} system_receipt_t;

#endif /* __types_h__ */
