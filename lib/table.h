#ifndef __table_h__
#define __table_h__

#include <stddef.h>
#include <stdbool.h>

typedef struct table_struct table_t;
typedef struct index_struct index_t;
typedef size_t iterator_t;
typedef bool (*predicate_t) (const void*, const void*);

table_t* table_create (size_t);
void table_destroy (table_t*);

void index_destroy (index_t*);
index_t* index_create_list (table_t*);
index_t* index_create_ordered_list (table_t*, predicate_t);

iterator_t index_begin (index_t*);
iterator_t index_advance (index_t*, iterator_t);
iterator_t index_end (index_t*);
iterator_t index_rbegin (index_t*);
iterator_t index_radvance (index_t*, iterator_t);
iterator_t index_rend (index_t*);

bool index_empty (index_t*);
size_t index_size (index_t*);

void* index_front (index_t*);
void* index_back (index_t*);
void* index_value (index_t*, iterator_t);
void* index_at (index_t*, size_t);

/* assign */
void index_push_front (index_t*, const void*);
void index_pop_front (index_t*);
void index_push_back (index_t*, const void*);
void index_pop_back (index_t*);
iterator_t index_insert_before (index_t*, iterator_t, const void*);
iterator_t index_insert (index_t*, const void*);
iterator_t index_erase (index_t*, iterator_t);
/* swap */
void index_clear (index_t*);

/* splice */
/* remove */
/* remove_if */
/* unique */
/* merge */
/* sort */
/* reverse */

/* find */
/* count */
/* lower_bound */
/* upper_bound */
/* equal_range */

typedef void (*function_t) (const void*, void*);
typedef void (*tfunction_t) (void*, void*);

void index_for_each (index_t*, iterator_t, iterator_t, function_t, void*);
iterator_t index_find (index_t*, iterator_t, iterator_t, predicate_t, const void*);
void* index_find_value (index_t*, iterator_t, iterator_t, predicate_t, const void*, iterator_t*);
void index_remove (index_t*, iterator_t, iterator_t, predicate_t, const void*);

void index_transform (index_t*, iterator_t, iterator_t, tfunction_t, void*);

#endif /* __table_h__ */
