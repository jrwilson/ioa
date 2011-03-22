#include "buffers.h"

#include <pthread.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <algorithm>

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
  1.  Allocation
      After allocation, the allocating automaton 1) is owned by the creator, 2) is READWRITE, and 3) has a single reference from the creator.
      Owning a buffer enables functions such as _size and _read_ptr.
      When a buffer is READWRITE, functions such as _write_ptr and add/remove_child can be used.
  2.  The creator acquires a pointer to the storage backing the buffer (_write_ptr) and initializes it.
  3.  Any automaton that owns the buffer or already has a reference on the buffer can reference the buffer (_incref) and acquire a pointer to the storage backing the buffer (_read_ptr).
      The _change_owner command can be used to change the owner of a buffer so that other automata can take refernces (_incref).
      The _change_owner command also changes the mode from READWRITE to READONLY.
      Referencing a buffer converts it from READWRITE to READONLY.
  4.  An automaton can decrement the reference count of a buffer (_decref).
      When the reference count is zero, the buffer is destroyed.

  A buffer returned by an output action should have its reference count decremented after the final input has processed the buffer.
  A system following this recommendation will evolve as follows.

  Event              | Count     | Mode
  -------------------------------------
  Create             | 1         | READWRITE
  Creator incref     | 1 + m     | READONLY or READWRITE if (m == 0)
  Return from output |           |
  Change owner       | 1 + m     | READONLY
  Input increfs      | 1 + m + n | READONLY
  System decref      | m + n     | READONLY

  If the creator takes no additional references (m == 0) and the inputs don't reference the buffer (n == 0), then the buffer will be destroyed.

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
  In order to add or remove children, the parent buffer must be READWRITE.

  Duplicating Buffers
  -------------------

  Buffers can be duplicated.
  Duplicate buffers have the same semantics as new buffers except they have the same children as the original.
  The automaton requesting a duplicate must have a reference to the buffer being duplicated.
  The duplication process implicitly decrements the reference count by one.
  This allows the system to reuse the original buffer if buffer has a single reference which necessary from the automaton requesting the duplicate.

 */

Buffers::Buffers () :
  m_next_bid (0),
  m_buffer_index (m_buffer_table),
  m_buffer_ref_index (m_buffer_ref_table),
  m_buffer_edge_index (m_buffer_edge_table)
{
  pthread_rwlock_init (&m_lock, NULL);
}

class FreeData {
public:
  void operator() (const buffer_entry_t& x) const {
    free (x.data);
  }
};

Buffers::~Buffers ()
{
  std::for_each (m_buffer_index.begin (),
		 m_buffer_index.end (),
		 FreeData ());
  pthread_rwlock_destroy (&m_lock);
}

bool
Buffers::buffer_exists (bid_t bid) const
{
  return m_buffer_index.find_if (BufferEntryBidEqual (bid)) != m_buffer_index.end ();
}

Buffers::BufferIndexType::const_iterator
Buffers::allocate (aid_t owner, size_t size, size_t alignment)
{
  /* Find a bid. */
  do {
    ++m_next_bid;
    if (m_next_bid < 0) {
      m_next_bid = 0;
    }
  } while (buffer_exists (m_next_bid));

  bid_t bid = m_next_bid;

  void* data = NULL;
  if (size > 0) {
    if (alignment == 0) {
      data = malloc (size);
    }
    else {
      assert (posix_memalign (&data, alignment, size) == 0);
    }
  }

  /* Insert. */
  buffer_entry_t entry;
  entry.bid = bid;
  entry.owner = owner;
  entry.mode = READWRITE;
  entry.size = size;
  entry.alignment = alignment;
  entry.data = data;
  entry.ref_count = 1; /* Creator get's one reference. See next. */

  /* Create new entry. */
  buffer_ref_entry_t key;
  key.bid = bid;
  key.aid = owner;
  key.count = 1;
  m_buffer_ref_index.insert (key);

  return m_buffer_index.insert (entry);
}

bid_t
Buffers::alloc (aid_t owner, size_t size)
{
  /*
   * NO CHECK IF AID EXISTS.
   */

  pthread_rwlock_wrlock (&m_lock);
  BufferIndexType::const_iterator pos = allocate (owner, size, 0);
  pthread_rwlock_unlock (&m_lock);

  return pos->bid;
}

bid_t
Buffers::alloc_aligned (aid_t owner, size_t size, size_t alignment)
{
  /*
   * NO CHECK IF AID EXISTS.
   */

  pthread_rwlock_wrlock (&m_lock);
  BufferIndexType::const_iterator pos = allocate (owner, size, alignment);
  pthread_rwlock_unlock (&m_lock);

  return pos->bid;
}

void*
Buffers::write_ptr (aid_t aid, bid_t bid)
{
  void* ptr;

  pthread_rwlock_rdlock (&m_lock);
  /* Lookup. */
  BufferIndexType::const_iterator pos = m_buffer_index.find_if (BufferEntryBidEqual (bid));
  /* Only for the owner between creation and the first reference. */
  if (pos != m_buffer_index.end () && pos->owner == aid && pos->mode == READWRITE) {
    /* READWRITE implies ref_count == 1. */
    assert (pos->ref_count == 1);
    ptr = pos->data;
  }
  else {
    ptr = NULL;
  }
  pthread_rwlock_unlock (&m_lock);

  return ptr;
}

bool
Buffers::owner_or_reference (aid_t aid, bid_t bid) const
{
  BufferIndexType::const_iterator pos = m_buffer_index.find_if (BufferEntryBidEqual (bid));
  BufferRefIndexType::const_iterator ref_pos = m_buffer_ref_index.find_if (BufferRefEntryAidBidEqual (aid, bid));
  return
    (pos != m_buffer_index.end () && pos->owner == aid) ||
    (ref_pos != m_buffer_ref_index.end ());
}

const void*
Buffers::read_ptr (aid_t aid, bid_t bid)
{
  void* ptr;

  pthread_rwlock_rdlock (&m_lock);
  /* Only for the owner or if they have a reference. */
  if (owner_or_reference (aid, bid)) {
    BufferIndexType::const_iterator pos = m_buffer_index.find_if (BufferEntryBidEqual (bid));
    ptr = pos->data;
  }
  else {
    ptr = NULL;
  }
  pthread_rwlock_unlock (&m_lock);

  return ptr;
}

size_t
Buffers::size (aid_t aid, bid_t bid)
{
  size_t size;

  pthread_rwlock_rdlock (&m_lock);
  /* Owner or have a reference. */
  if (owner_or_reference (aid, bid)) {
    BufferIndexType::const_iterator pos = m_buffer_index.find_if (BufferEntryBidEqual (bid));
    size = pos->size;
  }
  else {
    size = 0;
  }
  pthread_rwlock_unlock (&m_lock);

  return size;
}

/* size_t */
/* buffers_ref_count (buffers_t* buffers, aid_t aid, bid_t bid) */
/* { */
/*   assert (buffers != NULL); */

/*   /\* Lookup. *\/ */
/*   buffer_entry_t* buffer_entry = buffer_entry_for_bid (buffers, bid, NULL); */
/*   buffer_ref_entry_t* buffer_ref_entry = buffer_ref_entry_for_aid_bid (buffers, aid, bid, NULL); */
/*   /\* Owner or have a reference. *\/ */
/*   if ((buffer_entry != NULL && buffer_entry->owner == aid) || */
/*       (buffer_ref_entry != NULL)) { */
/*     return buffer_ref_entry->count; */
/*   } */
/*   else { */
/*     return 0; */
/*   } */
/* } */

// bool
// Buffers::exists (aid_t aid, bid_t bid)
// {
//   pthread_rwlock_wrlock (&m_lock);
//   bool retval = m_buffer_ref_index.find_if (BufferRefEntryAidBidEqual (aid, bid)) != m_buffer_ref_index.end ();
//   pthread_rwlock_unlock (&m_lock);
//   return retval;
// }

void
Buffers::change_owner (aid_t aid, bid_t bid)
{
  /*
   * NO CHECK IF AID EXISTS.
   */
  pthread_rwlock_wrlock (&m_lock);
  BufferIndexType::iterator pos = m_buffer_index.find_if (BufferEntryBidEqual (bid));
  assert (pos != m_buffer_index.end ());
  pos->owner = aid;
  pos->mode = READONLY;
  pthread_rwlock_unlock (&m_lock);
}

// typedef struct {
//   buffers_t* buffers;
//   bid_t parent;
// } insert_arg_t;

// static void
// insert_child_into_open (const void* value, void* a)
// {
//   const buffer_edge_entry_t* entry = (const buffer_edge_entry_t*)value;
//   insert_arg_t* arg = (insert_arg_t*)a;

//   if (entry->parent == arg->parent) {
//     /* Insert into the open list. */
//     index_insert (arg->m_reachable_open_index, &entry->child);
//   }
// }

class InsertChild {
private:
  std::set<bid_t>& m_open_list;
public:
  InsertChild (std::set<bid_t>& open_list) :
    m_open_list (open_list) { }
  void operator() (const buffer_edge_entry_t& x) { m_open_list.insert (x.child); }
};

void
Buffers::find_reachable_buffers (bid_t root_bid, std::set<bid_t>& closed_list) const
{
  /* We have an open list of buffers and we have a closed list of buffers. */
  std::set<bid_t> open_list;
  closed_list.clear ();
  
  /* Seed the open list. */
  open_list.insert (root_bid);
   
  while (!open_list.empty ()) {
    /* Pop from open. */
    std::set<bid_t>::const_iterator pos = open_list.begin ();
    bid_t bid = *pos;
    open_list.erase (pos);

    std::set<bid_t>::const_iterator pos2 = closed_list.find (bid);

    if (pos2 == closed_list.end ()) {
      /* Not in closed list. */
      
      /* Push to closed. */
      closed_list.insert (pos2, bid);

      /* Insert all children into open. */
      m_buffer_edge_index.for_each_if (BufferEdgeEntryParentEqual (bid), InsertChild (open_list));
    }
  }
}

void
Buffers::incref (bid_t bid, aid_t aid, size_t count)
{
  assert (buffer_exists (bid));

  BufferRefIndexType::iterator pos = m_buffer_ref_index.find_if (BufferRefEntryAidBidEqual (aid, bid));
  if (pos != m_buffer_ref_index.end ()) {
    /* Increment reference count. */
    pos->count += count;
  }
  else {
    /* Create new entry. */
    buffer_ref_entry_t key;
    key.bid = bid;
    key.aid = aid;
    key.count = count;
    m_buffer_ref_index.insert (key);
  }

  /* Increment the global reference count. */
  BufferIndexType::iterator pos2 = m_buffer_index.find_if (BufferEntryBidEqual (bid));
  pos2->ref_count += count;
  /* Buffer becomes READONLY the first time (and any subsequent time) it is referenced. */
  pos2->mode = READONLY;
}

void
Buffers::incref (aid_t aid, bid_t bid)
{
  /*
   * NO CHECK IF AID EXISTS.
   */

  pthread_rwlock_wrlock (&m_lock);

  /* Must be owner or already have a reference. */
  if (owner_or_reference (aid, bid)) {

    /* Find all reachable children. */
    std::set<bid_t> children;

    find_reachable_buffers (bid, children);
    
    /* Increment the reference count for all reachable. */
    std::for_each (children.begin (),
		   children.end (),
		   Incref (*this, aid, 1));
  }

  pthread_rwlock_unlock (&m_lock);
}

void
Buffers::remove_buffer_entry (BufferIndexType::iterator& pos)
{
  assert (pos->ref_count == 0);

  /* Remove parent-child relationships. */
  m_buffer_edge_index.remove_if (BufferEdgeEntryParentOrChildEqual (pos->bid));
  
  /* Free the data. */
  free (pos->data);
  
  /* Remove. */
  m_buffer_index.erase (pos);
}

void
Buffers::decref (bid_t bid, aid_t aid, size_t count)
{
  assert (buffer_exists (bid));

  BufferRefIndexType::iterator pos = m_buffer_ref_index.find_if (BufferRefEntryAidBidEqual (aid, bid));

  /* Decrement reference count. */
  pos->count -= count;
  if (pos->count == 0) {
    /* Remove. */
    m_buffer_ref_index.erase (pos);
  }
  
  /* Decrement the global reference count. */
  BufferIndexType::iterator pos2 = m_buffer_index.find_if (BufferEntryBidEqual (bid));
  pos2->ref_count -= count;

  if (pos2->ref_count == 0) {
    remove_buffer_entry (pos2);
  }
}

void
Buffers::decref_core (aid_t aid, bid_t root_bid)
{
  BufferRefIndexType::const_iterator pos = m_buffer_ref_index.find_if (BufferRefEntryAidBidEqual (aid, root_bid));
  /* Must have a reference. */
  if (pos != m_buffer_ref_index.end ()) {

    /* Find all reachable children. */
    std::set<bid_t> children;

    find_reachable_buffers (root_bid, children);   

    /* Decrement the reference count for all reachable. */
    std::for_each (children.begin (),
		   children.end (),
		   Decref (*this, aid, 1));
  }
}

void
Buffers::decref (aid_t aid, bid_t root_bid)
{
  /*
   * NO CHECK IF AID EXISTS.
   */

  pthread_rwlock_wrlock (&m_lock);
  decref_core (aid, root_bid);
  pthread_rwlock_unlock (&m_lock);
}

void
Buffers::add_child (aid_t aid, bid_t parent, bid_t child)
{
  /*
   * NO CHECK IF AID EXISTS.
   */

  pthread_rwlock_wrlock (&m_lock);

  BufferEdgeIndexType::const_iterator edge_pos = m_buffer_edge_index.find_if (BufferEdgeEntryParentChildEqual (parent, child));
  if (edge_pos == m_buffer_edge_index.end ()) {
    /* Edge doesn't exist. */

    BufferIndexType::const_iterator parent_pos = m_buffer_index.find_if (BufferEntryBidEqual (parent));
    BufferIndexType::const_iterator child_pos = m_buffer_index.find_if (BufferEntryBidEqual (child));
    
    BufferRefIndexType::const_iterator child_ref_pos = m_buffer_ref_index.find_if (BufferRefEntryAidBidEqual (aid, child));
  
    /* Parent and child must exist, parent must be READWRITE. */
    if ((parent != child) &&
	(parent_pos != m_buffer_index.end () && parent_pos->owner == aid && parent_pos->mode == READWRITE) &&
	(child_pos != m_buffer_index.end ()) &&
	(child_pos->owner == aid || child_ref_pos != m_buffer_ref_index.end ())) {
      /* READWRITE implies ref_count == 1. */
      assert (parent_pos->ref_count == 1);

      /* Find buffers reachable from the child. */
      std::set<bid_t> reachable_from_child;
      find_reachable_buffers (child, reachable_from_child);
      
      if (reachable_from_child.count (parent) == 0) {
        /* Parent is not reachable from child so we have avoided a cycle. */
	
	/* Find the buffers reachable from the parent. */
	std::set<bid_t> reachable_from_parent;
	find_reachable_buffers (parent, reachable_from_parent);

	/* Remove the child set from the parent. */
	std::vector<bid_t> children;
	std::set_difference (reachable_from_parent.begin (),
			     reachable_from_parent.end (),
			     reachable_from_child.begin (),
			     reachable_from_child.end (),
			     std::back_inserter (children));
	
	/* Transfer references from the parent to the child. */
	m_buffer_ref_index.for_each_if (BufferRefEntryBidEqual (parent), Transfer1 (*this, children));

	/* Add to edge table. */
	buffer_edge_entry_t edge_key;
	edge_key.parent = parent;
	edge_key.child = child;
	m_buffer_edge_index.insert (edge_key);
      }
    }
  }

  pthread_rwlock_unlock (&m_lock);
}

void
Buffers::remove_child (aid_t aid, bid_t parent, bid_t child)
{
  /*
   * NO CHECK IF AID EXISTS.
   */

  pthread_rwlock_wrlock (&m_lock);

  BufferEdgeIndexType::const_iterator edge_pos = m_buffer_edge_index.find_if (BufferEdgeEntryParentChildEqual (parent, child));
  if (edge_pos != m_buffer_edge_index.end ()) {
    /* Edge does exist. */

    BufferIndexType::const_iterator parent_pos = m_buffer_index.find_if (BufferEntryBidEqual (parent));    
    assert (parent_pos != m_buffer_index.end ());
    
    /* Parent must be READWRITE. */
    if (parent_pos->owner == aid && parent_pos->mode == READWRITE) {
      /* READWRITE implies ref_count == 1. */
      assert (parent_pos->ref_count == 1);

      /* Remove the edge from the table. */
      m_buffer_edge_index.erase (edge_pos);

      /* Find buffers reachable from the child. */
      std::set<bid_t> reachable_from_child;
      find_reachable_buffers (child, reachable_from_child);

      /* Find the buffers reachable from the parent. */
      std::set<bid_t> reachable_from_parent;
      find_reachable_buffers (parent, reachable_from_parent);

      /* Remove the child set from the parent. */
      std::vector<bid_t> children;
      std::set_difference (reachable_from_parent.begin (),
			   reachable_from_parent.end (),
			   reachable_from_child.begin (),
			   reachable_from_child.end (),
			   std::back_inserter (children));
      
      /* Untransfer references from the parent to the child. */
      m_buffer_ref_index.for_each_if (BufferRefEntryBidEqual (parent), Untransfer1 (*this, children));
    }    
  }

  pthread_rwlock_unlock (&m_lock);
}

class NullAid {
public:
  void operator() (buffer_entry_t& x) const {
    x.owner = -1;
  }
};

void
Buffers::purge_aid (aid_t aid)
{
  /*
   * NO CHECK IF AID EXISTS.
   */

  pthread_rwlock_wrlock (&m_lock);

  /* Update the global counts. */
  m_buffer_ref_index.for_each_if (BufferRefEntryAidEqual (aid), DecGlobal (*this));

  /* Remove the reference entries. */
  m_buffer_ref_index.remove_if (BufferRefEntryAidEqual (aid));

  /* Remove owners. */
  m_buffer_index.for_each_if (BufferEntryOwnerEqual (aid), NullAid ());

  pthread_rwlock_unlock (&m_lock);
}

bid_t
Buffers::dup (aid_t aid, bid_t bid, size_t size)
{
  /*
   * NO CHECK IF AID EXISTS.
   */

  pthread_rwlock_wrlock (&m_lock);

  bid_t retval;

  BufferRefIndexType::const_iterator buffer_ref_pos = m_buffer_ref_index.find_if (BufferRefEntryAidBidEqual (aid, bid));
  /* Must have a reference. */
  if (buffer_ref_pos != m_buffer_ref_index.end ()) {
    BufferIndexType::iterator buffer_pos = m_buffer_index.find_if (BufferEntryBidEqual (bid));
    assert (buffer_pos != m_buffer_index.end ());

    if (buffer_pos->ref_count == 1) {
      /*
	This aid holds the only reference.
	Instead of allocating and copying, we can just change the owner and mode.
      */
      buffer_pos->owner = aid;
      buffer_pos->mode = READWRITE;
      if (size != buffer_pos->size) {
	buffer_pos->size = size;
	buffer_pos->data = realloc (buffer_pos->data, size);
      }
      retval = bid;
    }
    else {
      /* Allocate a new buffer with the new size. */
      BufferIndexType::const_iterator new_pos = allocate (aid, size, buffer_pos->alignment);
      retval = new_pos->bid;
      
      /* Copy the data. */
      memcpy (new_pos->data,
	      buffer_pos->data,
	      (buffer_pos->size < new_pos->size) ? buffer_pos->size : new_pos->size);
      
      /* Copy parent child relationships. */
      m_buffer_edge_index.for_each_if (BufferEdgeEntryParentEqual (buffer_pos->bid), DuplicateEdge (*this, new_pos->bid));

      /* Decrement the reference count. */
      decref_core (aid, bid);
    }
  }
  else {
    retval = -1;
  }

  pthread_rwlock_unlock (&m_lock);

  return retval;
}
