#ifndef __table_h__
#define __table_h__

#include <stddef.h>
#include <stdbool.h>

typedef struct table_struct table_t;
typedef struct index_struct index_t;
typedef struct {
  size_t pos;
} iterator_t;
typedef struct {
  size_t pos;
} riterator_t;
typedef bool (*predicate_t) (const void*, void*);

table_t* table_create (size_t);
void table_destroy (table_t*);

void index_destroy (index_t*);
index_t* index_create_list (table_t*);
index_t* index_create_ordered_list (table_t*, predicate_t);

iterator_t index_advance (index_t*, iterator_t);
iterator_t index_retreat (index_t*, iterator_t);

riterator_t index_radvance (index_t*, riterator_t);
riterator_t index_rretreat (index_t*, riterator_t);

iterator_t index_begin (index_t*);
iterator_t index_end (index_t*);
riterator_t index_rbegin (index_t*);
riterator_t index_rend (index_t*);

bool index_empty (index_t*);
size_t index_size (index_t*);

void* index_front (index_t*);
void* index_back (index_t*);
void* index_value (index_t*, iterator_t);
void* index_rvalue (index_t*, riterator_t);
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

bool iterator_eq (iterator_t iter1, iterator_t iter2);
bool iterator_ne (iterator_t iter1, iterator_t iter2);
bool riterator_eq (riterator_t iter1, riterator_t iter2);
bool riterator_ne (riterator_t iter1, riterator_t iter2);

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
iterator_t index_find (index_t*, iterator_t, iterator_t, predicate_t, void*);
riterator_t index_rfind (index_t*, riterator_t, riterator_t, predicate_t, void*);
void* index_find_value (index_t*, iterator_t, iterator_t, predicate_t, void*, iterator_t*);
void* index_rfind_value (index_t*, riterator_t, riterator_t, predicate_t, void*, riterator_t*);
void index_remove (index_t*, iterator_t, iterator_t, predicate_t, void*);

void index_transform (index_t*, iterator_t, iterator_t, tfunction_t, void*);
void index_insert_unique (index_t*, predicate_t, void*);

#endif /* __table_h__ */
