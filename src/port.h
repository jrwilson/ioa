#ifndef __port_h__
#define __port_h__

#include <ueioa.h>
#include "component_manager.h"

typedef struct port_create_arg_struct {
  aid_t* component_aid;
  input_t request_proxy;
  port_type_descriptor_t* port_type_descriptors;
  uint32_t input_count;
  uint32_t output_count;
  uuid_t component;
  uint32_t port_type;
  uint32_t port;
  aid_t aid;
  input_t free_input;
} port_create_arg_t;

void port_create_arg_init (port_create_arg_t*,
			   aid_t* component_aid,
			   input_t request_proxy,
			   port_type_descriptor_t* port_type_descriptors,
			   uint32_t input_count,
			   uint32_t output_count,
			   uuid_t component,
			   uint32_t port_type,
			   uint32_t port,
			   aid_t aid,
			   input_t free_input);
void port_strobe_in (void*, void*, bid_t);
void port_in (void*, void*, bid_t);
bid_t port_out (void*, void*);

extern descriptor_t port_descriptor;

#endif
