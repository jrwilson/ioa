#ifndef __bitset_h__
#define __bitset_h__

#include <stddef.h>
#include <stdbool.h>

typedef struct bitset_struct bitset_t;

bitset_t* bitset_create (size_t);
void bitset_set (bitset_t*, size_t);
bool bitset_test (bitset_t*, size_t);
void bitset_clear (bitset_t*, size_t);
size_t bitset_size (bitset_t*);
bool bitset_empty (bitset_t*);
bool bitset_full (bitset_t*);
size_t bitset_capacity (bitset_t*);
size_t bitset_next_set (bitset_t*, size_t);
size_t bitset_next_clear (bitset_t*, size_t);
void bitset_set_all (bitset_t*);

#endif /* __bitset_h__ */
