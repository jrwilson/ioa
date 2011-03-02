#ifndef __port_manager_h__
#define __port_manager_h__

#include <ueioa.h>

typedef struct {
  size_t cardinality; /* 0 means infinity. */
  input_t* input_messages;
  output_t* output_messages;
} port_descriptor_t;

typedef struct {
  descriptor_t* component_descriptor;
  void* create_arg;
  input_t request_proxy;
  size_t component;
  port_descriptor_t* port_descriptors;
} port_manager_create_arg_t;

void port_manager_create_arg_init (port_manager_create_arg_t*,
				   descriptor_t*,
				   void*,
				   input_t,
				   size_t,
				   port_descriptor_t*);
typedef struct {
  size_t port_type;
} port_request_t;

void port_request_init (port_request_t*, size_t);

typedef struct {
  size_t port;
} port_receipt_t;

void port_receipt_init (port_receipt_t*, size_t);

void port_manager_request_port (void*, void*, bid_t);

extern descriptor_t port_manager_descriptor;

#endif
