#include "instance_allocator.h"

#include <assert.h>
#include <stdlib.h>

#include "bitset.h"

typedef struct {
  uint32_t cardinality;
  bitset_t* bitset;
  uint32_t input_message_count;
  uint32_t output_message_count;
} port_t;

struct instance_allocator_struct {
  uint32_t port_count;
  port_t* ports;
};

instance_allocator_t*
instance_allocator_create (const port_descriptor_t* port_descriptors)
{
  assert (port_descriptors != NULL);

  uint32_t port_count;
  for (port_count = 0;
       port_descriptors[port_count].input_message_descriptors != NULL;
       ++port_count)
    ;;
  assert (port_count > 0);


  assert (port_count > 0);

  instance_allocator_t* instance_allocator = malloc (sizeof (instance_allocator_t));

  instance_allocator->port_count = port_count;
  instance_allocator->ports = malloc (port_count * sizeof (port_t));

  uint32_t idx;
  for (idx = 0; idx < instance_allocator->port_count; ++idx) {
    instance_allocator->ports[idx].cardinality = port_descriptors[idx].cardinality;
    if (instance_allocator->ports[idx].cardinality == 0) {
      instance_allocator->ports[idx].bitset = bitset_create (1);
    }
    else {
      instance_allocator->ports[idx].bitset = bitset_create (instance_allocator->ports[idx].cardinality);
    }

    uint32_t input_message_count = 0;
    for (input_message_count = 0;
	 port_descriptors[idx].input_message_descriptors[input_message_count].input != NULL;
	 ++input_message_count)
      ;;
    instance_allocator->ports[idx].input_message_count = input_message_count;

    uint32_t output_message_count = 0;
    for (output_message_count = 0;
	 port_descriptors[idx].output_message_descriptors[output_message_count].output != NULL;
	 ++output_message_count)
      ;;
    instance_allocator->ports[idx].output_message_count = output_message_count;
  }

  return instance_allocator;
}

uint32_t
instance_allocator_port_count (instance_allocator_t* instance_allocator)
{
  assert (instance_allocator != NULL);

  return instance_allocator->port_count;
}

bool
instance_allocator_contains_free_instance (instance_allocator_t* instance_allocator,
					   uint32_t port)
{
  assert (instance_allocator != NULL);
  assert (port < instance_allocator->port_count);

  if (instance_allocator->ports[port].cardinality == 0) {
    return true;
  }
  else {
    return !bitset_full (instance_allocator->ports[port].bitset);
  }
}

uint32_t
instance_allocator_get_instance (instance_allocator_t* instance_allocator,
				 uint32_t port)
{
  assert (instance_allocator != NULL);
  assert (port < instance_allocator->port_count);

  if (instance_allocator->ports[port].cardinality == 0) {
    if (bitset_full (instance_allocator->ports[port].bitset)) {
      bitset_reserve (instance_allocator->ports[port].bitset, 2 * bitset_capacity (instance_allocator->ports[port].bitset));
    }
  }
  else {
    assert (!bitset_full (instance_allocator->ports[port].bitset));
  }

  /* Could improve by remembering the last cleared and/or set. */
  uint32_t retval = bitset_next_clear (instance_allocator->ports[port].bitset, 0);
  bitset_set (instance_allocator->ports[port].bitset, retval);
  return retval;
}

void
instance_allocator_return_instance (instance_allocator_t* instance_allocator,
				    uint32_t port,
				    uint32_t instance)
{
  assert (instance_allocator != NULL);
  assert (port < instance_allocator->port_count);

  assert (bitset_test (instance_allocator->ports[port].bitset, instance));
  bitset_clear (instance_allocator->ports[port].bitset, instance);
}

uint32_t
instance_allocator_cardinality (instance_allocator_t* instance_allocator,
				uint32_t port)
{
  assert (instance_allocator != NULL);
  assert (port < instance_allocator->port_count);

  return instance_allocator->ports[port].cardinality;
}

uint32_t
instance_allocator_free_count (instance_allocator_t* instance_allocator,
			       uint32_t port)
{
  assert (instance_allocator != NULL);
  assert (port < instance_allocator->port_count);

  return bitset_size (instance_allocator->ports[port].bitset);
}

uint32_t
instance_allocator_input_message_count (instance_allocator_t* instance_allocator,
					uint32_t port)
{
  assert (instance_allocator != NULL);
  assert (port < instance_allocator->port_count);

  return instance_allocator->ports[port].input_message_count;
}

uint32_t
instance_allocator_output_message_count (instance_allocator_t* instance_allocator,
					 uint32_t port)
{
  assert (instance_allocator != NULL);
  assert (port < instance_allocator->port_count);

  return instance_allocator->ports[port].output_message_count;
}
