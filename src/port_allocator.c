#include "port_allocator.h"

#include <assert.h>
#include <stdlib.h>

#include "bitset.h"

typedef struct {
  size_t cardinality;
  bitset_t* bitset;
  size_t input_count;
  size_t output_count;
} port_set_t;

struct port_allocator_struct {
  size_t port_set_count;
  port_set_t* port_sets;
};

port_allocator_t*
port_allocator_create (port_descriptor_t* port_descriptors)
{
  assert (port_descriptors != NULL);

  size_t port_set_count;
  for (port_set_count = 0;
       port_descriptors[port_set_count].input_messages != NULL;
       ++port_set_count)
    ;;
  assert (port_set_count > 0);


  assert (port_set_count > 0);

  port_allocator_t* port_allocator = malloc (sizeof (port_allocator_t));

  port_allocator->port_set_count = port_set_count;
  port_allocator->port_sets = malloc (port_set_count * sizeof (port_set_t));

  size_t idx;
  for (idx = 0; idx < port_allocator->port_set_count; ++idx) {
    port_allocator->port_sets[idx].cardinality = port_descriptors[idx].cardinality;
    if (port_allocator->port_sets[idx].cardinality == 0) {
      port_allocator->port_sets[idx].bitset = bitset_create (1);
    }
    else {
      port_allocator->port_sets[idx].bitset = bitset_create (port_allocator->port_sets[idx].cardinality);
    }

    size_t input_count = 0;
    for (input_count = 0;
	 port_descriptors[idx].input_messages[input_count] != NULL;
	 ++input_count)
      ;;
    port_allocator->port_sets[idx].input_count = input_count;

    size_t output_count = 0;
    for (output_count = 0;
	 port_descriptors[idx].output_messages[output_count] != NULL;
	 ++output_count)
      ;;
    port_allocator->port_sets[idx].output_count = output_count;
  }

  return port_allocator;
}

uint32_t
port_allocator_port_set_count (port_allocator_t* port_allocator)
{
  assert (port_allocator != NULL);

  return port_allocator->port_set_count;
}

bool
port_allocator_contains_free_port (port_allocator_t* port_allocator, uint32_t idx)
{
  assert (port_allocator != NULL);
  assert (idx < port_allocator->port_set_count);

  if (port_allocator->port_sets[idx].cardinality == 0) {
    return true;
  }
  else {
    return !bitset_full (port_allocator->port_sets[idx].bitset);
  }
}

uint32_t
port_allocator_get_free_port (port_allocator_t* port_allocator, uint32_t idx)
{
  assert (port_allocator != NULL);
  assert (idx < port_allocator->port_set_count);

  if (port_allocator->port_sets[idx].cardinality == 0) {
    if (bitset_full (port_allocator->port_sets[idx].bitset)) {
      bitset_reserve (port_allocator->port_sets[idx].bitset, 2 * bitset_capacity (port_allocator->port_sets[idx].bitset));
    }
  }
  else {
    assert (!bitset_full (port_allocator->port_sets[idx].bitset));
  }

  /* Could improve by remembering the last cleared and/or set. */
  uint32_t retval = bitset_next_clear (port_allocator->port_sets[idx].bitset, 0);
  bitset_set (port_allocator->port_sets[idx].bitset, retval);
  return retval;
}

uint32_t
port_allocator_input_count (port_allocator_t* port_allocator, uint32_t idx)
{
  assert (port_allocator != NULL);
  assert (idx < port_allocator->port_set_count);

  return port_allocator->port_sets[idx].input_count;
}

uint32_t
port_allocator_output_count (port_allocator_t* port_allocator, uint32_t idx)
{
  assert (port_allocator != NULL);
  assert (idx < port_allocator->port_set_count);

  return port_allocator->port_sets[idx].output_count;
}
