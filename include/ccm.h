#ifndef __ccm_h__
#define __ccm_h__

#include <ueioa.h>
#include <stdint.h>
#include <uuid/uuid.h>

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

typedef struct {
  const descriptor_t* descriptor;
  const void* create_arg;
  input_t request_proxy;
  uuid_t id;
  const port_type_descriptor_t* port_type_descriptors;
  aid_t msg_sender;
  aid_t msg_receiver;
} component_create_arg_t;

void
component_create_arg_init (component_create_arg_t*,
			   const descriptor_t* descriptor,
			   const void* create_arg,
			   input_t request_proxy,
			   const uuid_t id,
			   const port_type_descriptor_t* port_type_descriptors,
			   aid_t msg_sender,
			   aid_t msg_receiver);

extern descriptor_t component_descriptor;

bid_t
port_request_create (uint32_t port_type);
void
component_request_port (void*,
			void*,
			bid_t);
typedef struct {
  uint32_t port;
} port_receipt_t;

extern descriptor_t port_descriptor;

void
port_in (void*,
	 void*,
	 bid_t);

bid_t
port_out (void*,
	  void*);

typedef struct message_struct {
  uuid_t src_component;
  uint32_t src_port_type;
  uint32_t src_port;
  uint32_t src_message;

  uuid_t dst_component;
  uint32_t dst_port_type;
  uint32_t dst_port;
  uint32_t dst_message;

  bid_t bid;
} message_t;

#endif
