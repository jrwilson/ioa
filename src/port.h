#ifndef __port_h__
#define __port_h__

#include <ueioa.h>
#include "port_manager.h"

typedef struct port_create_arg_struct {
  aid_t* component_aid;
  input_t request_proxy;
  port_descriptor_t* port_descriptors;
  size_t input_count;
  size_t output_count;
  size_t component;
  size_t port_type;
  size_t port;
  aid_t aid;
  input_t free_input;
} port_create_arg_t;

void port_create_arg_init (port_create_arg_t*,
			   aid_t* component_aid,
			   input_t request_proxy,
			   port_descriptor_t* port_descriptors,
			   size_t input_count,
			   size_t output_count,
			   size_t component,
			   size_t port_type,
			   size_t port,
			   aid_t aid,
			   input_t free_input);
void port_strobe (void*, void*, bid_t);
void port_in (void*, void*, bid_t);
bid_t port_out (void*, void*);

extern descriptor_t port_descriptor;

#endif
