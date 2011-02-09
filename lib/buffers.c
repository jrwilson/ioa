#include "buffers.h"

#include <pthread.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include "table.h"

struct buffers_struct {
  pthread_rwlock_t lock;
  bid_t next_bid;  
  table_t* buffer_table;
  index_t* buffer_index;
  table_t* buffer_ref_table;
  index_t* buffer_ref_index;
  table_t* buffer_edge_table;
  index_t* buffer_edge_index;
  table_t* reachable_open_table;
  index_t* reachable_open_index;
  table_t* ref_closed_table;
  index_t* ref_closed_index;
  table_t* parent_reach_table;
  index_t* parent_reach_index;
  table_t* child_reach_table;
  index_t* child_reach_index;
};

/*
  Buffers
  =======

  A buffer is a contiguous region of storage used to pass values from an output action to all composed inputs actions.

  The typical usage of a buffer is as follows:
  1.  The buffer is created by an output action, written with values, and returned.
  2.  The system passes the buffer as a argument to the inputs actions composed with the output action.
  3.  The input actions process the content of the buffer.

  Reference Counting
  ------------------

  Buffers are reference counted to reduce copying.
  Reference counting facilitates the common case when an input action queues a buffer for subsequent processing.

  The buffer lifecycle is as follows:
  1.  The buffer is created.
  2.  The creator acquires a pointer to the storage backing the buffer and initializes it.
  3.  Any automaton (including the creator) can reference the buffer and acquire a pointer to the storage backing the buffer.
      Referencing the buffer converts it from read-write to read-only.
  4.  An automaton can decrement the reference count of a buffer.
      When the reference count is zero, the buffer is destroyed.

  Between the time a buffer is returned from an output action and fed to composed input actions, the system increments the reference count of a buffer.
  Thus, the buffer is read-only for all input actions.
  After all of the input actions have fired, the system decrements the reference count.
  If the buffer has not been referenced, then the buffer will be destroyed as its reference count will be zero.

  Buffer Hiearchies
  -----------------

  Buffers can contain references to other buffers.
  Buffer references are useful for scatter/gather I/O, bucket brigades, etc.
  Users must inform the system of this hiearchy by specifying the appropriate relationships.
  The hiearchy is limited to a directed acyclic graph (DAG).

  Consider a buffer (parent) that contains a reference to other buffers (children).
  The transitive closure of this parent-child relationship yields all buffers reachable from the given parent.
  The invariant maintained by the system is that every reachable buffer from a given parent buffer contains all of references of the parent buffer.
  (The child may contain more references from other parents.)
  Incrementing/decrementing the reference count increments/decrements the reference count for all reachable children.

  Duplicating Buffers
  -------------------

  Buffers can be duplicated.
  Duplicate buffers have the same semantics as new buffers except they have the same children as the original.
  The system can elect to reuse the original if the original buffer has no references.

 */

typedef struct {
  bid_t bid;
  aid_t owner;
  size_t size;
  void* data;
  size_t ref_count;
} buffer_entry_t;

static bool
buffer_entry_bid_equal (const void* x0, const void* y0)
{
  const buffer_entry_t* x = x0;
  const buffer_entry_t* y = y0;
  return x->bid == y->bid;
}

static buffer_entry_t*
buffer_entry_for_bid (buffers_t* buffers, bid_t bid, iterator_t* ptr)
{
  assert (buffers != NULL);

  buffer_entry_t key = {
    .bid = bid
  };

  return index_find_value (buffers->buffer_index,
			   index_begin (buffers->buffer_index),
			   index_end (buffers->buffer_index),
			   buffer_entry_bid_equal,
			   &key,
			   ptr);
}

typedef struct {
  aid_t aid;
  bid_t bid;
  size_t count;
} buffer_ref_entry_t;

static bool
buffer_ref_entry_aid_bid_equal (const void* x0, const void* y0)
{
  const buffer_ref_entry_t* x = x0;
  const buffer_ref_entry_t* y = y0;
  return x->aid == y->aid && x->bid == y->bid;
}

static bool
buffer_ref_entry_aid_equal (const void* x0, const void* y0)
{
  const buffer_ref_entry_t* x = x0;
  const buffer_ref_entry_t* y = y0;
  return x->aid == y->aid;
}

static buffer_ref_entry_t*
buffer_ref_entry_for_aid_bid (buffers_t* buffers, aid_t aid, bid_t bid, iterator_t* ptr)
{
  assert (buffers != NULL);

  buffer_ref_entry_t key = {
    .bid = bid,
    .aid = aid
  };

  return index_find_value (buffers->buffer_ref_index,
			   index_begin (buffers->buffer_ref_index),
			   index_end (buffers->buffer_ref_index),
			   buffer_ref_entry_aid_bid_equal, 
			   &key,
			   ptr);
}

typedef struct {
  bid_t parent;
  bid_t child;
} buffer_edge_entry_t;

static bool
buffer_edge_entry_parent_or_child_equal (const void* x0, const void* y0)
{
  const buffer_edge_entry_t* x = x0;
  const buffer_edge_entry_t* y = y0;
  return
    x->parent == y->parent ||
    x->child == y->parent;
}

static bool
buffer_edge_entry_parent_child_equal (const void* x0, const void* y0)
{
  const buffer_edge_entry_t* x = x0;
  const buffer_edge_entry_t* y = y0;
  return
    x->parent == y->parent &&
    x->child == y->child;
}

static buffer_edge_entry_t*
buffer_edge_entry_for_parent_child (buffers_t* buffers, bid_t parent, bid_t child, iterator_t* ptr)
{
  assert (buffers != NULL);

  buffer_edge_entry_t key = {
    .parent = parent,
    .child = child,
  };

  return index_find_value (buffers->buffer_edge_index,
			   index_begin (buffers->buffer_edge_index),
			   index_end (buffers->buffer_edge_index),
			   buffer_edge_entry_parent_child_equal,
			   &key,
			   ptr);
}

static bool
bid_equal (const void* x0, const void* y0)
{
  const bid_t* x = x0;
  const bid_t* y = y0;
  return *x == *y;
}

static void
free_data (const void* e, void* ignored)
{
  const buffer_entry_t* buffer_entry = e;
  free (buffer_entry->data);
}

static buffer_entry_t*
allocate (buffers_t* buffers, aid_t owner, size_t size)
{
  /* Find a bid. */
  while (buffer_entry_for_bid (buffers, buffers->next_bid, NULL) != NULL) {
    ++buffers->next_bid;
    if (buffers->next_bid < 0) {
      buffers->next_bid = 0;
    }
  }
  bid_t bid = buffers->next_bid;

  /* Insert. */
  buffer_entry_t entry = { 
    .bid = bid,
    .owner = owner,
    .size = size,
    .data = malloc (size),
    .ref_count = 0 };
  return index_value (buffers->buffer_index, index_insert (buffers->buffer_index, &entry));
}

bid_t
buffers_alloc (buffers_t* buffers, aid_t owner, size_t size)
{
  assert (buffers != NULL);

  /*
   * NO CHECK IF AID EXISTS.
   */

  buffer_entry_t* buffer_entry;
  pthread_rwlock_wrlock (&buffers->lock);
  buffer_entry = allocate (buffers, owner, size);
  pthread_rwlock_unlock (&buffers->lock);

  return buffer_entry->bid;
}

void*
buffers_write_ptr (buffers_t* buffers, aid_t aid, bid_t bid)
{
  assert (buffers != NULL);

  void* ptr = NULL;

  pthread_rwlock_rdlock (&buffers->lock);
  /* Lookup. */
  buffer_entry_t* entry = buffer_entry_for_bid (buffers, bid, NULL);
  /* Only for the owner and only if not referenced. */
  if (entry != NULL && entry->owner == aid && entry->ref_count == 0) {
    ptr = entry->data;
  }
  pthread_rwlock_unlock (&buffers->lock);

  return ptr;
}

const void*
buffers_read_ptr (buffers_t* buffers, aid_t aid, bid_t bid)
{
  assert (buffers != NULL);


  void* ptr = NULL;
  pthread_rwlock_rdlock (&buffers->lock);
  /* Lookup. */
  buffer_entry_t* buffer_entry = buffer_entry_for_bid (buffers, bid, NULL);
  buffer_ref_entry_t* buffer_ref_entry = buffer_ref_entry_for_aid_bid (buffers, aid, bid, NULL);
  /* Only for the owner or if they have a reference. */
  if ((buffer_entry != NULL && buffer_entry->owner == aid && buffer_entry->ref_count != 0) ||
      (buffer_ref_entry != NULL)) {
    ptr = buffer_entry->data;
  }
  pthread_rwlock_unlock (&buffers->lock);

  return ptr;
}

size_t
buffers_size (buffers_t* buffers, aid_t aid, bid_t bid)
{
  assert (buffers != NULL);

  size_t size = 0;
  pthread_rwlock_rdlock (&buffers->lock);
  /* Lookup. */
  buffer_entry_t* buffer_entry = buffer_entry_for_bid (buffers, bid, NULL);
  buffer_ref_entry_t* buffer_ref_entry = buffer_ref_entry_for_aid_bid (buffers, aid, bid, NULL);
  /* Owner or have a reference. */
  if ((buffer_entry != NULL && buffer_entry->owner == aid) ||
      (buffer_ref_entry != NULL)) {
    size = buffer_entry->size;
  }
  pthread_rwlock_unlock (&buffers->lock);

  return size;
}

size_t
buffers_ref_count (buffers_t* buffers, aid_t aid, bid_t bid)
{
  assert (buffers != NULL);

  /* Lookup. */
  buffer_entry_t* buffer_entry = buffer_entry_for_bid (buffers, bid, NULL);
  buffer_ref_entry_t* buffer_ref_entry = buffer_ref_entry_for_aid_bid (buffers, aid, bid, NULL);
  /* Owner or have a reference. */
  if ((buffer_entry != NULL && buffer_entry->owner == aid) ||
      (buffer_ref_entry != NULL)) {
    return buffer_ref_entry->count;
  }
  else {
    return 0;
  }
}

bool
buffers_exists (buffers_t* buffers, bid_t bid)
{
  assert (buffers != NULL);

  /* Lookup. */
  return buffer_entry_for_bid (buffers, bid, NULL) != NULL;
}

void
buffers_change_owner (buffers_t* buffers, aid_t aid, bid_t bid)
{
  assert (buffers != NULL);

  /*
   * NO CHECK IF AID EXISTS.
   */
  pthread_rwlock_wrlock (&buffers->lock);
  buffer_entry_t* buffer_entry = buffer_entry_for_bid (buffers, bid, NULL);
  if (buffer_entry != NULL) {
    buffer_entry->owner = aid;
  }
  pthread_rwlock_unlock (&buffers->lock);
}

typedef struct {
  buffers_t* buffers;
  bid_t parent;
} insert_arg_t;

static void
insert_child_into_open (const void* value, void* a)
{
  const buffer_edge_entry_t* entry = value;
  insert_arg_t* arg = a;

  if (entry->parent == arg->parent) {
    /* Insert into the open list. */
    index_insert (arg->buffers->reachable_open_index, &entry->child);
  }
}

static void
find_reachable_buffers (buffers_t* buffers, bid_t root_bid, index_t* target_index)
{
  /* We have an open list of buffers and we have a closed list of buffers. */
  index_clear (buffers->reachable_open_index);
  index_clear (target_index);
  
  /* Seed the open list. */
  index_insert (buffers->reachable_open_index, &root_bid);
  
  while (!index_empty (buffers->reachable_open_index)) {
    /* Pop from open. */
    iterator_t idx = index_begin (buffers->reachable_open_index);
    bid_t bid = *(bid_t*)index_value (buffers->reachable_open_index, idx);
    index_erase (buffers->reachable_open_index, idx);
    
    if (index_find_value (target_index,
			  index_begin (target_index),
			  index_end (target_index),
			  bid_equal,
			  &bid,
			  NULL) == NULL) {
      /* Not in closed list. */
      
      /* Push to closed. */
      index_insert (target_index, &bid);

      /* Insert all children into open. */
      insert_arg_t arg = {
	.buffers = buffers,
	.parent = bid
      };
      index_for_each (buffers->buffer_edge_index,
		      index_begin (buffers->buffer_edge_index),
		      index_end (buffers->buffer_edge_index),
		      insert_child_into_open,
		      &arg);
    }
  }
}

static void
incref (buffers_t* buffers, bid_t bid, aid_t aid, size_t count)
{
  buffer_ref_entry_t* buffer_ref_entry = buffer_ref_entry_for_aid_bid (buffers, aid, bid, NULL);
  if (buffer_ref_entry != NULL) {
    /* Increment reference count. */
    buffer_ref_entry->count += count;
  }
  else {
    /* Create new entry. */
    buffer_ref_entry_t key = {
      .bid = bid,
      .aid = aid,
      .count = count,
    };
    index_insert (buffers->buffer_ref_index, &key);
  }

  /* Increment the global reference count. */
  buffer_entry_t* buffer_entry = buffer_entry_for_bid (buffers, bid, NULL);
  assert (buffer_entry != NULL);
  buffer_entry->ref_count += count;
}

typedef struct {
  buffers_t* buffers;
  aid_t aid;
  size_t count;
} incref_arg_t;

static void
incref_bid (const void* value, void* a)
{
  bid_t bid = *(bid_t*)value;
  incref_arg_t* arg = a;

  incref (arg->buffers, bid, arg->aid, arg->count);
}

void
buffers_incref (buffers_t* buffers, aid_t aid, bid_t bid)
{
  assert (buffers != NULL);

  /*
   * NO CHECK IF AID EXISTS.
   */

  pthread_rwlock_wrlock (&buffers->lock);

  buffer_entry_t* buffer_entry = buffer_entry_for_bid (buffers, bid, NULL);
  buffer_ref_entry_t* buffer_ref_entry = buffer_ref_entry_for_aid_bid (buffers, aid, bid, NULL);
  /* Must be owner or already have a reference. */
  if ((buffer_entry != NULL && buffer_entry->owner == aid) ||
      (buffer_ref_entry != NULL)) {
    
    /* Find all reachable children. */
    find_reachable_buffers (buffers, bid, buffers->ref_closed_index);   
    
    /* Increment the reference count for all reachable. */
    incref_arg_t arg = {
      .buffers = buffers,
      .aid = aid,
      .count = 1
    };
    index_for_each (buffers->ref_closed_index,
		    index_begin (buffers->ref_closed_index),
		    index_end (buffers->ref_closed_index),
		    incref_bid,
		    &arg);
  }

  pthread_rwlock_unlock (&buffers->lock);
}

static void
remove_buffer_entry (buffers_t* buffers, buffer_entry_t* buffer_entry, iterator_t buffer_entry_idx)
{
  assert (buffer_entry->ref_count == 0);

  /* Remove parent-child relationships. */
  buffer_edge_entry_t key = {
    .parent = buffer_entry->bid
  };

  index_remove (buffers->buffer_edge_index,
		index_begin (buffers->buffer_edge_index),
		index_end (buffers->buffer_edge_index),
		buffer_edge_entry_parent_or_child_equal,
		&key);
  
  /* Free the data. */
  free (buffer_entry->data);
  
  /* Remove. */
  index_erase (buffers->buffer_index, buffer_entry_idx);
}

static void
decref (buffers_t* buffers, bid_t bid, aid_t aid, size_t count)
{
  iterator_t buffer_ref_entry_idx;
  buffer_ref_entry_t* buffer_ref_entry = buffer_ref_entry_for_aid_bid (buffers, aid, bid, &buffer_ref_entry_idx);
  
  /* Decrement reference count. */
  buffer_ref_entry->count -= count;
  if (buffer_ref_entry->count == 0) {
    /* Remove. */
    index_erase (buffers->buffer_ref_index, buffer_ref_entry_idx);
  }
  
  /* Decrement the global reference count. */
  iterator_t buffer_entry_idx;
  buffer_entry_t* buffer_entry = buffer_entry_for_bid (buffers, bid, &buffer_entry_idx);
  assert (buffer_entry != NULL);
  buffer_entry->ref_count -= count;

  if (buffer_entry->ref_count == 0) {
    remove_buffer_entry (buffers, buffer_entry, buffer_entry_idx);
  }
}

static void
decref_bid (const void* value, void* a)
{
  bid_t bid = *(bid_t*)value;
  incref_arg_t* arg = a;

  decref (arg->buffers, bid, arg->aid, arg->count);
}

void
buffers_decref (buffers_t* buffers, aid_t aid, bid_t root_bid)
{
  assert (buffers != NULL);

  /*
   * NO CHECK IF AID EXISTS.
   */

  pthread_rwlock_wrlock (&buffers->lock);

  buffer_ref_entry_t* root_buffer_ref_entry = buffer_ref_entry_for_aid_bid (buffers, aid, root_bid, NULL);
  /* Must have a reference. */
  if (root_buffer_ref_entry != NULL) {

    /* Find all reachable children. */
    find_reachable_buffers (buffers, root_bid, buffers->ref_closed_index);   

    /* Decrement the reference count for all reachable. */
    incref_arg_t arg = {
      .buffers = buffers,
      .aid = aid,
      .count = 1
    };
    index_for_each (buffers->ref_closed_index,
		    index_begin (buffers->ref_closed_index),
		    index_end (buffers->ref_closed_index),
		    decref_bid,
		    &arg);

  }

  pthread_rwlock_unlock (&buffers->lock);
}

static bool
in_parent (const void* value, const void* arg)
{
  bid_t bid = *(bid_t*)value;
  const buffers_t* buffers = arg;

  return index_find_value (buffers->parent_reach_index,
  			   index_begin (buffers->parent_reach_index),
  			   index_end (buffers->parent_reach_index),
  			   bid_equal,
  			   &bid,
  			   NULL) != NULL;
}

typedef enum {
  TRANSFER,
  UNTRANSFER
} transfer_mode_t;

typedef struct {
  transfer_mode_t mode;
  buffers_t* buffers;
  bid_t parent;
  aid_t aid;
  size_t count;
} transfer_arg_t;

static void
transfer2 (const void* value, void* a)
{
  bid_t bid = *(bid_t*)value;
  transfer_arg_t* arg = a;

  switch (arg->mode) {
  case TRANSFER:
    incref (arg->buffers, bid, arg->aid, arg->count);
    break;
  case UNTRANSFER:
    decref (arg->buffers, bid, arg->aid, arg->count);
    break;
  }

}

static void
transfer1 (const void* value, void* a)
{
  const buffer_ref_entry_t* buffer_ref_entry = value;
  transfer_arg_t* arg = a;

  if (buffer_ref_entry->bid == arg->parent) {
    arg->aid = buffer_ref_entry->aid;
    arg->count = buffer_ref_entry->count;
    index_for_each (arg->buffers->child_reach_index,
		    index_begin (arg->buffers->child_reach_index),
		    index_end (arg->buffers->child_reach_index),
		    transfer2,
		    arg);

  }
}

void
buffers_add_child (buffers_t* buffers, aid_t aid, bid_t parent, bid_t child)
{
  assert (buffers != NULL);

  /*
   * NO CHECK IF AID EXISTS.
   */

  pthread_rwlock_wrlock (&buffers->lock);

  buffer_edge_entry_t* edge_entry = buffer_edge_entry_for_parent_child (buffers, parent, child, NULL);
  if (edge_entry == NULL) {
    /* Edge doesnt' exist. */

    buffer_entry_t* parent_entry = buffer_entry_for_bid (buffers, parent, NULL);
    buffer_ref_entry_t* parent_ref_entry = buffer_ref_entry_for_aid_bid (buffers, aid, parent, NULL);
    buffer_entry_t* child_entry = buffer_entry_for_bid (buffers, child, NULL);
    buffer_ref_entry_t* child_ref_entry = buffer_ref_entry_for_aid_bid (buffers, aid, child, NULL);
    
    /* Parent and child must exist and aid must be owner or have a reference. */
    if ((parent != child) &&
	((parent_entry != NULL && parent_entry->owner == aid) ||
	 (parent_ref_entry != NULL)) &&
	((child_entry != NULL && child_entry->owner == aid) ||
	 (child_ref_entry != NULL))) {
      
      /* Find buffers reachable from the child. */
      find_reachable_buffers (buffers, child, buffers->child_reach_index);
      
      if (index_find_value (buffers->child_reach_index,
			    index_begin (buffers->child_reach_index),
			    index_end (buffers->child_reach_index),
			    bid_equal,
			    &parent,
			    NULL) == NULL) {
        /* Parent is not reachable from child so we have avoided a cycle. */
	
	/* Find the buffers reachable from the parent. */
	find_reachable_buffers (buffers, parent, buffers->parent_reach_index);
	
	/* Remove the parent set from the child set. */
	index_remove (buffers->child_reach_index,
		      index_begin (buffers->child_reach_index),
		      index_end (buffers->child_reach_index),
		      in_parent,
		      buffers);
	
	/* Transfer references from the parent to the child. */
	transfer_arg_t arg = {
	  .mode = TRANSFER,
	  .buffers = buffers,
	  .parent = parent
	};
	index_for_each (buffers->buffer_ref_index,
			index_begin (buffers->buffer_ref_index),
			index_end (buffers->buffer_ref_index),
			transfer1,
			&arg);
      
	/* Add to edge table. */
	buffer_edge_entry_t edge_key = {
	  .parent = parent,
	  .child = child,
	};
	index_insert (buffers->buffer_edge_index, &edge_key);
      }
    }
  }

  pthread_rwlock_unlock (&buffers->lock);
}

void
buffers_remove_child (buffers_t* buffers, aid_t aid, bid_t parent, bid_t child)
{
  assert (buffers != NULL);

  /*
   * NO CHECK IF AID EXISTS.
   */

  pthread_rwlock_wrlock (&buffers->lock);

  iterator_t edge_entry_idx;
  buffer_edge_entry_t* edge_entry = buffer_edge_entry_for_parent_child (buffers, parent, child, &edge_entry_idx);
  if (edge_entry != NULL) {
    /* Edge doesnt' exist. */

    buffer_entry_t* parent_entry = buffer_entry_for_bid (buffers, parent, NULL);
    buffer_ref_entry_t* parent_ref_entry = buffer_ref_entry_for_aid_bid (buffers, aid, parent, NULL);
    buffer_entry_t* child_entry = buffer_entry_for_bid (buffers, child, NULL);
    buffer_ref_entry_t* child_ref_entry = buffer_ref_entry_for_aid_bid (buffers, aid, child, NULL);
    
    /* Parent and child must exist and aid must be owner or have a reference. */
    if ((parent != child) &&
  	((parent_entry != NULL && parent_entry->owner == aid) ||
  	 (parent_ref_entry != NULL)) &&
  	((child_entry != NULL && child_entry->owner == aid) ||
  	 (child_ref_entry != NULL))) {

      /* Remove the edge from the table. */
      index_erase (buffers->buffer_edge_index, edge_entry_idx);
      
      /* Find buffers reachable from the child. */
      find_reachable_buffers (buffers, child, buffers->child_reach_index);
      
      /* Find the buffers reachable from the parent. */
      find_reachable_buffers (buffers, parent, buffers->parent_reach_index);
	
      /* Remove the parent set from the child set. */
      index_remove (buffers->child_reach_index,
		    index_begin (buffers->child_reach_index),
		    index_end (buffers->child_reach_index),
		    in_parent,
		    buffers);
	
      /* Untransfer references from the parent to the child. */
      transfer_arg_t arg = {
	.mode = UNTRANSFER,
	.buffers = buffers,
	.parent = parent
      };
      index_for_each (buffers->buffer_ref_index,
		      index_begin (buffers->buffer_ref_index),
		      index_end (buffers->buffer_ref_index),
		      transfer1,
		      &arg);      
    }    
  }

  pthread_rwlock_unlock (&buffers->lock);
}

typedef struct {
  buffers_t* buffers;
  aid_t aid;
} null_arg_t;

static void
null_aid (void* e, void* a)
{
  buffer_entry_t* buffer_entry = e;
  null_arg_t* arg = a;
  
  if (buffer_entry->owner == arg->aid) {
    buffer_entry->owner = -1;
  }
}

static void
dec_global (const void* e, void* a)
{
  const buffer_ref_entry_t* buffer_ref_entry = e;
  null_arg_t* arg = a;

  if (buffer_ref_entry->aid == arg->aid) {
    iterator_t buffer_entry_idx;
    buffer_entry_t* buffer_entry = buffer_entry_for_bid (arg->buffers, buffer_ref_entry->bid, &buffer_entry_idx);
    assert (buffer_entry != NULL);
    buffer_entry->ref_count -= buffer_ref_entry->count;

    if (buffer_entry->ref_count == 0) {
      remove_buffer_entry (arg->buffers, buffer_entry, buffer_entry_idx);
    }
    
  }
}

void
buffers_purge_aid (buffers_t* buffers, aid_t aid)
{
  assert (buffers != NULL);

  /*
   * NO CHECK IF AID EXISTS.
   */

  pthread_rwlock_wrlock (&buffers->lock);

  /* Update the global counts. */
  null_arg_t arg = {
    .buffers = buffers,
    .aid = aid
  };
  index_for_each (buffers->buffer_ref_index,
		  index_begin (buffers->buffer_ref_index),
		  index_end (buffers->buffer_ref_index),
		  dec_global,
		  &arg);

  /* Remove the reference entries. */
  buffer_ref_entry_t key = {
    .aid = aid
  };
  index_remove (buffers->buffer_ref_index,
		index_begin (buffers->buffer_ref_index),
		index_end (buffers->buffer_ref_index),
		buffer_ref_entry_aid_equal,
		&key);

  /* Remove owners. */
  index_transform (buffers->buffer_index,
		   index_begin (buffers->buffer_index),
		   index_end (buffers->buffer_index),
		   null_aid,
		   &arg);

  pthread_rwlock_unlock (&buffers->lock);
}

typedef struct {
  buffers_t* buffers;
  bid_t old_parent;
  bid_t new_parent;
} edge_arg_t;

static void
dup_edge (const void* e, void* a)
{
  const buffer_edge_entry_t* entry = e;
  edge_arg_t* arg = a;

  if (entry->parent == arg->old_parent) {
    buffer_edge_entry_t key = {
      .parent = arg->new_parent,
      .child = entry->child
    };
    index_insert (arg->buffers->buffer_edge_index, &key);
  }
}

bid_t
buffers_dup (buffers_t* buffers, aid_t aid, bid_t bid)
{
  assert (buffers != NULL);

  /*
   * NO CHECK IF AID EXISTS.
   */

  pthread_rwlock_wrlock (&buffers->lock);

  bid_t retval = -1;

  buffer_entry_t* buffer_entry = buffer_entry_for_bid (buffers, bid, NULL);
  buffer_ref_entry_t* buffer_ref_entry = buffer_ref_entry_for_aid_bid (buffers, aid, bid, NULL);
  /* Only for the owner or if they have a reference. */
  if ((buffer_entry != NULL && buffer_entry->owner == aid && buffer_entry->ref_count != 0) ||
      (buffer_ref_entry != NULL)) {

    /* Allocate a new buffer with the same size. */
    buffer_entry_t* new_entry = allocate (buffers, aid, buffer_entry->size);
    retval = new_entry->bid;
    
    /* Copy the data. */
    memcpy (new_entry->data, buffer_entry->data, new_entry->size);
    
    /* Copy parent child relationships. */
    edge_arg_t arg = {
      .buffers = buffers,
      .old_parent = buffer_entry->bid,
      .new_parent = new_entry->bid
    };
    index_for_each (buffers->buffer_edge_index,
		    index_begin (buffers->buffer_edge_index),
		    index_end (buffers->buffer_edge_index),
		    dup_edge,
		    &arg);
  }

  pthread_rwlock_unlock (&buffers->lock);

  return retval;
}

buffers_t*
buffers_create (void)
{
  buffers_t* buffers = malloc (sizeof (buffers_t));

  buffers->next_bid = 0;
  pthread_rwlock_init (&buffers->lock, NULL);

  buffers->buffer_table = table_create (sizeof (buffer_entry_t));
  buffers->buffer_index = index_create_list (buffers->buffer_table);
  buffers->buffer_ref_table = table_create (sizeof (buffer_ref_entry_t));
  buffers->buffer_ref_index = index_create_list (buffers->buffer_ref_table);
  buffers->buffer_edge_table = table_create (sizeof (buffer_edge_entry_t));
  buffers->buffer_edge_index = index_create_list (buffers->buffer_edge_table);
  buffers->reachable_open_table = table_create (sizeof (bid_t));
  buffers->reachable_open_index = index_create_list (buffers->reachable_open_table);
  buffers->ref_closed_table = table_create (sizeof (bid_t));
  buffers->ref_closed_index = index_create_list (buffers->ref_closed_table);
  buffers->parent_reach_table = table_create (sizeof (bid_t));
  buffers->parent_reach_index = index_create_list (buffers->parent_reach_table);
  buffers->child_reach_table = table_create (sizeof (bid_t));
  buffers->child_reach_index = index_create_list (buffers->child_reach_table);

  return buffers;
}

void
buffers_destroy (buffers_t* buffers)
{
  assert (buffers != NULL);

  table_destroy (buffers->child_reach_table);
  table_destroy (buffers->parent_reach_table);
  table_destroy (buffers->reachable_open_table);
  table_destroy (buffers->ref_closed_table);
  table_destroy (buffers->buffer_edge_table);
  table_destroy (buffers->buffer_ref_table);
  index_for_each (buffers->buffer_index,
		  index_begin (buffers->buffer_index),
		  index_end (buffers->buffer_index),
		  free_data,
		  NULL);
  table_destroy (buffers->buffer_table);

  pthread_rwlock_destroy (&buffers->lock);

  free (buffers);
}
