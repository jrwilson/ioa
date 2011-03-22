#include "receipts.h"

#include <assert.h>
#include <pthread.h>
#include <stdlib.h>

#include "table.hh"

class ReceiptEntryToEqual {
private:
  const aid_t m_to;
public:
  ReceiptEntryToEqual (const aid_t to) :
    m_to (to) { }
  bool operator() (const receipt_entry_t& receipt_entry) { return m_to == receipt_entry.to; }
};

// static receipt_entry_t*
// receipt_entry_for_to (receipts_t* receipts, aid_t to, iterator_t* ptr)
// {
//   receipt_entry_t key;
//   key.to = to;

//   return (receipt_entry_t*)index_find_value (receipts->index,
// 					     index_begin (receipts->index),
// 					     index_end (receipts->index),
// 					     receipt_entry_to_equal,
// 					     &key,
// 					     ptr);
// }

Receipts::Receipts (void) :
  m_index (m_table)
{
  pthread_rwlock_init (&m_lock, NULL);
}

Receipts::~Receipts (void)
{
  pthread_rwlock_destroy (&m_lock);
}

void
Receipts::push (receipt_entry_t& receipt)
{
  pthread_rwlock_wrlock (&m_lock);
  m_index.push_back (receipt);
  pthread_rwlock_unlock (&m_lock);
}

void
Receipts::push_bad_order ( aid_t to)
{
  receipt_entry_t receipt;
  receipt.to = to;
  receipt.receipt.type = BAD_ORDER;

  push (receipt);
}

void
Receipts::push_self_created ( aid_t to, aid_t self)
{
  receipt_entry_t receipt;
  receipt.to = to;
  receipt.receipt.type = SELF_CREATED;
  receipt.receipt.self_created.self = self;

  push (receipt);
}

void
Receipts::push_child_created ( aid_t to, aid_t child)
{ 
  receipt_entry_t receipt;
  receipt.to = to;
  receipt.receipt.type = CHILD_CREATED;
  receipt.receipt.child_created.child = child;

  push (receipt);
}

void
Receipts::push_bad_descriptor ( aid_t to)
{ 
  receipt_entry_t receipt;
  receipt.to = to;
  receipt.receipt.type = BAD_DESCRIPTOR;

  push (receipt);
}

void
Receipts::push_declared ( aid_t to)
{
  receipt_entry_t receipt;
  receipt.to = to;
  receipt.receipt.type = DECLARED;

  push (receipt);
}

void
Receipts::push_output_dne ( aid_t to)
{ 
  receipt_entry_t receipt;
  receipt.to = to;
  receipt.receipt.type = OUTPUT_DNE;

  push (receipt);
}

void
Receipts::push_input_dne ( aid_t to)
{ 
  receipt_entry_t receipt;
  receipt.to = to;
  receipt.receipt.type = INPUT_DNE;

  push (receipt);
}

void
Receipts::push_output_unavailable ( aid_t to)
{ 
  receipt_entry_t receipt;
  receipt.to = to;
  receipt.receipt.type = OUTPUT_UNAVAILABLE;

  push (receipt);
}

void
Receipts::push_input_unavailable ( aid_t to)
{ 
  receipt_entry_t receipt;
  receipt.to = to;
  receipt.receipt.type = INPUT_UNAVAILABLE;

  push (receipt);
}

void
Receipts::push_composed ( aid_t to)
{ 
  receipt_entry_t receipt;
  receipt.to = to;
  receipt.receipt.type = COMPOSED;

  push (receipt);
}

void
Receipts::push_input_composed ( aid_t to, input_t input, void* in_param)
{ 
  receipt_entry_t receipt;
  receipt.to = to;
  receipt.receipt.type = INPUT_COMPOSED;
  receipt.receipt.input_composed.input = input;
  receipt.receipt.input_composed.in_param = in_param;

  push (receipt);
}

void
Receipts::push_output_composed ( aid_t to, output_t output, void* out_param)
{ 
  receipt_entry_t receipt;
  receipt.to = to;
  receipt.receipt.type = OUTPUT_COMPOSED;
  receipt.receipt.output_composed.output = output;
  receipt.receipt.output_composed.out_param = out_param;

  push (receipt);
}

void
Receipts::push_not_composed ( aid_t to)
{ 
  receipt_entry_t receipt;
  receipt.to = to;
  receipt.receipt.type = NOT_COMPOSED;

  push (receipt);
}

void
Receipts::push_decomposed ( aid_t to, aid_t in_aid, input_t input, void*  in_param)
{ 
  receipt_entry_t receipt;
  receipt.to = to;
  receipt.receipt.type = DECOMPOSED;
  receipt.receipt.decomposed.in_aid = in_aid;
  receipt.receipt.decomposed.input = input;
  receipt.receipt.decomposed.in_param = in_param;

  push (receipt);
}

void
Receipts::push_input_decomposed ( aid_t to, input_t input, void* in_param)
{ 
  receipt_entry_t receipt;
  receipt.to = to;
  receipt.receipt.type = INPUT_DECOMPOSED;
  receipt.receipt.input_decomposed.input = input;
  receipt.receipt.input_decomposed.in_param = in_param;

  push (receipt);
}

void
Receipts::push_output_decomposed ( aid_t to, output_t output, void* out_param)
{ 
  receipt_entry_t receipt;
  receipt.to = to;
  receipt.receipt.type = OUTPUT_DECOMPOSED;
  receipt.receipt.output_decomposed.output = output;
  receipt.receipt.output_decomposed.out_param = out_param;

  push (receipt);
}

void
Receipts::push_rescinded ( aid_t to)
{
  receipt_entry_t receipt;
  receipt.to = to;
  receipt.receipt.type = RESCINDED;

  push (receipt);
}

void
Receipts::push_automaton_dne ( aid_t to)
{ 
  receipt_entry_t receipt;
  receipt.to = to;
  receipt.receipt.type = AUTOMATON_DNE;

  push (receipt);
}

void
Receipts::push_not_owner ( aid_t to)
{ 
  receipt_entry_t receipt;
  receipt.to = to;
  receipt.receipt.type = NOT_OWNER;

  push (receipt);
}

void
Receipts::push_child_destroyed ( aid_t to, aid_t child)
{ 
  receipt_entry_t receipt;
  receipt.to = to;
  receipt.receipt.type = CHILD_DESTROYED;
  receipt.receipt.child_destroyed.child = child;

  push (receipt);
}

int
Receipts::pop (aid_t to, receipt_t* receipt)
{
  assert (receipt != NULL);

  int retval = -1;

  pthread_rwlock_wrlock (&m_lock);
  IndexType::const_iterator pos = std::find_if (m_index.begin (), m_index.end (), ReceiptEntryToEqual (to));
  if (pos != m_index.end ()) {
    *receipt = pos->receipt;
    m_index.erase (pos);
    retval = 0;
  }
  else {
    retval = -1;
  }
  pthread_rwlock_unlock (&m_lock);

  return retval;
}

bool
Receipts::empty ( aid_t to)
{ 
  pthread_rwlock_wrlock (&m_lock);
  IndexType::const_iterator pos = std::find_if (m_index.begin (), m_index.end (), ReceiptEntryToEqual (to));
  bool retval = pos == m_index.end ();
  pthread_rwlock_unlock (&m_lock);

  return retval;
}

void
Receipts::purge_aid ( aid_t to)
{
  pthread_rwlock_wrlock (&m_lock);
  m_index.remove_if (ReceiptEntryToEqual (to));
  pthread_rwlock_unlock (&m_lock);
}
