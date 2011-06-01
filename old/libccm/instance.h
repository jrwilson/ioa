#ifndef __instance_h__
#define __instance_h__

#include <ccm.h>

typedef struct instance_create_arg_struct {
  aid_t component_aid;
  input_t request_proxy;
  const port_descriptor_t* port_descriptors;
  uint32_t input_count;
  uint32_t output_count;
  uuid_t component;
  uint32_t port;
  uint32_t instance;
} instance_create_arg_t;

void instance_create_arg_init (instance_create_arg_t*,
			       aid_t component_aid,
			       input_t request_proxy,
			       const port_descriptor_t* port_descriptors,
			       uint32_t input_count,
			       uint32_t output_count,
			       uuid_t component,
			       uint32_t port,
			       uint32_t instance);
#endif
