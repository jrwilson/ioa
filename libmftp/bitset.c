#include "bitset.h"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define IDX_TO_BLOCK(idx) ((idx) >> 5)
#define IDX_TO_OFFSET(idx) ((idx) & (32 - 1))
#define BLOCK_OFFSET_TO_IDX(block, offset) (((block) << 5) | (offset))

struct bitset_struct {
  size_t capacity;
  size_t size;
  uint32_t data[];
};

bitset_t*
bitset_create (size_t capacity)
{
  assert (capacity > 0);

  size_t blocks = IDX_TO_BLOCK (capacity - 1) + 1;
  bitset_t* bitset = malloc (sizeof (bitset_t) + sizeof (uint32_t) * blocks);
  bitset->capacity = capacity;
  bitset->size = 0;
  memset (bitset->data, 0, sizeof (uint32_t) * blocks);
  return bitset;
}

void
bitset_set (bitset_t* bitset, size_t index)
{
  assert (bitset != NULL);
  assert (index < bitset->capacity);

  size_t block = IDX_TO_BLOCK (index);
  size_t offset = IDX_TO_OFFSET (index);
  uint32_t before = bitset->data[block];
  bitset->data[block] |= (1 << offset);
  if (bitset->data[block] != before) {
    ++bitset->size;
  }
}

bool
bitset_test (bitset_t* bitset, size_t index)
{
  assert (bitset != NULL);
  assert (index < bitset->capacity);

  size_t block = IDX_TO_BLOCK (index);
  size_t offset = IDX_TO_OFFSET (index);
  return (bitset->data[block] & (1 << offset)) != 0;
}

void
bitset_clear (bitset_t* bitset, size_t index)
{
  assert (bitset != NULL);
  assert (index < bitset->capacity);

  size_t block = IDX_TO_BLOCK (index);
  size_t offset = IDX_TO_OFFSET (index);
  uint32_t before = bitset->data[block];
  bitset->data[block] &= ~(1 << offset);
  if (bitset->data[block] != before) {
    --bitset->size;
  }
}

size_t
bitset_size (bitset_t* bitset)
{
  assert (bitset != NULL);

  return bitset->size;
}

bool
bitset_empty (bitset_t* bitset)
{
  assert (bitset != NULL);

  return bitset->size == 0;
}

bool
bitset_full (bitset_t* bitset)
{
  assert (bitset != NULL);

  return bitset->size == bitset->capacity;
}

size_t
bitset_capacity (bitset_t* bitset)
{
  assert (bitset != NULL);

  return bitset->capacity;
}

size_t
bitset_next_set (bitset_t* bitset, size_t index)
{
  assert (bitset != NULL);
  assert (bitset->size != 0);

  /* Wrap around. */
  if (index >= bitset->capacity) {
    index = 0;
  }

  size_t b;
  size_t block_count = IDX_TO_BLOCK (bitset->capacity - 1) + 1;
  for (b = 0; b < block_count; ++b) {
    size_t block = (IDX_TO_BLOCK (index) + b) % block_count;
    if (bitset->data[block] != 0) {
      /* Found a block with bits set. */
      size_t o;
      for (o = 0; o < 32; ++o) {
	/* It would be sweet to have some assembly for rotating the mask!! */
	size_t offset = (IDX_TO_OFFSET (index) + o) % 32;
	if ((bitset->data[block] & (1 << offset)) != 0) {
	  return BLOCK_OFFSET_TO_IDX (block, offset);
	}
      }
    }
  }

  /* Not reached. */
  assert (0);
  return -1;
}

size_t
bitset_next_clear (bitset_t* bitset, size_t index)
{
  assert (bitset != NULL);
  assert (bitset->size != bitset->capacity);

  /* Wrap around. */
  if (index >= bitset->capacity) {
    index = 0;
  }

  size_t b;
  size_t block_count = IDX_TO_BLOCK (bitset->capacity - 1) + 1;
  for (b = 0; b < block_count; ++b) {
    size_t block = (IDX_TO_BLOCK (index) + b) % block_count;
    if ((~bitset->data[block]) != 0) {
      /* Found a block with bits clear. */
      size_t o;
      for (o = 0; o < 32; ++o) {
	/* It would be sweet to have some assembly for rotating the mask!! */
	size_t offset = (IDX_TO_OFFSET (index) + o) % 32;
	if (((~bitset->data[block]) & (1 << offset)) != 0) {
	  return BLOCK_OFFSET_TO_IDX (block, offset);
	}
      }
    }
  }

  /* Not reached. */
  assert (0);
  return -1;
}

void
bitset_set_all (bitset_t* bitset)
{
  assert (bitset != NULL);

  size_t b;
  size_t block_count = IDX_TO_BLOCK (bitset->capacity - 1) + 1;
  for (b = 0; b < block_count; ++b) {
    bitset->data[b] = -1;
  }

  size_t block = IDX_TO_BLOCK (bitset->capacity - 1);
  bitset->data[block] = 0;
  size_t offset = IDX_TO_OFFSET (bitset->capacity);
  
  size_t o;
  for (o = 0; o < offset; ++o) {
    bitset->data[block] |= (1 << o);
  }

  bitset->size = bitset->capacity;
}
