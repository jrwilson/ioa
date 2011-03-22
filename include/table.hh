#ifndef __table_hh__
#define __table_hh__

#include <memory>
#include <iostream>
#include <vector>
#include <cassert>
#include <list>
#include <algorithm>

// Table Implementation
template <class T, class A> class TableImpl {
public:
  typedef T value_type;
  typedef A allocator_type;
  typedef typename A::size_type size_type;
  typedef typename A::difference_type difference_type;
  typedef typename A::pointer pointer;
  typedef typename A::const_pointer const_pointer;
  typedef typename A::reference reference;
  typedef typename A::const_reference const_reference;

  static const size_type END = -1;

private:
  struct Entry {
    size_t m_prev;
    size_t m_next;
  };
  
  size_type m_size;
  size_type m_capacity;
  std::vector<Entry> m_entries;
  A m_allocator;
  size_type m_used_head;
  pointer m_values;
  size_type m_free_head;
  size_type m_used_tail;
    
  bool is_used (size_type entry) const
  {
    assert (entry != END);
    assert (entry < m_capacity);

    return (m_entries[entry].m_next != m_entries[entry].m_prev) ||
      entry == m_used_head ||
      entry == m_used_tail;
  }
      
  void push_free_list (size_type entry)
  {
    assert (entry < m_capacity);

    m_entries[entry].m_next = m_free_head;
    m_entries[entry].m_prev = m_free_head;

    m_free_head = entry;
  }

  size_type pop_free_list (void)
  {
    assert (m_free_head != END);

    size_type entry = m_free_head;
    m_free_head = m_entries[entry].m_next;
    return entry;
  }
  
public:
  TableImpl () :
    m_size (0),
    m_capacity (1),
    m_entries (m_capacity),
    m_used_head (END),
    m_values (m_allocator.allocate (m_capacity)),
    m_free_head (END),
    m_used_tail (END)
  {
    for (size_type idx = 0; idx < m_capacity; ++idx) {
      push_free_list (idx);
    }
  }

  ~TableImpl ()
  {
    for (size_type idx = m_used_head; idx != END; idx = m_entries[idx].m_next) {
      m_allocator.destroy (m_values + idx);
    }
    
    m_allocator.deallocate (m_values, m_capacity);
  }

  size_type size () const { return m_size; }

  size_type capacity () const { return m_capacity; }

  size_type head () const { return m_used_head; }

  size_type tail () const { return m_used_tail; }

  reference ref (size_type entry) const
  {
    assert (is_used (entry));
    return m_values[entry];
  }

  pointer ptr (size_type entry) const
  {
    assert (is_used (entry));
    return &m_values[entry];
  }

  size_type next (size_type entry) const
  {
    assert (is_used (entry));
    return m_entries[entry].m_next;
  }

  size_type prev (size_type entry) const
  {
    assert (entry != m_used_head);
    if (entry != END) {
      assert (is_used (entry));
      return m_entries[entry].m_prev;
    }
    else {
      return m_used_tail;
    }
  }

  void resize (size_type new_capacity)
  {
    assert (new_capacity >= m_capacity);

    size_type old_capacity = m_capacity;
    m_capacity = new_capacity;
    m_entries.resize (m_capacity);
    T* new_values = m_allocator.allocate (m_capacity);
    
    for (size_type entry = m_used_head; entry != END; entry = m_entries[entry].m_next) {
      m_allocator.construct (new_values + entry, m_values[entry]);
      m_allocator.destroy (m_values + entry);
    }
    m_allocator.deallocate (m_values, old_capacity);
    m_values = new_values;
    
    for (; old_capacity < m_capacity; ++old_capacity) {
      push_free_list (old_capacity);
    }
  }

  size_type insert (size_type next_entry, const T& x)
  {
    assert (next_entry == END || is_used (next_entry));
    assert (m_size < m_capacity);

    /* Get a node from the free list and insert into the used list. */
    size_type entry = pop_free_list ();
    
    /* Copy the data. */
    m_allocator.construct (m_values + entry, x);
    
    size_type prev_entry;
    
    /* Update entry's next pointer. */
    m_entries[entry].m_next = next_entry;
    
    /* Update next entry's prev pointer. */
    if (next_entry != END) {
      prev_entry = m_entries[next_entry].m_prev;
      m_entries[next_entry].m_prev = entry;
    }
    else {
      prev_entry = m_used_tail;
      m_used_tail = entry;
    }
    
    /* Update entry's prev pointer. */
    m_entries[entry].m_prev = prev_entry;
    
    /* Upate the previous entry's next pointer. */
    if (prev_entry != END) {
      m_entries[prev_entry].m_next = entry;
    }
    else {
      m_used_head = entry;
    }
    
    ++m_size;

    return entry;
  }

// static size_t
// erase (table* table, size_t entry)
// {
//   assert (table != NULL);
//   assert (is_used (table, entry));

//   size_t next = table->entries[entry].next;

//   if (table->entries[entry].prev != size_t(-1)) {
//     table->entries[table->entries[entry].prev].next = table->entries[entry].next;
//   }

//   if (table->entries[entry].next != size_t(-1)) {
//     table->entries[table->entries[entry].next].prev = table->entries[entry].prev;
//   }

//   if (entry == table->used_head) {
//     table->used_head = table->entries[entry].next;
//   }

//   if (entry == table->used_tail) {
//     table->used_tail = table->entries[entry].prev;
//   }

//   free_insert (table, entry);
//   --table->size;

//   return next;
// }

// static  void
// clear (table* table)
// {
//   assert (table != NULL);

//   if (table->size * 2 > table->capacity) {
//     /* Over half full.  Reinitialize. */
//     table->used_head = -1;
//     table->used_tail = -1;
//     table->free_head = -1;
//     table->size = 0;
    
//     size_t idx;
//     for (idx = 0; idx < table->capacity; ++idx) {
//       free_insert (table, idx);
//     }
//   }
//   else {
//     /* Remove individually. */
//     while (table->used_head != size_t(-1)) {
//       erase (table, table->used_head);
//     }
//   }

// }

};

// Iterator
template <class T, class A> struct TableIterator {
  typedef std::bidirectional_iterator_tag iterator_category;
  typedef typename A::difference_type difference_type;
  typedef T value_type;
  typedef T* pointer;
  typedef T& reference;
  typedef typename A::size_type size_type;

  TableImpl<T, A>& m_table;
  size_type m_position;

  TableIterator (TableImpl<T, A>& table, size_type position) :
    m_table (table),
    m_position (position) { }
  TableIterator (const TableIterator& i) :
    m_table (i.m_table),
    m_position (i.m_position) { }
  bool operator== (const TableIterator& x) const { return m_position == x.m_position; }
  bool operator!= (const TableIterator& x) const { return m_position != x.m_position; }
  TableIterator& operator++ ()
  {
    m_position = m_table.next (m_position);
    return *this;
  }
  TableIterator& operator-- () {
    assert (m_position != m_table.m_used_head);
    if (m_position != m_table.END) {
      m_position = m_table.m_entries[m_position].m_prev;
    }
    else {
      m_position = m_table.m_used_tail;
    }
    return *this;
  }
  reference operator* () const { return m_table.ref (m_position); }
  pointer operator-> () const { return m_table.ptr (m_position); }
};

// Constant Iterator
template <class T, class A> struct ConstTableIterator {
  typedef std::bidirectional_iterator_tag iterator_category;
  typedef typename A::difference_type difference_type;
  typedef T value_type;
  typedef const T* pointer;
  typedef const T& reference;
  typedef typename A::size_type size_type;
  
  TableImpl<T, A>& m_table;
  size_type m_position;

  ConstTableIterator (TableImpl<T, A>& table, size_type position) :
    m_table (table),
    m_position (position) { }
  ConstTableIterator (const ConstTableIterator& i) :
    m_table (i.m_table),
    m_position (i.m_position) { }
  ConstTableIterator (const TableIterator<T, A>& i) :
    m_table (i.m_table),
    m_position (i.m_position) { }
  bool operator== (const ConstTableIterator& x) const { return m_position == x.m_position; }
  bool operator!= (const ConstTableIterator& x) const { return m_position != x.m_position; }
  ConstTableIterator& operator++ ()
  {
    m_position = m_table.next (m_position);
    return *this;
  }
  ConstTableIterator& operator-- () {
    m_position = m_table.prev (m_position);
    return *this;
  }
  reference operator* () const { return m_table.ref (m_position); }
  pointer operator-> () const { return m_table.ptr (m_position); }
};

template <class T> class IndexInterface {
public:
  typedef typename T::const_iterator const_iterator;
  // I would like these to only be accessible by Table.
  virtual void new_capacity () = 0;
  virtual void insert (const_iterator position) = 0;
};

// Table
template <class T, class A = std::allocator<T> > class Table {
public:
  typedef T value_type;
  typedef A allocator_type;
  typedef typename A::size_type size_type;
  typedef typename A::difference_type difference_type;
  typedef typename A::pointer pointer;
  typedef typename A::const_pointer const_pointer;
  typedef typename A::reference reference;
  typedef typename A::const_reference const_reference;
  typedef TableIterator<T, A> iterator;
  typedef ConstTableIterator<T, A> const_iterator;
  typedef std::reverse_iterator<iterator> reverse_iterator;
  typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
  
private:
  typedef IndexInterface<Table<T> >* IndexPointer;
  typedef typename std::list<IndexPointer> IndexList;

  TableImpl<T, A> m_table;
  IndexList m_indices;
    
public:
  Table ()
  {
  }

  ~Table ()
  {
  }

  // I would like these only to be accessible by Index and friends.
  void add_index (IndexPointer index)
  {
    assert (index != 0);
    m_indices.push_back (index);
  }

  iterator begin () { return iterator (m_table, m_table.head ()); }
  const_iterator begin () const { return const_iterator (m_table, m_table.head ()); }
  iterator end () { return iterator (m_table, m_table.END); }
  const_iterator end () const { return const_iterator (m_table, m_table.END); }
  reverse_iterator rbegin () { return reverse_iterator (end ()); }
  const_reverse_iterator rbegin () const { return const_reverse_iterator (end ()); }
  reverse_iterator rend () { return reverse_iterator (begin ()); }
  const_reverse_iterator rend () const { return const_reverse_iterator (begin ()); }

  bool empty () const { return m_table.size () == 0; }
  size_type size () const { return m_table.size (); }

  reference front () { assert (!empty ()); return m_table.ref (m_table.head ()); }
  const_reference front () const { assert (!empty ()); return m_table.ref (m_table.head ()); }
  reference back () { assert (!empty ()); return m_table.ref (m_table.tail ()); }
  const_reference back () const { assert (!empty ()); return m_table.ref (m_table.tail ()); }

  iterator insert (iterator position, const T& x)
  {
    assert (&m_table == &position.m_table);

    if (m_table.size () == m_table.capacity ()) {
      m_table.resize (m_table.size () << 1);

      // Tell the indices about it.
      for (typename IndexList::iterator pos = m_indices.begin ();
	   pos != m_indices.end ();
	   ++pos) {
	(*pos)->new_capacity ();
      }
    }

    size_type entry = m_table.insert (position.m_position, x);
    
    iterator retval (m_table, entry);

    for (typename IndexList::iterator pos = m_indices.begin ();
	 pos != m_indices.end ();
	 ++pos) {
      (*pos)->insert (retval);
    }
    
    return retval;
  }

};
  
template <class T> class ListIndex : public IndexInterface<T> {
private:
  T& m_table;

public:
  typedef typename T::value_type value_type;
  typedef typename T::size_type size_type;
  typedef typename T::reference reference;
  typedef typename T::const_reference const_reference;
  typedef typename T::iterator iterator;
  typedef typename T::const_iterator const_iterator;
  typedef typename T::reverse_iterator reverse_iterator;
  typedef typename T::const_reverse_iterator const_reverse_iterator;


  virtual void new_capacity () { /* Do nothing. */ }
  virtual void insert (const_iterator position) { /* Do nothing. */ }

  ListIndex (T& table) :
    m_table (table)
  {
    m_table.add_index (this);
  }

  iterator begin () { return m_table.begin (); }
  const_iterator begin () const { return m_table.begin (); }
  iterator end () { return m_table.end (); }
  const_iterator end () const { return m_table.end (); }
  reverse_iterator rbegin () { return m_table.rbegin (); }
  const_reverse_iterator rbegin () const { return m_table.rbegin (); }
  reverse_iterator rend () { return m_table.rend (); }
  const_reverse_iterator rend () const { return m_table.rend (); }

  bool empty () const { return m_table.empty (); }
  size_type size () const { return m_table.size (); }

  reference front () { return m_table.front (); }
  const_reference front () const { return m_table.front (); }
  reference back () { return m_table.back (); }
  const_reference back () const { return m_table.back (); }

  void push_back (const value_type& x) { m_table.insert (m_table.end (), x); }
  void pop_front () { assert (0); }
  
  iterator insert (const value_type& x)
  {
    assert (0);
  }

  iterator insert_unique (const value_type& x)
  {
    assert (0);
    // IndexType::const_iterator pos = std::find (m_index.begin (), m_index.end (), runnable);
    // if (pos == m_index.end ()) {
    //   m_index.insert (runnable);
    // }
  }

  void erase (const_iterator position)
  {
    assert (0);
  }

  template<class Predicate> void remove_if (Predicate p)
  {
    assert (0);
  }

  template <class Predicate> iterator find_if (Predicate p) {
    return std::find_if (begin (),
			 end (),
			 p);
  }

  template <class Predicate> const_iterator find_if (Predicate p) const {
    return std::find_if (begin (),
			 end (),
			 p);
  }

  template <class Predicate, class Action> void for_each_if (Predicate p, Action a)
  {
    for (iterator pos = begin ();
	 pos != end ();
	 ++pos) {
      if (p (*pos)) {
	a (*pos);
      }
    }
  }

  template <class Predicate, class Action> void for_each_if (Predicate p, Action a) const
  {
    for (const_iterator pos = begin ();
	 pos != end ();
	 ++pos) {
      if (p (*pos)) {
	a (*pos);
      }
    }
  }

};

template <class T> class SetIndex : public IndexInterface<T> {
private:
  T& m_table;

public:
  typedef typename T::value_type value_type;
  typedef typename T::size_type size_type;
  typedef typename T::reference reference;
  typedef typename T::const_reference const_reference;
  typedef typename T::iterator iterator;
  typedef typename T::const_iterator const_iterator;
  typedef typename T::reverse_iterator reverse_iterator;
  typedef typename T::const_reverse_iterator const_reverse_iterator;


  virtual void new_capacity () { /* Do nothing. */ }
  virtual void insert (const_iterator position) { /* Do nothing. */ }

  SetIndex (T& table) :
    m_table (table)
  {
    m_table.add_index (this);
  }

  // iterator begin () { return m_table.begin (); }
  // const_iterator begin () const { return m_table.begin (); }
  // iterator end () { return m_table.end (); }
  // const_iterator end () const { return m_table.end (); }
  // reverse_iterator rbegin () { return m_table.rbegin (); }
  // const_reverse_iterator rbegin () const { return m_table.rbegin (); }
  // reverse_iterator rend () { return m_table.rend (); }
  // const_reverse_iterator rend () const { return m_table.rend (); }

  // bool empty () const { return m_table.empty (); }
  // size_type size () const { return m_table.size (); }

  // reference front () { return m_table.front (); }
  // const_reference front () const { return m_table.front (); }
  // reference back () { return m_table.back (); }
  // const_reference back () const { return m_table.back (); }

  // void push_back (const value_type& x) { m_table.insert (m_table.end (), x); }

  template <class Predicate> iterator lower_bound (Predicate p)
  {
    assert (0);
  }

  template <class Predicate> iterator upper_bound (Predicate p)
  {
    assert (0);
  }

  template <class Iterator, class Predicate, class Action> void for_each_if (Iterator begin, Iterator end, Predicate p, Action a)
  {
    for (; begin != end; ++begin) {
      if (p (*begin)) {
	a (*begin);
      }
    }
  }
  
};







// typedef struct index_struct index_t;

// typedef struct {
//   size_t pos;
// } iterator_t;
// typedef struct {
//   size_t pos;
// } riterator_t;
// typedef bool (*predicate_t) (const void*, const void*);

// void index_destroy (index_t*);
// index_t* index_create_list (table*);
// index_t* index_create_ordered_list (table*, predicate_t);

// iterator_t index_advance (index_t*, iterator_t);
// iterator_t index_retreat (index_t*, iterator_t);

// riterator_t index_radvance (index_t*, riterator_t);
// riterator_t index_rretreat (index_t*, riterator_t);

// iterator_t index_begin (index_t*);
// iterator_t index_end (index_t*);
// riterator_t index_rbegin (index_t*);
// riterator_t index_rend (index_t*);

// bool index_empty (index_t*);
// size_t index_size (index_t*);

// void* index_front (index_t*);
// void* index_back (index_t*);
// void* index_value (index_t*, iterator_t);
// void* index_rvalue (index_t*, riterator_t);
// void* index_at (index_t*, size_t);

// /* assign */
// void index_push_front (index_t*, const void*);
// void index_pop_front (index_t*);
// void index_push_back (index_t*, const void*);
// void index_pop_back (index_t*);
// iterator_t index_insert_before (index_t*, iterator_t, const void*);
// iterator_t index_insert (index_t*, const void*);
// iterator_t index_erase (index_t*, iterator_t);
// /* swap */
// void index_clear (index_t*);

// bool iterator_eq (iterator_t iter1, iterator_t iter2);
// bool iterator_ne (iterator_t iter1, iterator_t iter2);
// bool riterator_eq (riterator_t iter1, riterator_t iter2);
// bool riterator_ne (riterator_t iter1, riterator_t iter2);

// iterator_t riterator_reverse (index_t*, riterator_t);

// /* splice */
// /* remove */
// /* remove_if */
// /* unique */
// /* merge */
// /* sort */
// /* reverse */

// /* find */
// /* count */
// /* lower_bound */
// /* upper_bound */
// /* equal_range */

// typedef void (*function_t) (const void*, void*);
// typedef void (*tfunction_t) (void*, void*);

// void index_for_each (index_t*, iterator_t, iterator_t, function_t, void*);
// iterator_t index_find (index_t*, iterator_t, iterator_t, predicate_t, const void*);
// riterator_t index_rfind (index_t*, riterator_t, riterator_t, predicate_t, void*);
// void* index_find_value (index_t*, iterator_t, iterator_t, predicate_t, const void*, iterator_t*);
// void* index_rfind_value (index_t*, riterator_t, riterator_t, predicate_t, void*, riterator_t*);
// void index_remove (index_t*, iterator_t, iterator_t, predicate_t, void*);

// void index_transform (index_t*, iterator_t, iterator_t, tfunction_t, void*);
// void index_insert_unique (index_t*, predicate_t, void*);

#endif /* __table_hh__ */
