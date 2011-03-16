#include "table.h"

#include <assert.h>
#include <string.h>
#include <stdlib.h>

typedef struct {
  size_t prev;
  size_t next;
} entry_t;

typedef void (*destroy_t) (void*, table_t*);
typedef void (*new_capacity_t) (void*, table_t*);
typedef size_t (*advance_t) (void*, table_t*, size_t);
typedef size_t (*retreat_t) (void*, table_t*, size_t);
typedef size_t (*radvance_t) (void*, table_t*, size_t);
typedef size_t (*rretreat_t) (void*, table_t*, size_t);
typedef size_t (*begin_t) (void*, table_t*);
typedef size_t (*rbegin_t) (void*, table_t*);
typedef void* (*front_t) (void*, table_t*);
typedef void* (*back_t) (void*, table_t*);
typedef void* (*at_t) (void*, table_t*, size_t);
typedef void (*insert_t) (void*, table_t*, size_t);
typedef void (*erase_t) (void*, table_t*, size_t);
typedef void (*clear_t) (void*, table_t*);

struct index_struct {
  index_t* next;
  void* this;
  table_t* table;
  destroy_t destroy;
  new_capacity_t new_capacity;
  advance_t advance;
  retreat_t retreat;
  radvance_t radvance;
  rretreat_t rretreat;
  begin_t begin;
  rbegin_t rbegin;
  front_t front;
  back_t back;
  at_t at;
  insert_t insert;
  erase_t erase;
  clear_t clear;
};

struct table_struct {
  size_t used_head;
  size_t used_tail;
  size_t free_head;
  size_t size;
  size_t capacity;
  entry_t* entries;
  size_t value_size;
  void* values;
  index_t* indices;
};

#define IDX_TO_VALUE(tab, entry) ((tab)->values + (entry) * (tab)->value_size)

static  bool
is_used (table_t* table, size_t idx)
{
  assert (table != NULL);
  assert (idx < table->capacity);

  return
    (table->entries[idx].next != table->entries[idx].prev) ||
    idx == table->used_head ||
    idx == table->used_tail;
}

static void
free_insert (table_t* table, size_t entry)
{
  assert (table != NULL);
  assert (entry < table->capacity);

  table->entries[entry].next = table->free_head;
  table->entries[entry].prev = table->free_head;
  
  table->free_head = entry;
}

static size_t
free_remove (table_t* table)
{
  assert (table != NULL);
  assert (table->free_head != -1);

  size_t entry = table->free_head;
  table->free_head = table->entries[entry].next;
  return entry;
}

static size_t
insert (table_t* table, size_t next_entry, const void* value)
{
  assert (table != NULL);
  assert (value != NULL);

  if (table->size == table->capacity) {
    /* Resize. */
    size_t old_capacity = table->capacity;
    if (table->capacity != 0) {
      table->capacity <<= 1;
    }
    else {
      table->capacity = 1;
    }
    table->entries = realloc (table->entries, table->capacity * sizeof (entry_t));
    table->values = realloc (table->values, table->capacity * table->value_size);
    
    for (; old_capacity < table->capacity; ++old_capacity) {
      free_insert (table, old_capacity);
    }

    /* Tell the indices about it. */
    index_t* index;
    for (index = table->indices; index != NULL; index = index->next) {
      assert (index->new_capacity != NULL);
      index->new_capacity (index->this, index->table);
    }
  }
  
  /* Get a node from the free list and insert into the used list. */
  size_t entry = free_remove (table);
 
  /* Copy the data. */
  memcpy (IDX_TO_VALUE (table, entry), value, table->value_size);
  
  size_t prev_entry;
  
  /* Update entry's next pointer. */
  table->entries[entry].next = next_entry;
  
  /* Update next entry's prev pointer. */
  if (next_entry != -1) {
    prev_entry = table->entries[next_entry].prev;
    table->entries[next_entry].prev = entry;
  }
  else {
    prev_entry = table->used_tail;
    table->used_tail = entry;
  }
  
  /* Update entry's prev pointer. */
  table->entries[entry].prev = prev_entry;
  
  /* Upate the previous entry's next pointer. */
  if (prev_entry != -1) {
    table->entries[prev_entry].next = entry;
  }
  else {
    table->used_head = entry;
  }
  
  ++table->size;

  return entry;
}

static size_t
erase (table_t* table, size_t entry)
{
  assert (table != NULL);
  assert (is_used (table, entry));

  size_t next = table->entries[entry].next;

  if (table->entries[entry].prev != -1) {
    table->entries[table->entries[entry].prev].next = table->entries[entry].next;
  }

  if (table->entries[entry].next != -1) {
    table->entries[table->entries[entry].next].prev = table->entries[entry].prev;
  }

  if (entry == table->used_head) {
    table->used_head = table->entries[entry].next;
  }

  if (entry == table->used_tail) {
    table->used_tail = table->entries[entry].prev;
  }

  free_insert (table, entry);
  --table->size;

  return next;
}

static  void
clear (table_t* table)
{
  assert (table != NULL);

  if (table->size * 2 > table->capacity) {
    /* Over half full.  Reinitialize. */
    table->used_head = -1;
    table->used_tail = -1;
    table->free_head = -1;
    table->size = 0;
    
    size_t idx;
    for (idx = 0; idx < table->capacity; ++idx) {
      free_insert (table, idx);
    }
  }
  else {
    /* Remove individually. */
    while (table->used_head != -1) {
      erase (table, table->used_head);
    }
  }

}

table_t*
table_create (size_t value_size)
{
  assert (value_size > 0);

  table_t* table = malloc (sizeof (table_t));
  table->used_head = -1;
  table->used_tail = -1;
  table->free_head = -1;
  table->size = 0;
  table->capacity = 0;
  table->entries = NULL;
  table->value_size = value_size;
  table->values = NULL;
  table->indices = NULL;

  return table;
}

void
table_destroy (table_t* table)
{
  assert (table != NULL);

  while (table->indices != NULL) {
    index_t* temp = table->indices;
    table->indices = temp->next;
    temp->destroy (temp->this, table);
    free (temp);
  }

  free (table->entries);
  free (table->values);
  free (table);
}

void
index_destroy (index_t* index)
{
  assert (index != NULL);
  assert (index->destroy != NULL);

  index->destroy (index->this, index->table);

  index_t** ptr;
  for (ptr = &index->table->indices;
       *ptr != index;
       ptr = &(*ptr)->next)
    ;;

  *ptr = index->next;
  
  free (index);
}

/**************
 * LIST BEGIN *
 **************/

static void
list_destroy (void* this, table_t* table)
{
  assert (table != NULL);
}

static void
list_new_capacity (void* this, table_t* table)
{
  assert (table != NULL);
}

static size_t
list_advance (void* this, table_t* table, size_t iterator)
{
  assert (table != NULL);
  assert (is_used (table, iterator));

  return table->entries[iterator].next;
}

static size_t
list_retreat (void* this, table_t* table, size_t iterator)
{
  assert (table != NULL);
  assert (is_used (table, iterator));

  return table->entries[iterator].prev;
}

static size_t
list_radvance (void* this, table_t* table, size_t iterator)
{
  assert (table != NULL);
  assert (is_used (table, iterator));

  return table->entries[iterator].prev;
}

static size_t
list_rretreat (void* this, table_t* table, size_t iterator)
{
  assert (table != NULL);
  assert (is_used (table, iterator));

  return table->entries[iterator].next;
}

static size_t
list_begin (void* this, table_t* table)
{
  assert (table != NULL);

  return table->used_head;
}

static size_t
list_rbegin (void* this, table_t* table)
{
  assert (table != NULL);

  return table->used_tail;
}

static void*
list_front (void* this, table_t* table)
{
  assert (table != NULL);
  assert (is_used (table, table->used_head));

  return IDX_TO_VALUE (table, table->used_head);
}

static void*
list_back (void* this, table_t* table)
{
  assert (table != NULL);
  assert (is_used (table, table->used_tail));

  return IDX_TO_VALUE (table, table->used_tail);
}

static void*
list_at (void* this, table_t* table, size_t pos)
{
  assert (table != NULL);

  if (2 * pos < table->size) {
    /* Iterate forward. */
    size_t iterator;
    for (iterator = table->used_head; pos != 0; --pos, iterator = table->entries[iterator].next)
      ;;

    return IDX_TO_VALUE (table, iterator);
  }
  else {
    /* Iterate backward. */
    pos = table->size - (pos + 1);
    size_t iterator;
    for (iterator = table->used_tail; pos != 0; --pos, iterator = table->entries[iterator].prev)
      ;;

    return IDX_TO_VALUE (table, iterator);
  }
}

static void
list_insert (void* this, table_t* table, size_t iterator)
{
  assert (table != NULL);

  /* Do nothing. */
  assert (is_used (table, iterator));
}

static void
list_erase (void* this, table_t* table, size_t iterator)
{
  assert (table != NULL);

  /* Do nothing. */
  assert (!is_used (table, iterator));
}

static void
list_clear (void* this, table_t* table)
{
  assert (table != NULL);

  /* Do nothing. */
  assert (table->used_head == -1);
  assert (table->used_tail == -1);
  assert (table->size == 0);
}

index_t*
index_create_list (table_t* table)
{
  assert (table != NULL);

  index_t* index = malloc (sizeof (index_t));
  index->this = NULL;
  index->table = table;
  index->destroy = list_destroy;
  index->new_capacity = list_new_capacity;
  index->advance = list_advance;
  index->retreat = list_retreat;
  index->radvance = list_radvance;
  index->rretreat = list_rretreat;
  index->begin = list_begin;
  index->rbegin = list_rbegin;
  index->front = list_front;
  index->back = list_back;
  index->at = list_at;
  index->insert = list_insert;
  index->erase = list_erase;
  index->clear = list_clear;

  index->next = table->indices;
  table->indices = index;

  return index;
}

/************
 * LIST END *
 ************/

/**********************
 * ORDERED LIST BEGIN *
 **********************/

typedef struct {
  size_t prev;
  size_t next;
} ordered_list_entry_t;

typedef struct {
  size_t head;
  size_t tail;
  ordered_list_entry_t* entries;
  predicate_t predicate;
} ordered_list_t;

static void
ordered_list_destroy (void* this, table_t* table)
{
  ordered_list_t* ordered_list = this;
  assert (ordered_list != NULL);
  assert (table != NULL);

  free (ordered_list->entries);
}

static void
ordered_list_new_capacity (void* this, table_t* table)
{
  ordered_list_t* ordered_list = this;
  assert (ordered_list != NULL);
  assert (table != NULL);

  ordered_list->entries = realloc (ordered_list->entries, table->capacity * sizeof (ordered_list_entry_t));
}

static size_t
ordered_list_advance (void* this, table_t* table, size_t iterator)
{
  ordered_list_t* ordered_list = this;
  assert (ordered_list != NULL);
  assert (table != NULL);
  assert (is_used (table, iterator));

  return ordered_list->entries[iterator].next;
}

static size_t
ordered_list_retreat (void* this, table_t* table, size_t iterator)
{
  ordered_list_t* ordered_list = this;
  assert (ordered_list != NULL);
  assert (table != NULL);
  assert (is_used (table, iterator));

  return ordered_list->entries[iterator].prev;
}

static size_t
ordered_list_radvance (void* this, table_t* table, size_t iterator)
{
  ordered_list_t* ordered_list = this;
  assert (ordered_list != NULL);
  assert (table != NULL);
  assert (is_used (table, iterator));

  return ordered_list->entries[iterator].prev;
}

static size_t
ordered_list_rretreat (void* this, table_t* table, size_t iterator)
{
  ordered_list_t* ordered_list = this;
  assert (ordered_list != NULL);
  assert (table != NULL);
  assert (is_used (table, iterator));

  return ordered_list->entries[iterator].next;
}

static size_t
ordered_list_begin (void* this, table_t* table)
{
  ordered_list_t* ordered_list = this;
  assert (ordered_list != NULL);
  assert (table != NULL);

  return ordered_list->head;
}

static size_t
ordered_list_rbegin (void* this, table_t* table)
{
  ordered_list_t* ordered_list = this;
  assert (ordered_list != NULL);
  assert (table != NULL);

  return ordered_list->tail;
}

static void*
ordered_list_front (void* this, table_t* table)
{
  ordered_list_t* ordered_list = this;
  assert (ordered_list != NULL);
  assert (table != NULL);
  assert (is_used (table, ordered_list->head));

  return IDX_TO_VALUE (table, ordered_list->head);
}

static void*
ordered_list_back (void* this, table_t* table)
{
  ordered_list_t* ordered_list = this;
  assert (ordered_list != NULL);
  assert (table != NULL);
  assert (is_used (table, ordered_list->tail));

  return IDX_TO_VALUE (table, ordered_list->tail);
}

static void*
ordered_list_at (void* this, table_t* table, size_t pos)
{
  ordered_list_t* ordered_list = this;
  assert (ordered_list != NULL);
  assert (table != NULL);

  if (2 * pos < table->size) {
    /* Iterate forward. */
    size_t iterator;
    for (iterator = ordered_list->head; pos != 0; --pos, iterator = ordered_list->entries[iterator].next)
      ;;

    return IDX_TO_VALUE (table, iterator);
  }
  else {
    /* Iterate backward. */
    pos = table->size - (pos + 1);
    size_t iterator;
    for (iterator = ordered_list->tail; pos != 0; --pos, iterator = ordered_list->entries[iterator].prev)
      ;;

    return IDX_TO_VALUE (table, iterator);
  }
}

static void
ordered_list_insert (void* this, table_t* table, size_t entry)
{
  ordered_list_t* ordered_list = this;
  assert (ordered_list != NULL);
  assert (table != NULL);
  assert (is_used (table, entry));

  size_t next_entry;
  for (next_entry = ordered_list->head;
       next_entry != -1 &&
   	 !ordered_list->predicate (IDX_TO_VALUE (table, entry), IDX_TO_VALUE (table, next_entry));
       next_entry = ordered_list->entries[next_entry].next)
    ;;
  
  size_t prev_entry;
  
  /* Update entry's next pointer. */
  ordered_list->entries[entry].next = next_entry;
  
  /* Update next entry's prev pointer. */
  if (next_entry != -1) {
    prev_entry = ordered_list->entries[next_entry].prev;
    ordered_list->entries[next_entry].prev = entry;
  }
  else {
    prev_entry = ordered_list->tail;
    ordered_list->tail = entry;
  }
  
  /* Update entry's prev pointer. */
  ordered_list->entries[entry].prev = prev_entry;
  
  /* Upate the previous entry's next pointer. */
  if (prev_entry != -1) {
    ordered_list->entries[prev_entry].next = entry;
  }
  else {
    ordered_list->head = entry;
  }
}

static void
ordered_list_erase (void* this, table_t* table, size_t entry)
{
  ordered_list_t* ordered_list = this;
  assert (ordered_list != NULL);
  assert (table != NULL);

  if (ordered_list->entries[entry].prev != -1) {
    ordered_list->entries[ordered_list->entries[entry].prev].next = ordered_list->entries[entry].next;
  }
  
  if (ordered_list->entries[entry].next != -1) {
    ordered_list->entries[ordered_list->entries[entry].next].prev = ordered_list->entries[entry].prev;
  }
  
  if (entry == ordered_list->head) {
    ordered_list->head = ordered_list->entries[entry].next;
  }
  
  if (entry == ordered_list->tail) {
    ordered_list->tail = ordered_list->entries[entry].prev;
  }
}

static void
ordered_list_clear (void* this, table_t* table)
{
  ordered_list_t* ordered_list = this;
  assert (ordered_list != NULL);
  assert (table != NULL);

  ordered_list->head = -1;
  ordered_list->tail = -1;
}

index_t*
index_create_ordered_list (table_t* table, predicate_t predicate)
{
  assert (table != NULL);
  assert (predicate != NULL);

  ordered_list_t* ordered_list = malloc (sizeof (ordered_list_t));
  ordered_list->head = -1;
  ordered_list->tail = -1;
  ordered_list->entries = NULL;
  ordered_list->predicate = predicate;

  index_t* index = malloc (sizeof (index_t));
  index->this = ordered_list;
  index->table = table;
  index->destroy = ordered_list_destroy;
  index->new_capacity = ordered_list_new_capacity;
  index->advance = ordered_list_advance;
  index->retreat = ordered_list_retreat;
  index->radvance = ordered_list_radvance;
  index->rretreat = ordered_list_rretreat;
  index->begin = ordered_list_begin;
  index->rbegin = ordered_list_rbegin;
  index->front = ordered_list_front;
  index->back = ordered_list_back;
  index->at = ordered_list_at;
  index->insert = ordered_list_insert;
  index->erase = ordered_list_erase;
  index->clear = ordered_list_clear;

  index->next = table->indices;
  table->indices = index;

  return index;
}

/********************
 * ORDERED LIST END *
 ********************/

iterator_t
index_advance (index_t* index, iterator_t iterator)
{
  assert (index != NULL);
  assert (index->advance != NULL);

  iterator.pos = index->advance (index->this, index->table, iterator.pos);
  return iterator;
}

iterator_t
index_retreat (index_t* index, iterator_t iterator)
{
  assert (index != NULL);
  assert (index->retreat != NULL);

  iterator.pos = index->retreat (index->this, index->table, iterator.pos);
  return iterator;
}

riterator_t
index_radvance (index_t* index, riterator_t iterator)
{
  assert (index != NULL);
  assert (index->advance != NULL);

  iterator.pos = index->radvance (index->this, index->table, iterator.pos);
  return iterator;
}

riterator_t
index_rretreat (index_t* index, riterator_t iterator)
{
  assert (index != NULL);
  assert (index->retreat != NULL);

  iterator.pos = index->rretreat (index->this, index->table, iterator.pos);
  return iterator;
}

iterator_t
index_begin (index_t* index)
{
  assert (index != NULL);
  assert (index->begin != NULL);

  iterator_t iterator = {
    .pos = index->begin (index->this, index->table),
  };

  return iterator;
}

iterator_t
index_end (index_t* index)
{
  assert (index != NULL);

  iterator_t iterator = {
    .pos = -1,
  };

  return iterator;
}

riterator_t
index_rbegin (index_t* index)
{
  assert (index != NULL);
  assert (index->rbegin != NULL);

  riterator_t iterator = {
    .pos = index->rbegin (index->this, index->table),
  };

  return iterator;
}

riterator_t
index_rend (index_t* index)
{
  assert (index != NULL);

  riterator_t iterator = {
    .pos = -1,
  };

  return iterator;
}

bool
index_empty (index_t* index)
{
  assert (index != NULL);

  return index->table->size == 0;
}

size_t
index_size (index_t* index)
{
  assert (index != NULL);

  return index->table->size;
}

void*
index_front (index_t* index)
{
  assert (index != NULL);
  assert (index->front != NULL);

  return index->front (index->this, index->table);
}

void*
index_back (index_t* index)
{
  assert (index != NULL);
  assert (index->back != NULL);

  return index->back (index->this, index->table);
}

void*
index_value (index_t* index, iterator_t iterator)
{
  assert (index != NULL);
  assert (is_used (index->table, iterator.pos));

  return IDX_TO_VALUE (index->table, iterator.pos);
}

void*
index_rvalue (index_t* index, riterator_t iterator)
{
  assert (index != NULL);
  assert (is_used (index->table, iterator.pos));

  return IDX_TO_VALUE (index->table, iterator.pos);
}

void*
index_at (index_t* index, size_t pos)
{
  assert (index != NULL);
  assert (index->at != NULL);
  assert (pos < index->table->size);

  return index->at (index->this, index->table, pos);
}

void
index_push_front (index_t* index, const void* value)
{
  assert (index != NULL);
  assert (value != NULL);

  /* Insert the value into the table. */
  size_t iterator = insert (index->table, index->table->used_head, value);

  /* Tell the indices about it. */
  for (index = index->table->indices; index != NULL; index = index->next) {
    assert (index->insert != NULL);
    index->insert (index->this, index->table, iterator);
  }
}

void
index_pop_front (index_t* index)
{
  assert (index != NULL);

  /* Remove the value from the table. */
  iterator_t iterator = index_begin (index);
  erase (index->table, iterator.pos);

  /* Tell the indices about it. */
  for (index = index->table->indices; index != NULL; index = index->next) {
    assert (index->erase != NULL);
    index->erase (index->this, index->table, iterator.pos);
  }
}

void
index_push_back (index_t* index, const void* value)
{
  assert (index != NULL);
  assert (value != NULL);

  /* Insert the value into the table. */
  size_t iterator = insert (index->table, -1, value);

  /* Tell the indices about it. */
  for (index = index->table->indices; index != NULL; index = index->next) {
    assert (index->insert != NULL);
    index->insert (index->this, index->table, iterator);
  }
}

void
index_pop_back (index_t* index)
{
  assert (index != NULL);

  /* Remove the value from the table. */
  riterator_t iterator = index_rbegin (index);
  erase (index->table, iterator.pos);

  /* Tell the indices about it. */
  for (index = index->table->indices; index != NULL; index = index->next) {
    assert (index->erase != NULL);
    index->erase (index->this, index->table, iterator.pos);
  }
}

iterator_t
index_insert_before (index_t* index, iterator_t iterator, const void* value)
{
  assert (index != NULL);
  assert (iterator.pos == -1 || is_used (index->table, iterator.pos));
  assert (value != NULL);

  /* Insert the value into the table. */
  iterator.pos = insert (index->table, iterator.pos, value);

  /* Tell the indices about it. */
  for (index = index->table->indices; index != NULL; index = index->next) {
    assert (index->insert != NULL);
    index->insert (index->this, index->table, iterator.pos);
  }

  return iterator;
}

iterator_t
index_insert (index_t* index, const void* value)
{
  assert (index != NULL);
  assert (value != NULL);

  /* Currently equivalent to push back. */

  /* Insert the value into the table. */
  iterator_t iterator = {
    .pos = insert (index->table, -1, value),
  };

  /* Tell the indices about it. */
  for (index = index->table->indices; index != NULL; index = index->next) {
    assert (index->insert != NULL);
    index->insert (index->this, index->table, iterator.pos);
  }
  
  return iterator;
}

iterator_t
index_erase (index_t* index, iterator_t iterator)
{
  assert (index != NULL);
  assert (is_used (index->table, iterator.pos));

  /* Erase the value. */
  iterator_t iter = {
    .pos = erase (index->table, iterator.pos),
  };

  /* Tell the indices about it. */
  for (index = index->table->indices; index != NULL; index = index->next) {
    assert (index->erase != NULL);
    index->erase (index->this, index->table, iterator.pos);
  }

  return iter;
}

void
index_clear (index_t* index)
{
  assert (index != NULL);

  /* Clear the table. */
  clear (index->table);

  /* Tell the indices about it. */
  for (index = index->table->indices; index != NULL; index = index->next) {
    assert (index->clear != NULL);
    index->clear (index->this, index->table);
  }

}

bool
iterator_eq (iterator_t iter1, iterator_t iter2)
{
  return iter1.pos == iter2.pos;
}

bool
iterator_ne (iterator_t iter1, iterator_t iter2)
{
  return iter1.pos != iter2.pos;
}

bool
riterator_eq (riterator_t iter1, riterator_t iter2)
{
  return iter1.pos == iter2.pos;
}

bool
riterator_ne (riterator_t iter1, riterator_t iter2)
{
  return iter1.pos != iter2.pos;
}

iterator_t
riterator_reverse (index_t* index, riterator_t riterator)
{
  assert (index != NULL);

  if (riterator.pos != -1) {
    assert (is_used (index->table, riterator.pos));
    iterator_t iterator = {
      .pos = riterator.pos,
    };
    return iterator;
  }
  else {
    return index_begin (index);
  }
}


void
index_for_each (index_t* index, iterator_t begin, iterator_t end, function_t function, void* arg)
{
  assert (index != NULL);
  assert (function != NULL);

  for (;
       iterator_ne (begin, end);
       begin = index_advance (index, begin)) {
    void* v = index_value (index, begin);
    function (v, arg);
  }

}

iterator_t
index_find (index_t* index, iterator_t begin, iterator_t end, predicate_t predicate, const void* value)
{
  assert (index != NULL);
  assert (predicate != NULL);

  for (;
       iterator_ne (begin, end);
       begin = index_advance (index, begin)) {
    void* v = index_value (index, begin);
    if (predicate (v, value)) {
      break;
    }
  }

  return begin;
}

riterator_t
index_rfind (index_t* index, riterator_t begin, riterator_t end, predicate_t predicate, void* value)
{
  assert (index != NULL);
  assert (predicate != NULL);

  for (;
       riterator_ne (begin, end);
       begin = index_radvance (index, begin)) {
    void* v = index_rvalue (index, begin);
    if (predicate (v, value)) {
      break;
    }
  }

  return begin;
}

void*
index_find_value (index_t* index, iterator_t begin, iterator_t end, predicate_t predicate, const void* arg, iterator_t* ptr)
{
  assert (index != NULL);

  iterator_t iterator = index_find (index,
				    begin,
				    end,
				    predicate,
				    arg);
  if (ptr != NULL) {
    *ptr = iterator;
  }
  if (iterator_ne (iterator, end)) {
    return index_value (index, iterator);
  }
  else {
    return NULL;
  }
}

void*
index_rfind_value (index_t* index, riterator_t rbegin, riterator_t rend, predicate_t predicate, void* arg, riterator_t* ptr)
{
  assert (index != NULL);

  riterator_t iterator = index_rfind (index,
				      rbegin,
				      rend,
				      predicate,
				      arg);
  if (ptr != NULL) {
    *ptr = iterator;
  }
  if (riterator_ne (iterator, rend)) {
    return index_rvalue (index, iterator);
  }
  else {
    return NULL;
  }
}

void
index_remove (index_t* index, iterator_t begin, iterator_t end, predicate_t predicate, void* value)
{
  assert (index != NULL);
  assert (predicate != NULL);

  while (iterator_ne (begin, end)) {
    void* v = index_value (index, begin);
    if (predicate (v, value)) {
      begin = index_erase (index, begin);
    }
    else {
      begin = index_advance (index, begin);
    }
  }
}

void
index_transform (index_t* index, iterator_t begin, iterator_t end, tfunction_t function, void* arg)
{
  assert (index != NULL);
  assert (function != NULL);

  for (;
       iterator_ne (begin, end);
       begin = index_advance (index, begin)) {
    void* v = index_value (index, begin);
    function (v, arg);
  }

}

void
index_insert_unique (index_t* index, predicate_t predicate, void* value)
{
  assert (index != NULL);
  assert (predicate != NULL);

  iterator_t iterator = index_find (index,
				    index_begin (index),
				    index_end (index),
				    predicate,
				    value);
  if (iterator_eq (iterator, index_end (index))) {
    index_insert (index, value);
  }
}
