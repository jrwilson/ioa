#include "hashtable.h"

#include <assert.h>
#include <string.h>
#include <stdlib.h>

/* TODO: Linked list instead of compacting.  Sorting. */
/* TODO: Hash size should always be power of 2.  Remove % and *. */

#define LOAD_NUMER 8
#define LOAD_DENOM 10

typedef struct node_struct node_t;
struct node_struct {
  size_t prev;
  size_t next;
  size_t code;
  size_t next_hash;
  unsigned char data[0];
};

struct hashtable_struct {
  size_t key_size;
  hashtable_equal_t equal;
  hashtable_hashfunc_t hashfunc;
  size_t node_size;
  size_t node_capacity;
  void* node_data;
  size_t node_head;
  size_t node_tail;
  size_t node_free;
  size_t hash_size;
  size_t* hash;
};

#define NODE(ht, idx) ((node_t *)((ht)->node_data + (idx) * (sizeof (node_t) + (ht)->key_size)))
#define KEY(ht, node) ((void*)((node)->data))

static void
free_insert (hashtable_t* hashtable, size_t idx)
{
  assert (hashtable != NULL);
  assert (idx < hashtable->node_capacity);
  node_t* node = NODE(hashtable, idx);
  node->next = hashtable->node_free;
  hashtable->node_free = idx;
}

static size_t
free_remove (hashtable_t* hashtable)
{
  assert (hashtable != NULL);
  assert (hashtable->node_free != -1);

  size_t retval = hashtable->node_free;
  node_t* node = NODE(hashtable, retval);
  hashtable->node_free = node->next;
  return retval;
}

static void
used_insert (hashtable_t* hashtable, size_t idx, const void* key)
{
  assert (hashtable != NULL);
  assert (idx < hashtable->node_capacity);
  assert (key != NULL);
  
  node_t* node = NODE(hashtable, idx);

  /* Copy the key and data. */
  memcpy (KEY(hashtable, node), key, hashtable->key_size);

  size_t i = hashtable->node_head;

  size_t x;
  
  /* Insert before i. */
  node->next = i;
  
  /* Update i's prev pointer. */
  if (i != -1) {
    node_t* n = NODE(hashtable, i);
    x = n->prev;
    n->prev = idx;
  }
  else {
    x = hashtable->node_tail;
    hashtable->node_tail = idx;
  }

  /* Update our prev pointer. */
  node->prev = x;
  
  if (x != -1) {
    node_t* n = NODE(hashtable, x);
    n->next = idx;
  }
  else {
    hashtable->node_head = idx;
  }
}

static void
used_remove (hashtable_t* hashtable, size_t idx)
{
  assert (hashtable != NULL);
  assert (idx < hashtable->node_capacity);

  node_t* node = NODE(hashtable, idx);

  if (node->prev != -1) {
    node_t* prev = NODE(hashtable, node->prev);
    prev->next = node->next;
  }

  if (node->next != -1) {
    node_t* next = NODE(hashtable, node->next);
    next->prev = node->prev;
  }

  if (idx == hashtable->node_head) {
    hashtable->node_head = node->next;
  }

  if (idx == hashtable->node_tail) {
    hashtable->node_tail = node->prev;
  }

}

hashtable_t*
hashtable_create (size_t key_size, hashtable_equal_t equal, hashtable_hashfunc_t hashfunc)
{
  assert (key_size > 0);
  assert (equal != NULL);
  assert (hashfunc != NULL);
  hashtable_t* hashtable = malloc (sizeof (hashtable_t));
  hashtable->key_size = key_size;
  hashtable->equal = equal;
  hashtable->hashfunc = hashfunc;
  hashtable->node_size = 0;
  hashtable->node_capacity = 1;
  hashtable->node_data = malloc (hashtable->node_capacity * (sizeof (node_t) + hashtable->key_size));
  hashtable->node_head = -1;
  hashtable->node_tail = -1;
  hashtable->node_free = -1;
  size_t idx;
  for (idx = 0; idx < hashtable->node_capacity; ++idx) {
    free_insert (hashtable, idx);
  }
  hashtable->hash_size = 1;
  hashtable->hash = malloc (hashtable->hash_size * sizeof (size_t));
  for (idx = 0; idx < hashtable->hash_size; ++idx) {
    hashtable->hash[idx] = -1;
  }
  return hashtable;
}

void
hashtable_destroy (hashtable_t* hashtable)
{
  assert (hashtable != NULL);
  free (hashtable->node_data);
  free (hashtable->hash);
  free (hashtable);
}

size_t
hashtable_size (hashtable_t* hashtable)
{
  assert (hashtable != NULL);
  return hashtable->node_size;
}

bool
hashtable_empty (hashtable_t* hashtable)
{
  assert (hashtable != NULL);
  return hashtable->node_size == 0;
}

void
hashtable_insert (hashtable_t* hashtable, const void* key)
{
  assert (hashtable != NULL);
  assert (key != NULL);

  /* Resize. */
  if (hashtable->node_size == hashtable->node_capacity) {
    size_t oldcapacity = hashtable->node_capacity;
    hashtable->node_capacity <<= 1;
    hashtable->node_data = realloc (hashtable->node_data, hashtable->node_capacity * (sizeof (node_t) + hashtable->key_size));
    for (; oldcapacity < hashtable->node_capacity; ++oldcapacity) {
      free_insert (hashtable, oldcapacity);
    }
  }

  /* Rehash. */
  if (LOAD_DENOM * hashtable->node_size > LOAD_NUMER * hashtable->hash_size) {
    size_t oldsize = hashtable->hash_size;
    hashtable->hash_size <<= 1;
    hashtable->hash = realloc (hashtable->hash, hashtable->hash_size * sizeof (size_t));
    size_t idx;
    for (idx = oldsize; idx < hashtable->hash_size; ++idx) {
      hashtable->hash[idx] = -1;
    }
    for (idx = 0; idx < oldsize; ++idx) {
      size_t p = hashtable->hash[idx];
      hashtable->hash[idx] = -1;
      while (p != -1) {
	node_t* node = NODE(hashtable, p);
	size_t next_p = node->next_hash;
	size_t hash_idx = node->code & (hashtable->hash_size - 1);
	node->next_hash = hashtable->hash[hash_idx];
	hashtable->hash[hash_idx] = p;
	p = next_p;
      }
    }
  }

  /* Get a node from the free list. */
  size_t idx = free_remove (hashtable);
  /* Insert into the used list. */
  used_insert (hashtable, idx, key);
  ++hashtable->node_size;

  /* Compute the hash code and index into the hash. */
  node_t* node = NODE(hashtable, idx);
  node->code = hashtable->hashfunc (key);
  size_t hash_idx = node->code & (hashtable->hash_size - 1);
  /* Insert into the list. */
  node->next_hash = hashtable->hash[hash_idx];
  hashtable->hash[hash_idx] = idx;
}

static size_t*
lookup (hashtable_t* hashtable, const void* key)
{
  assert (hashtable != NULL);
  assert (key != NULL);

  size_t code = hashtable->hashfunc (key);
  size_t hash_idx = code & (hashtable->hash_size - 1);
  size_t* idx = &hashtable->hash[hash_idx];
  while (*idx != -1) {
    node_t* node = NODE(hashtable, *idx);
    if (node->code == code && hashtable->equal (KEY(hashtable, node), key)) {
      return idx;
    }
    idx = &node->next_hash;
  }

  return NULL;
}

bool
hashtable_contains_key (hashtable_t* hashtable, const void* key)
{
  return lookup (hashtable, key) != NULL;
}

size_t
hashtable_first (hashtable_t* hashtable)
{
  assert (hashtable != NULL);
  return hashtable->node_head;
}

size_t hashtable_next (hashtable_t* hashtable, size_t idx)
{
  assert (hashtable != NULL);
  assert (idx < hashtable->node_capacity);
  node_t* node = NODE(hashtable, idx);
  return node->next;
}

size_t hashtable_last (hashtable_t* hashtable)
{
  assert (hashtable != NULL);
  return -1;
}

void*
hashtable_key_at (hashtable_t* hashtable, size_t idx)
{
  assert (hashtable != NULL);
  assert (idx < hashtable->node_capacity);

  node_t* node = NODE(hashtable, idx);
  return KEY(hashtable, node);
}

void*
hashtable_lookup (hashtable_t* hashtable, const void* key)
{
  assert (hashtable != NULL);
  assert (key != NULL);

  size_t* idx = lookup (hashtable, key);
  if (idx != NULL) {
    node_t* node = NODE(hashtable, *idx);
    return KEY(hashtable, node);
  }
  return NULL;
}

void
hashtable_remove (hashtable_t* hashtable, const void* key)
{
  assert (hashtable != NULL);
  assert (key != NULL);

  size_t* idx = lookup (hashtable, key);
  if (idx != NULL) {
    /* Remove the node from the used list. */
    used_remove (hashtable, *idx);
    /* Insert into the free list. */
    free_insert (hashtable, *idx);
    /* Update the size. */
    --hashtable->node_size;

    /* Remove the node from the hash. */
    node_t* node = NODE(hashtable, *idx);
    *idx = node->next_hash;
  }
}

void
hashtable_clear (hashtable_t* hashtable)
{
  assert (hashtable != NULL);

  hashtable->node_head = -1;
  hashtable->node_tail = -1;
  hashtable->node_free = -1;
  size_t idx;
  for (idx = 0; idx < hashtable->node_capacity; ++idx) {
    free_insert (hashtable, idx);
  }

  hashtable->node_size = 0;

  size_t hash_idx;
  for (hash_idx = 0; hash_idx < hashtable->hash_size; ++hash_idx) {
    hashtable->hash[hash_idx] = -1;
  }
}
