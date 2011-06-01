#ifndef __port_type_descriptor_h__
#define __port_type_descriptor_h__

#include <stdint.h>
#include <ueioa.h>

typedef struct {
  char* name;
  char* type;
  input_t input;
} input_message_t;

typedef struct {
  char* name;
  char* type;
  output_t output;
} output_message_t;

typedef struct {
  uint32_t cardinality; /* 0 means infinity. */
  input_message_t* input_messages; /* NULL terminated array of input messages. */
  output_message_t* output_messages; /* NULL terminated array of output messages. */
} port_type_descriptor_t;

#endif
