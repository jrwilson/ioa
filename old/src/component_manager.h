#ifndef __component_manager_h__
#define __component_manager_h__

#include <ueioa.h>
#include <uuid/uuid.h>
#include "port_type_descriptor.h"

typedef struct {
  descriptor_t* component_descriptor;
  void* create_arg;
  input_t request_proxy;
  uuid_t component;
  port_type_descriptor_t* port_type_descriptors;
  aid_t msg_sender;
  aid_t msg_receiver;
} component_manager_create_arg_t;

void component_manager_create_arg_init (component_manager_create_arg_t*,
					descriptor_t*,
					void*,
					input_t,
					uuid_t,
					port_type_descriptor_t*,
					aid_t,
					aid_t);
typedef struct {
  uint32_t port_type;
} port_request_t;

void port_request_init (port_request_t*, uint32_t);

typedef struct {
  uint32_t port;
} port_receipt_t;

void port_receipt_init (port_receipt_t*, uint32_t);

void component_manager_request_port (void*, void*, bid_t);
void component_manager_strobe_in (void*, void*, bid_t);

extern descriptor_t component_manager_descriptor;

#endif
