#include "port_allocator.h"

#include <assert.h>
#include <stdlib.h>

#include "bitset.h"

typedef struct {
  uint32_t cardinality;
  bitset_t* bitset;
  uint32_t input_message_count;
  uint32_t output_message_count;
} port_t;

struct port_allocator_struct {
  uint32_t port_count;
  port_t* ports;
};

port_allocator_t*
port_allocator_create (const port_descriptor_t* port_descriptors)
{
  assert (port_descriptors != NULL);

  uint32_t port_count;
  for (port_count = 0;
       port_descriptors[port_count].input_message_descriptors != NULL;
       ++port_count)
    ;;
  assert (port_count > 0);


  assert (port_count > 0);

  port_allocator_t* port_allocator = malloc (sizeof (port_allocator_t));

  port_allocator->port_count = port_count;
  port_allocator->ports = malloc (port_count * sizeof (port_t));

  uint32_t idx;
  for (idx = 0; idx < port_allocator->port_count; ++idx) {
    port_allocator->ports[idx].cardinality = port_descriptors[idx].cardinality;
    if (port_allocator->ports[idx].cardinality == 0) {
      port_allocator->ports[idx].bitset = bitset_create (1);
    }
    else {
      port_allocator->ports[idx].bitset = bitset_create (port_allocator->ports[idx].cardinality);
    }

    uint32_t input_message_count = 0;
    for (input_message_count = 0;
	 port_descriptors[idx].input_message_descriptors[input_message_count].input != NULL;
	 ++input_message_count)
      ;;
    port_allocator->ports[idx].input_message_count = input_message_count;

    uint32_t output_message_count = 0;
    for (output_message_count = 0;
	 port_descriptors[idx].output_message_descriptors[output_message_count].output != NULL;
	 ++output_message_count)
      ;;
    port_allocator->ports[idx].output_message_count = output_message_count;
  }

  return port_allocator;
}

uint32_t
port_allocator_port_count (port_allocator_t* port_allocator)
{
  assert (port_allocator != NULL);

  return port_allocator->port_count;
}

bool
port_allocator_contains_free_instance (port_allocator_t* port_allocator, uint32_t idx)
{
  assert (port_allocator != NULL);
  assert (idx < port_allocator->port_count);

  if (port_allocator->ports[idx].cardinality == 0) {
    return true;
  }
  else {
    return !bitset_full (port_allocator->ports[idx].bitset);
  }
}

uint32_t
port_allocator_get_free_instance (port_allocator_t* port_allocator, uint32_t idx)
{
  assert (port_allocator != NULL);
  assert (idx < port_allocator->port_count);

  if (port_allocator->ports[idx].cardinality == 0) {
    if (bitset_full (port_allocator->ports[idx].bitset)) {
      bitset_reserve (port_allocator->ports[idx].bitset, 2 * bitset_capacity (port_allocator->ports[idx].bitset));
    }
  }
  else {
    assert (!bitset_full (port_allocator->ports[idx].bitset));
  }

  /* Could improve by remembering the last cleared and/or set. */
  uint32_t retval = bitset_next_clear (port_allocator->ports[idx].bitset, 0);
  bitset_set (port_allocator->ports[idx].bitset, retval);
  return retval;
}

uint32_t
port_allocator_cardinality (port_allocator_t* port_allocator, uint32_t idx)
{
  assert (port_allocator != NULL);
  assert (idx < port_allocator->port_count);

  return port_allocator->ports[idx].cardinality;
}

uint32_t
port_allocator_free_count (port_allocator_t* port_allocator, uint32_t idx)
{
  assert (port_allocator != NULL);
  assert (idx < port_allocator->port_count);

  return bitset_size (port_allocator->ports[idx].bitset);
}

uint32_t
port_allocator_input_message_count (port_allocator_t* port_allocator, uint32_t idx)
{
  assert (port_allocator != NULL);
  assert (idx < port_allocator->port_count);

  return port_allocator->ports[idx].input_message_count;
}

uint32_t
port_allocator_output_message_count (port_allocator_t* port_allocator, uint32_t idx)
{
  assert (port_allocator != NULL);
  assert (idx < port_allocator->port_count);

  return port_allocator->ports[idx].output_message_count;
}
