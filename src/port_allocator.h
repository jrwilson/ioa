#ifndef __port_allocator_h__
#define __port_allocator_h__

#include "port_manager.h"

typedef struct port_allocator_struct port_allocator_t;

port_allocator_t* port_allocator_create (port_descriptor_t* port_descriptors);
size_t port_allocator_port_set_count (port_allocator_t* port_allocator);
bool port_allocator_contains_free_port (port_allocator_t* port_allocator, size_t idx);
size_t port_allocator_get_free_port (port_allocator_t* port_allocator, size_t idx);
size_t port_allocator_input_count (port_allocator_t* port_allocator, size_t idx);
size_t port_allocator_output_count (port_allocator_t* port_allocator, size_t idx);

#endif
