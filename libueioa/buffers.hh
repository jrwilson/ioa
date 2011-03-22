#ifndef __buffers_hh__
#define __buffers_hh__

#include <pthread.h>
#include <ueioa.h>
#include <set>
#include <vector>
#include <algorithm>
#include <cassert>
#include <list>

class buffers {
 public:
  buffers ();
  ~buffers ();

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
  typedef enum {
    READWRITE,
    READONLY,
  } buffer_mode_t;

  struct buffer_entry {
    bid_t bid;
    aid_t owner;
    buffer_mode_t mode;
    size_t size;
    size_t alignment;
    void* data;
    size_t ref_count;
  };

  class buffer_entry_bid_equal {
  private:
    bid_t m_bid;
  public:
    buffer_entry_bid_equal (bid_t bid) :
      m_bid (bid) { }
    bool operator() (const buffer_entry& x) const { return m_bid == x.bid; }
  };

  struct buffer_ref_entry {
    aid_t aid;
    bid_t bid;
    size_t count;
  };

  class buffer_ref_entry_aid_bid_equal {
  private:
    aid_t m_aid;
    bid_t m_bid;
  public:
    buffer_ref_entry_aid_bid_equal (aid_t aid, bid_t bid) :
      m_aid (aid),
      m_bid (bid) { }
    bool operator() (const buffer_ref_entry& x) const { return m_aid == x.aid && m_bid == x.bid; }
  };

  struct buffer_edge_entry {
    bid_t parent;
    bid_t child;
  };
  
  class buffer_edge_entry_parent_child_equal {
  private:
    bid_t m_parent;
    bid_t m_child;
  public:
    buffer_edge_entry_parent_child_equal (bid_t parent, bid_t child) :
      m_parent (parent),
      m_child (child) { }
    bool operator() (const buffer_edge_entry& x) const { return m_parent == x.parent && m_child == x.child; }
  };

  pthread_rwlock_t m_lock;
  bid_t m_next_bid;
  typedef std::list<buffer_entry> buffer_list;
  buffer_list m_buffer_entries;
  typedef std::list<buffer_ref_entry> buffer_ref_list;
  buffer_ref_list m_buffer_ref_entries;
  typedef std::list<buffer_edge_entry> buffer_edge_list;
  buffer_edge_list m_buffer_edge_entries;

  bool buffer_exists (bid_t bid) const;
  buffer_list::const_iterator allocate (aid_t owner, size_t size, size_t alignment);
  bool owner_or_reference (aid_t aid, bid_t bid) const;
  void find_reachable_buffers (bid_t root_bid, std::set<bid_t>& target_index) const;
  void incref (bid_t bid, aid_t aid, size_t count);
  void remove_buffer_entry (buffer_list::iterator& pos);
  void decref (bid_t bid, aid_t aid, size_t count);
  void decref_core (aid_t aid, bid_t root_bid);

  class free_data {
  public:
    void operator() (const buffer_entry& x) const {
      free (x.data);
    }
  };

  class insert_child {
  private:
    const bid_t m_bid;
    std::set<bid_t>& m_open_list;
  public:
    insert_child (const bid_t& bid, std::set<bid_t>& open_list) :
      m_bid (bid),
      m_open_list (open_list) { }
    void operator() (const buffer_edge_entry& x)
    {
      if (m_bid == x.parent) {
	m_open_list.insert (x.child);
      }
    }
  };

  class incref_bid {
  private:
    buffers& m_buffers;
    aid_t m_aid;
    size_t m_count;
  public:
    incref_bid (buffers& buffers, aid_t aid, size_t count) :
    m_buffers (buffers),
      m_aid (aid),
      m_count (count) { }
    void operator() (bid_t bid) { m_buffers.incref (bid, m_aid, m_count); }
  };

  class decref_bid {
  private:
    buffers& m_buffers;
    aid_t m_aid;
    size_t m_count;
  public:
    decref_bid (buffers& buffers, aid_t aid, size_t count) :
    m_buffers (buffers),
      m_aid (aid),
      m_count (count) { }
    void operator() (bid_t bid) { m_buffers.decref (bid, m_aid, m_count); }
  };

  class transfer2 {
  private:
    buffers& m_buffers;
    const aid_t m_aid;
    const size_t m_count;
  public:
    transfer2 (buffers& buffers, const aid_t aid, const size_t count) :
    m_buffers (buffers),
      m_aid (aid),
      m_count (count) { }
    void operator() (const bid_t& bid) const {
      m_buffers.incref (bid, m_aid, m_count);
    }
  };

  class transfer1 {
  private:
    buffers& m_buffers;
    const bid_t m_bid;
    const std::vector<bid_t>& m_children;
  public:
    transfer1 (buffers& buffers, const bid_t& bid, const std::vector<bid_t>& children) :
    m_buffers (buffers),
    m_bid (bid),
    m_children (children) { }
    void operator() (const buffer_ref_entry& x) const
    {
      if (m_bid == x.bid) {
	std::for_each (m_children.begin (),
		       m_children.end (),
		       transfer2 (m_buffers, x.aid, x.count));
      }
    }
  };

  class untransfer2 {
  private:
    buffers& m_buffers;
    const aid_t m_aid;
    const size_t m_count;
  public:
    untransfer2 (buffers& buffers, const aid_t aid, const size_t count) :
    m_buffers (buffers),
      m_aid (aid),
      m_count (count) { }
    void operator() (const bid_t& bid) const {
      m_buffers.decref (bid, m_aid, m_count);
    }
  };

  class untransfer1 {
  private:
    buffers& m_buffers;
    const bid_t m_bid;
    const std::vector<bid_t>& m_children;
  public:
    untransfer1 (buffers& buffers, const bid_t& bid, const std::vector<bid_t>& children) :
    m_buffers (buffers),
    m_bid (bid),
    m_children (children) { }
    void operator() (const buffer_ref_entry& x) const
    {
      if (m_bid == x.bid) {
	std::for_each (m_children.begin (),
		       m_children.end (),
		       untransfer2 (m_buffers, x.aid, x.count));
      }
    }
  };

  class null_aid {
  private:
    const aid_t m_aid;
  public:
    null_aid (const aid_t& aid) :
      m_aid (aid) { }
    void operator() (buffer_entry& x) const {
      if (m_aid == x.owner) {
	x.owner = -1;
      }
    }
  };

  class duplicate_edge {
  private:
    buffers& m_buffers;
    const aid_t m_old_parent;
    const aid_t m_new_parent;
  public:
    duplicate_edge (buffers& buffers, const aid_t old_parent, const aid_t new_parent) :
      m_buffers (buffers),
      m_old_parent (old_parent),
      m_new_parent (new_parent) { }
    void operator() (const buffer_edge_entry& x) const
    {
      if (m_old_parent == x.parent) {
	buffer_edge_entry key;
	key.parent = m_new_parent;
	key.child = x.child;
	m_buffers.m_buffer_edge_entries.push_back (key);
      }
    }
  };

};


#endif /* __buffers_hh__ */
