#include "buffers.hh"

#include <algorithm>
#include <cstring>

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

buffers::buffers () :
  m_next_bid (0)
{
  pthread_rwlock_init (&m_lock, NULL);
}

buffers::~buffers ()
{
  std::for_each (m_buffer_entries.begin (),
		 m_buffer_entries.end (),
		 free_data ());
  pthread_rwlock_destroy (&m_lock);
}

bool
buffers::buffer_exists (bid_t bid) const
{
  return std::find_if (m_buffer_entries.begin (),
		       m_buffer_entries.end (),
		       buffer_entry_bid_equal (bid)) != m_buffer_entries.end ();
}

buffers::buffer_list::const_iterator
buffers::allocate (aid_t owner, size_t size, size_t alignment)
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
  buffer_entry entry;
  entry.bid = bid;
  entry.owner = owner;
  entry.mode = READWRITE;
  entry.size = size;
  entry.alignment = alignment;
  entry.data = data;
  entry.ref_count = 1; /* Creator get's one reference. See next. */

  /* Create new entry. */
  buffer_ref_entry key;
  key.bid = bid;
  key.aid = owner;
  key.count = 1;
  m_buffer_ref_entries.push_back (key);

  return m_buffer_entries.insert (m_buffer_entries.end (), entry);
}

bid_t
buffers::alloc (aid_t owner, size_t size)
{
  /*
   * NO CHECK IF AID EXISTS.
   */

  pthread_rwlock_wrlock (&m_lock);
  buffer_list::const_iterator pos = allocate (owner, size, 0);
  pthread_rwlock_unlock (&m_lock);

  return pos->bid;
}

bid_t
buffers::alloc_aligned (aid_t owner, size_t size, size_t alignment)
{
  /*
   * NO CHECK IF AID EXISTS.
   */

  pthread_rwlock_wrlock (&m_lock);
  buffer_list::const_iterator pos = allocate (owner, size, alignment);
  pthread_rwlock_unlock (&m_lock);

  return pos->bid;
}

void*
buffers::write_ptr (aid_t aid, bid_t bid)
{
  void* ptr;

  pthread_rwlock_rdlock (&m_lock);
  /* Lookup. */
  buffer_list::const_iterator pos = std::find_if (m_buffer_entries.begin (),
						  m_buffer_entries.end (),
						  buffer_entry_bid_equal (bid));
  /* Only for the owner between creation and the first reference. */
  if (pos != m_buffer_entries.end () && pos->owner == aid && pos->mode == READWRITE) {
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
buffers::owner_or_reference (aid_t aid, bid_t bid) const
{
  buffer_list::const_iterator pos = std::find_if (m_buffer_entries.begin (),
						  m_buffer_entries.end (),
						  buffer_entry_bid_equal (bid));
  buffer_ref_list::const_iterator ref_pos = std::find_if (m_buffer_ref_entries.begin (),
							  m_buffer_ref_entries.end (),
							  buffer_ref_entry_aid_bid_equal (aid, bid));
  return
    (pos != m_buffer_entries.end () && pos->owner == aid) ||
    (ref_pos != m_buffer_ref_entries.end ());
}

const void*
buffers::read_ptr (aid_t aid, bid_t bid)
{
  void* ptr;

  pthread_rwlock_rdlock (&m_lock);
  /* Only for the owner or if they have a reference. */
  if (owner_or_reference (aid, bid)) {
    buffer_list::const_iterator pos = std::find_if (m_buffer_entries.begin (),
						    m_buffer_entries.end (),
						    buffer_entry_bid_equal (bid));
    ptr = pos->data;
  }
  else {
    ptr = NULL;
  }
  pthread_rwlock_unlock (&m_lock);

  return ptr;
}

size_t
buffers::size (aid_t aid, bid_t bid)
{
  size_t size;

  pthread_rwlock_rdlock (&m_lock);
  /* Owner or have a reference. */
  if (owner_or_reference (aid, bid)) {
    buffer_list::const_iterator pos = std::find_if (m_buffer_entries.begin (),
						    m_buffer_entries.end (),
						    buffer_entry_bid_equal (bid));
    size = pos->size;
  }
  else {
    size = 0;
  }
  pthread_rwlock_unlock (&m_lock);

  return size;
}

bool
buffers::exists (aid_t aid, bid_t bid)
{
  pthread_rwlock_wrlock (&m_lock);
  bool retval = std::find_if (m_buffer_ref_entries.begin (),
			      m_buffer_ref_entries.end (),
			      buffer_ref_entry_aid_bid_equal (aid, bid)) != m_buffer_ref_entries.end ();
  pthread_rwlock_unlock (&m_lock);
  return retval;
}

void
buffers::change_owner (aid_t aid, bid_t bid)
{
  /*
   * NO CHECK IF AID EXISTS.
   */
  pthread_rwlock_wrlock (&m_lock);
  buffer_list::iterator pos = std::find_if (m_buffer_entries.begin (),
					    m_buffer_entries.end (),
					    buffer_entry_bid_equal (bid));
  assert (pos != m_buffer_entries.end ());
  pos->owner = aid;
  pos->mode = READONLY;
  pthread_rwlock_unlock (&m_lock);
}

void
buffers::find_reachable_buffers (bid_t root_bid, std::set<bid_t>& closed_list) const
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
      std::for_each (m_buffer_edge_entries.begin (),
		     m_buffer_edge_entries.end (),
		     insert_child (bid, open_list));
      // m_buffer_edge_entries.for_each_if (BufferEdgeEntryParentEqual (bid), InsertChild (open_list));
    }
  }
}

void
buffers::incref (bid_t bid, aid_t aid, size_t count)
{
  assert (buffer_exists (bid));

  buffer_ref_list::iterator pos = std::find_if (m_buffer_ref_entries.begin (),
						m_buffer_ref_entries.end (),
						buffer_ref_entry_aid_bid_equal (aid, bid));
  if (pos != m_buffer_ref_entries.end ()) {
    /* Increment reference count. */
    pos->count += count;
  }
  else {
    /* Create new entry. */
    buffer_ref_entry key;
    key.bid = bid;
    key.aid = aid;
    key.count = count;
    m_buffer_ref_entries.push_back (key);
  }

  /* Increment the global reference count. */
  buffer_list::iterator pos2 = std::find_if (m_buffer_entries.begin (),
					     m_buffer_entries.end (),
					     buffer_entry_bid_equal (bid));
  pos2->ref_count += count;
  /* Buffer becomes READONLY the first time (and any subsequent time) it is referenced. */
  pos2->mode = READONLY;
}

void
buffers::incref (aid_t aid, bid_t bid)
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
		   incref_bid (*this, aid, 1));
  }

  pthread_rwlock_unlock (&m_lock);
}

void
buffers::remove_buffer_entry (buffer_list::iterator& pos)
{
  assert (pos->ref_count == 0);

  /* Remove parent-child relationships. */
  buffer_edge_list::iterator edge_pos = m_buffer_edge_entries.begin ();
  while (edge_pos != m_buffer_edge_entries.end ()) {
    if (edge_pos->parent == pos->bid ||
	edge_pos->child == pos->bid) {
      edge_pos = m_buffer_edge_entries.erase (edge_pos);
    }
    else {
      ++edge_pos;
    }
  }
  
  /* Free the data. */
  free (pos->data);
  
  /* Remove. */
  m_buffer_entries.erase (pos);
}

void
buffers::decref (bid_t bid, aid_t aid, size_t count)
{
  assert (buffer_exists (bid));

  buffer_ref_list::iterator pos = std::find_if (m_buffer_ref_entries.begin (),
						m_buffer_ref_entries.end (),
						buffer_ref_entry_aid_bid_equal (aid, bid));

  /* Decrement reference count. */
  pos->count -= count;
  if (pos->count == 0) {
    /* Remove. */
    m_buffer_ref_entries.erase (pos);
  }
  
  /* Decrement the global reference count. */
  buffer_list::iterator pos2 = std::find_if (m_buffer_entries.begin (),
					     m_buffer_entries.end (),
					     buffer_entry_bid_equal (bid));
  pos2->ref_count -= count;

  if (pos2->ref_count == 0) {
    remove_buffer_entry (pos2);
  }
}

void
buffers::decref_core (aid_t aid, bid_t root_bid)
{
  buffer_ref_list::const_iterator pos = std::find_if (m_buffer_ref_entries.begin (),
						      m_buffer_ref_entries.end (),
						      buffer_ref_entry_aid_bid_equal (aid, root_bid));
  /* Must have a reference. */
  if (pos != m_buffer_ref_entries.end ()) {

    /* Find all reachable children. */
    std::set<bid_t> children;

    find_reachable_buffers (root_bid, children);   

    /* Decrement the reference count for all reachable. */
    std::for_each (children.begin (),
		   children.end (),
		   decref_bid (*this, aid, 1));
  }
}

void
buffers::decref (aid_t aid, bid_t root_bid)
{
  /*
   * NO CHECK IF AID EXISTS.
   */

  pthread_rwlock_wrlock (&m_lock);
  decref_core (aid, root_bid);
  pthread_rwlock_unlock (&m_lock);
}

void
buffers::add_child (aid_t aid, bid_t parent, bid_t child)
{
  /*
   * NO CHECK IF AID EXISTS.
   */

  pthread_rwlock_wrlock (&m_lock);

  buffer_edge_list::const_iterator edge_pos = std::find_if (m_buffer_edge_entries.begin (),
							    m_buffer_edge_entries.end (),
							    buffer_edge_entry_parent_child_equal (parent, child));
  if (edge_pos == m_buffer_edge_entries.end ()) {
    /* Edge doesn't exist. */

    buffer_list::const_iterator parent_pos = std::find_if (m_buffer_entries.begin (),
							   m_buffer_entries.end (),
							   buffer_entry_bid_equal (parent));
    buffer_list::const_iterator child_pos = std::find_if (m_buffer_entries.begin (),
							  m_buffer_entries.end (),
							  buffer_entry_bid_equal (child));
    
    buffer_ref_list::const_iterator child_ref_pos = std::find_if (m_buffer_ref_entries.begin (),
								  m_buffer_ref_entries.end (),
								  buffer_ref_entry_aid_bid_equal (aid, child));
  
    /* Parent and child must exist, parent must be READWRITE. */
    if ((parent != child) &&
	(parent_pos != m_buffer_entries.end () && parent_pos->owner == aid && parent_pos->mode == READWRITE) &&
	(child_pos != m_buffer_entries.end ()) &&
	(child_pos->owner == aid || child_ref_pos != m_buffer_ref_entries.end ())) {
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
	std::for_each (m_buffer_ref_entries.begin (),
		       m_buffer_ref_entries.end (),
		       transfer1 (*this, parent, children));

	/* Add to edge table. */
	buffer_edge_entry edge_key;
	edge_key.parent = parent;
	edge_key.child = child;
	m_buffer_edge_entries.push_back (edge_key);
      }
    }
  }

  pthread_rwlock_unlock (&m_lock);
}

void
buffers::remove_child (aid_t aid, bid_t parent, bid_t child)
{
  /*
   * NO CHECK IF AID EXISTS.
   */

  pthread_rwlock_wrlock (&m_lock);

  buffer_edge_list::iterator edge_pos = std::find_if (m_buffer_edge_entries.begin (),
						      m_buffer_edge_entries.end (),
						      buffer_edge_entry_parent_child_equal (parent, child));
  if (edge_pos != m_buffer_edge_entries.end ()) {
    /* Edge does exist. */

    buffer_list::const_iterator parent_pos = std::find_if (m_buffer_entries.begin (),
							   m_buffer_entries.end (),
							   buffer_entry_bid_equal (parent));
    assert (parent_pos != m_buffer_entries.end ());
    
    /* Parent must be READWRITE. */
    if (parent_pos->owner == aid && parent_pos->mode == READWRITE) {
      /* READWRITE implies ref_count == 1. */
      assert (parent_pos->ref_count == 1);

      /* Remove the edge from the table. */
      m_buffer_edge_entries.erase (edge_pos);

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
      std::for_each (m_buffer_ref_entries.begin (),
		     m_buffer_ref_entries.end (),
		     untransfer1 (*this, parent, children));
    }    
  }

  pthread_rwlock_unlock (&m_lock);
}

void
buffers::purge_aid (aid_t aid)
{
  /*
   * NO CHECK IF AID EXISTS.
   */

  pthread_rwlock_wrlock (&m_lock);


  buffer_ref_list::iterator ref_pos = m_buffer_ref_entries.begin ();
  while (ref_pos != m_buffer_ref_entries.end ()) {
    if (ref_pos->aid == aid) {
      /* Update the global counts. */
      buffer_list::iterator pos = std::find_if (m_buffer_entries.begin (),
						m_buffer_entries.end (),
						buffer_entry_bid_equal (ref_pos->bid));
      assert (pos != m_buffer_entries.end ());
      pos->ref_count -= ref_pos->count;
      if (pos->ref_count == 0) {
  	remove_buffer_entry (pos);
      }

      /* Remove the reference entry. */
      ref_pos = m_buffer_ref_entries.erase (ref_pos);
    }
    else {
      ++ref_pos;
    }
  }

  /* Remove owners. */
  std::for_each (m_buffer_entries.begin (),
		 m_buffer_entries.end (),
		 null_aid (aid));
  
  pthread_rwlock_unlock (&m_lock);
}

bid_t
buffers::dup (aid_t aid, bid_t bid, size_t size)
{
  /*
   * NO CHECK IF AID EXISTS.
   */

  pthread_rwlock_wrlock (&m_lock);

  bid_t retval;

  buffer_ref_list::const_iterator buffer_ref_pos = std::find_if (m_buffer_ref_entries.begin (),
								 m_buffer_ref_entries.end (),
								 buffer_ref_entry_aid_bid_equal (aid, bid));
  /* Must have a reference. */
  if (buffer_ref_pos != m_buffer_ref_entries.end ()) {
    buffer_list::iterator buffer_pos = std::find_if (m_buffer_entries.begin (),
						     m_buffer_entries.end (),
						     buffer_entry_bid_equal (bid));
    assert (buffer_pos != m_buffer_entries.end ());

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
      buffer_list::const_iterator new_pos = allocate (aid, size, buffer_pos->alignment);
      retval = new_pos->bid;
      
      /* Copy the data. */
      memcpy (new_pos->data,
	      buffer_pos->data,
	      (buffer_pos->size < new_pos->size) ? buffer_pos->size : new_pos->size);
      
      /* Copy parent child relationships. */
      std::for_each (m_buffer_edge_entries.begin (),
		     m_buffer_edge_entries.end (),
		     duplicate_edge (*this, buffer_pos->bid, new_pos->bid));

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
