#ifndef __instance_allocator_h__
#define __instance_allocator_h__

#include <ccm.h>

typedef struct instance_allocator_struct instance_allocator_t;

instance_allocator_t*
instance_allocator_create (const port_descriptor_t* port_descriptors);
uint32_t
instance_allocator_port_count (instance_allocator_t* instance_allocator);
bool
instance_allocator_contains_free_instance (instance_allocator_t* instance_allocator,
					   uint32_t port);
uint32_t
instance_allocator_get_instance (instance_allocator_t* instance_allocator,
				 uint32_t port);
void
instance_allocator_return_instance (instance_allocator_t* instance_allocator,
				    uint32_t port,
				    uint32_t instance);
uint32_t
instance_allocator_cardinality (instance_allocator_t* instance_allocator,
				uint32_t port);
uint32_t
instance_allocator_free_count (instance_allocator_t* instance_allocator,
			       uint32_t port);
uint32_t
instance_allocator_input_message_count (instance_allocator_t* instance_allocator,
					uint32_t port);
uint32_t
instance_allocator_output_message_count (instance_allocator_t* instance_allocator,
					 uint32_t port);

#endif
