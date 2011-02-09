#ifndef __hashtable_h__
#define __hashtable_h__

#include <stddef.h>
#include <stdbool.h>

typedef struct hashtable_struct hashtable_t;
typedef bool (*hashtable_equal_t) (const void*, const void*);
typedef bool (*hashtable_sorted_t) (const void*, const void*);

hashtable_t* hashtable_create (size_t, size_t, hashtable_equal_t, hashtable_sorted_t);
void hashtable_destroy (hashtable_t*);

size_t hashtable_size (hashtable_t*);
bool hashtable_empty (hashtable_t*);

void hashtable_insert (hashtable_t*, void*, void*);
bool hashtable_contains_key (hashtable_t*, void*);
size_t hashtable_first (hashtable_t*);
size_t hashtable_next (hashtable_t*, size_t);
size_t hashtable_last (hashtable_t*);
const void* hashtable_key_at (hashtable_t*, size_t);
void* hashtable_value_at (hashtable_t*, size_t);

void* hashtable_value (hashtable_t*, void*);
void hashtable_remove (hashtable_t*, void*);
void hashtable_clear (hashtable_t*);

#endif /* __hashtable_h__ */