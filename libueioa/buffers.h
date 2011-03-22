#ifndef __buffers_h__
#define __buffers_h__

#include <pthread.h>
#include <ueioa.h>
#include <set>
#include <vector>
#include <algorithm>
#include <cassert>

typedef enum {
  READWRITE,
  READONLY,
} buffer_mode_t;

typedef struct {
  bid_t bid;
  aid_t owner;
  buffer_mode_t mode;
  size_t size;
  size_t alignment;
  void* data;
  size_t ref_count;
} buffer_entry_t;

typedef struct {
  aid_t aid;
  bid_t bid;
  size_t count;
} buffer_ref_entry_t;

typedef struct {
  bid_t parent;
  bid_t child;
} buffer_edge_entry_t;

class BufferEntryBidEqual {
private:
  bid_t m_bid;
public:
  BufferEntryBidEqual (bid_t bid) :
    m_bid (bid) { }
  bool operator() (const buffer_entry_t& x) const { return m_bid == x.bid; }
};

class BufferEntryOwnerEqual {
private:
  aid_t m_aid;
public:
  BufferEntryOwnerEqual (aid_t aid) :
    m_aid (aid) { }
  bool operator() (const buffer_entry_t& x) const { return m_aid == x.owner; }
};

class BufferRefEntryAidBidEqual {
private:
  aid_t m_aid;
  bid_t m_bid;
public:
  BufferRefEntryAidBidEqual (aid_t aid, bid_t bid) :
    m_aid (aid),
    m_bid (bid) { }
  bool operator() (const buffer_ref_entry_t& x) const { return m_aid == x.aid && m_bid == x.bid; }
};

class BufferRefEntryAidEqual {
private:
  aid_t m_aid;
public:
  BufferRefEntryAidEqual (aid_t aid) :
    m_aid (aid) { }
  bool operator() (const buffer_ref_entry_t& x) const { return m_aid == x.aid; }
};

class BufferRefEntryBidEqual {
private:
  bid_t m_bid;
public:
  BufferRefEntryBidEqual (bid_t bid) :
    m_bid (bid) { }
  bool operator() (const buffer_ref_entry_t& x) const { return m_bid == x.bid; }
};

class BufferEdgeEntryParentEqual {
private:
  bid_t m_bid;
public:
  BufferEdgeEntryParentEqual (bid_t bid) :
    m_bid (bid) { }
  bool operator() (const buffer_edge_entry_t& x) const { return m_bid == x.parent; }
};

class BufferEdgeEntryParentChildEqual {
private:
  bid_t m_parent;
  bid_t m_child;
public:
  BufferEdgeEntryParentChildEqual (bid_t parent, bid_t child) :
    m_parent (parent),
    m_child (child) { }
  bool operator() (const buffer_edge_entry_t& x) const { return m_parent == x.parent && m_child == x.child; }
};
  
class BufferEdgeEntryParentOrChildEqual {
private:
  bid_t m_bid;
public:
  BufferEdgeEntryParentOrChildEqual (bid_t bid) :
    m_bid (bid) { }
  bool operator() (const buffer_edge_entry_t& x) const { return m_bid == x.parent || m_bid == x.child; }
};

class Buffers {
 public:
  Buffers ();
  ~Buffers ();

  bid_t alloc (aid_t, size_t);
  bid_t alloc_aligned (aid_t, size_t, size_t);
  
  void* write_ptr (aid_t, bid_t);
  const void* read_ptr (aid_t, bid_t);
  size_t size (aid_t, bid_t);
  bool exists (aid_t, bid_t);
  
  void change_owner (aid_t, bid_t);
  void incref (aid_t, bid_t);
  void decref (aid_t, bid_t);
  
  void add_child (aid_t, bid_t, bid_t);
  void remove_child (aid_t, bid_t, bid_t);
  
  void purge_aid (aid_t);
  
  bid_t dup (aid_t, bid_t, size_t);

 private:
  pthread_rwlock_t m_lock;
  bid_t m_next_bid;
  typedef std::list<buffer_entry_t> BufferList;
  BufferList m_buffer_entries;
  typedef std::list<buffer_ref_entry_t> BufferRefList;
  BufferRefList m_buffer_ref_entries;
  typedef std::list<buffer_edge_entry_t> BufferEdgeList;
  BufferEdgeList m_buffer_edge_entries;

  bool buffer_exists (bid_t bid) const;
  BufferList::const_iterator allocate (aid_t owner, size_t size, size_t alignment);
  bool owner_or_reference (aid_t aid, bid_t bid) const;
  void find_reachable_buffers (bid_t root_bid, std::set<bid_t>& target_index) const;
  void incref (bid_t bid, aid_t aid, size_t count);
  void remove_buffer_entry (BufferList::iterator& pos);
  void decref (bid_t bid, aid_t aid, size_t count);
  void decref_core (aid_t aid, bid_t root_bid);

  class Incref {
  private:
    Buffers& m_buffers;
    aid_t m_aid;
    size_t m_count;
  public:
    Incref (Buffers& buffers, aid_t aid, size_t count) :
    m_buffers (buffers),
      m_aid (aid),
      m_count (count) { }
    void operator() (bid_t bid) { m_buffers.incref (bid, m_aid, m_count); }
  };

  class Decref {
  private:
    Buffers& m_buffers;
    aid_t m_aid;
    size_t m_count;
  public:
    Decref (Buffers& buffers, aid_t aid, size_t count) :
    m_buffers (buffers),
      m_aid (aid),
      m_count (count) { }
    void operator() (bid_t bid) { m_buffers.decref (bid, m_aid, m_count); }
  };

  class Transfer2 {
  private:
    Buffers& m_buffers;
    const aid_t m_aid;
    const size_t m_count;
  public:
    Transfer2 (Buffers& buffers, const aid_t aid, const size_t count) :
    m_buffers (buffers),
      m_aid (aid),
      m_count (count) { }
    void operator() (const bid_t& bid) const {
      m_buffers.incref (bid, m_aid, m_count);
    }
  };

  class Transfer1 {
  private:
    Buffers& m_buffers;
    const std::vector<bid_t>& m_children;
  public:
    Transfer1 (Buffers& buffers, const std::vector<bid_t>& children) :
    m_buffers (buffers),
    m_children (children) { }
    void operator() (const buffer_ref_entry_t& x) const
    {
      std::for_each (m_children.begin (),
		     m_children.end (),
		     Transfer2 (m_buffers, x.aid, x.count));
    }
  };

  class Untransfer2 {
  private:
    Buffers& m_buffers;
    const aid_t m_aid;
    const size_t m_count;
  public:
    Untransfer2 (Buffers& buffers, const aid_t aid, const size_t count) :
    m_buffers (buffers),
      m_aid (aid),
      m_count (count) { }
    void operator() (const bid_t& bid) const {
      m_buffers.decref (bid, m_aid, m_count);
    }
  };

  class Untransfer1 {
  private:
    Buffers& m_buffers;
    const std::vector<bid_t>& m_children;
  public:
    Untransfer1 (Buffers& buffers, const std::vector<bid_t>& children) :
    m_buffers (buffers),
    m_children (children) { }
    void operator() (const buffer_ref_entry_t& x) const
    {
      std::for_each (m_children.begin (),
		     m_children.end (),
		     Untransfer2 (m_buffers, x.aid, x.count));
    }
  };

  class DecGlobal {
  private:
    Buffers& m_buffers;
  public:
    DecGlobal (Buffers& buffers) :
    m_buffers (buffers) { }
    void operator() (const buffer_ref_entry_t& x) const {
      BufferList::iterator pos = std::find_if (m_buffers.m_buffer_entries.begin (),
					       m_buffers.m_buffer_entries.end (),
					       BufferEntryBidEqual (x.bid));
      assert (pos != m_buffers.m_buffer_entries.end ());
      pos->ref_count -= x.count;
      if (pos->ref_count == 0) {
	m_buffers.remove_buffer_entry (pos);
      }
    }
  };

  class DuplicateEdge {
  private:
    Buffers& m_buffers;
    aid_t m_new_parent;
  public:
    DuplicateEdge (Buffers& buffers, const aid_t new_parent) :
    m_buffers (buffers),
      m_new_parent (new_parent) { }
    void operator() (const buffer_edge_entry_t& x) const
    {
      buffer_edge_entry_t key;
      key.parent = m_new_parent;
      key.child = x.child;
      m_buffers.m_buffer_edge_entries.push_back (key);
    }
  };

};


#endif /* __buffers_h__ */
