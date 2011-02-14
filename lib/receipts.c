#include "receipts.h"

#include <assert.h>
#include <pthread.h>
#include <stdlib.h>

#include "table.h"

struct receipts_struct {
  pthread_rwlock_t lock;
  table_t* table;
  index_t* index;
};

typedef struct {
  aid_t to;
  receipt_t receipt;
} receipt_entry_t;

static bool
receipt_entry_to_equal (const void* x0, const void* y0)
{
  const receipt_entry_t* x = x0;
  const receipt_entry_t* y = y0;

  return x->to == y->to;
}

static receipt_entry_t*
receipt_entry_for_to (receipts_t* receipts, aid_t to, iterator_t* ptr)
{
  receipt_entry_t key = {
    .to = to
  };

  return index_find_value (receipts->index,
			   index_begin (receipts->index),
			   index_end (receipts->index),
			   receipt_entry_to_equal,
			   &key,
			   ptr);
}

receipts_t*
receipts_create (void)
{
  receipts_t* receipts = malloc (sizeof (receipts_t));
  pthread_rwlock_init (&receipts->lock, NULL);
  receipts->table = table_create (sizeof (receipt_entry_t));
  receipts->index = index_create_list (receipts->table);
  return receipts;
}

void
receipts_destroy (receipts_t* receipts)
{
  assert (receipts != NULL);

  pthread_rwlock_destroy (&receipts->lock);
  table_destroy (receipts->table);
  free (receipts);
}

static void
push (receipts_t* receipts, receipt_entry_t* receipt)
{
  assert (receipts != NULL);
  assert (receipt != NULL);
  pthread_rwlock_wrlock (&receipts->lock);
  index_push_back (receipts->index, receipt);
  pthread_rwlock_unlock (&receipts->lock);
}

void
receipts_push_bad_order (receipts_t* receipts, aid_t to)
{
  assert (receipts != NULL);

  receipt_entry_t receipt;
  receipt.to = to;
  receipt.receipt.type = BAD_ORDER;

  push (receipts, &receipt);
}

void
receipts_push_self_created (receipts_t* receipts, aid_t to, aid_t self, aid_t parent)
{
  assert (receipts != NULL);

  receipt_entry_t receipt;
  receipt.to = to;
  receipt.receipt.type = SELF_CREATED;
  receipt.receipt.self_created.self = self;
  receipt.receipt.self_created.parent = parent;

  push (receipts, &receipt);
}

void
receipts_push_child_created (receipts_t* receipts, aid_t to, aid_t child)
{ 
  assert (receipts != NULL);

  receipt_entry_t receipt;
  receipt.to = to;
  receipt.receipt.type = CHILD_CREATED;
  receipt.receipt.child_created.child = child;

  push (receipts, &receipt);
}

void
receipts_push_bad_descriptor (receipts_t* receipts, aid_t to)
{ 
  assert (receipts != NULL);

  receipt_entry_t receipt;
  receipt.to = to;
  receipt.receipt.type = BAD_DESCRIPTOR;

  push (receipts, &receipt);
}

void
receipts_push_declared (receipts_t* receipts, aid_t to)
{
 assert (receipts != NULL);

  receipt_entry_t receipt;
  receipt.to = to;
  receipt.receipt.type = DECLARED;

  push (receipts, &receipt);
}

void
receipts_push_output_dne (receipts_t* receipts, aid_t to)
{ 
  assert (receipts != NULL);

  receipt_entry_t receipt;
  receipt.to = to;
  receipt.receipt.type = OUTPUT_DNE;

  push (receipts, &receipt);
}

void
receipts_push_input_dne (receipts_t* receipts, aid_t to)
{ 
  assert (receipts != NULL);

  receipt_entry_t receipt;
  receipt.to = to;
  receipt.receipt.type = INPUT_DNE;

  push (receipts, &receipt);
}

void
receipts_push_output_unavailable (receipts_t* receipts, aid_t to)
{ 
  assert (receipts != NULL);

  receipt_entry_t receipt;
  receipt.to = to;
  receipt.receipt.type = OUTPUT_UNAVAILABLE;

  push (receipts, &receipt);
}

void
receipts_push_input_unavailable (receipts_t* receipts, aid_t to)
{ 
  assert (receipts != NULL);

  receipt_entry_t receipt;
  receipt.to = to;
  receipt.receipt.type = INPUT_UNAVAILABLE;

  push (receipts, &receipt);
}

void
receipts_push_composed (receipts_t* receipts, aid_t to)
{ 
  assert (receipts != NULL);

  receipt_entry_t receipt;
  receipt.to = to;
  receipt.receipt.type = COMPOSED;

  push (receipts, &receipt);
}

void
receipts_push_input_composed (receipts_t* receipts, aid_t to, input_t input, void* in_param)
{ 
  assert (receipts != NULL);

  receipt_entry_t receipt;
  receipt.to = to;
  receipt.receipt.type = INPUT_COMPOSED;
  receipt.receipt.input_composed.input = input;
  receipt.receipt.input_composed.in_param = in_param;

  push (receipts, &receipt);
}

void
receipts_push_output_composed (receipts_t* receipts, aid_t to, output_t output, void* out_param)
{ 
  assert (receipts != NULL);

  receipt_entry_t receipt;
  receipt.to = to;
  receipt.receipt.type = OUTPUT_COMPOSED;
  receipt.receipt.output_composed.output = output;
  receipt.receipt.output_composed.out_param = out_param;

  push (receipts, &receipt);
}

void
receipts_push_not_composed (receipts_t* receipts, aid_t to)
{ 
  assert (receipts != NULL);

  receipt_entry_t receipt;
  receipt.to = to;
  receipt.receipt.type = NOT_COMPOSED;

  push (receipts, &receipt);
}

void
receipts_push_decomposed (receipts_t* receipts, aid_t to, aid_t in_aid, input_t input, void*  in_param)
{ 
  assert (receipts != NULL);

  receipt_entry_t receipt;
  receipt.to = to;
  receipt.receipt.type = DECOMPOSED;
  receipt.receipt.decomposed.in_aid = in_aid;
  receipt.receipt.decomposed.input = input;
  receipt.receipt.decomposed.in_param = in_param;

  push (receipts, &receipt);
}

void
receipts_push_input_decomposed (receipts_t* receipts, aid_t to, input_t input, void* in_param)
{ 
  assert (receipts != NULL);

  receipt_entry_t receipt;
  receipt.to = to;
  receipt.receipt.type = INPUT_DECOMPOSED;
  receipt.receipt.input_decomposed.input = input;
  receipt.receipt.input_decomposed.in_param = in_param;

  push (receipts, &receipt);
}

void
receipts_push_output_decomposed (receipts_t* receipts, aid_t to, output_t output, void* out_param)
{ 
  assert (receipts != NULL);

  receipt_entry_t receipt;
  receipt.to = to;
  receipt.receipt.type = OUTPUT_DECOMPOSED;
  receipt.receipt.output_decomposed.output = output;
  receipt.receipt.output_decomposed.out_param = out_param;

  push (receipts, &receipt);
}

void
receipts_push_rescinded (receipts_t* receipts, aid_t to)
{
 assert (receipts != NULL);

  receipt_entry_t receipt;
  receipt.to = to;
  receipt.receipt.type = RESCINDED;

  push (receipts, &receipt);
}

void
receipts_push_automaton_dne (receipts_t* receipts, aid_t to)
{ 
  assert (receipts != NULL);

  receipt_entry_t receipt;
  receipt.to = to;
  receipt.receipt.type = AUTOMATON_DNE;

  push (receipts, &receipt);
}

void
receipts_push_not_owner (receipts_t* receipts, aid_t to)
{ 
  assert (receipts != NULL);

  receipt_entry_t receipt;
  receipt.to = to;
  receipt.receipt.type = NOT_OWNER;

  push (receipts, &receipt);
}

void
receipts_push_child_destroyed (receipts_t* receipts, aid_t to, aid_t child)
{ 
  assert (receipts != NULL);

  receipt_entry_t receipt;
  receipt.to = to;
  receipt.receipt.type = CHILD_DESTROYED;
  receipt.receipt.child_destroyed.child = child;

  push (receipts, &receipt);
}

void
receipts_push_wakeup (receipts_t* receipts, aid_t to)
{
  assert (receipts != NULL);

  receipt_entry_t receipt;
  receipt.to = to;
  receipt.receipt.type = WAKEUP;

  push (receipts, &receipt);
}

int
receipts_pop (receipts_t* receipts, aid_t to, receipt_t* receipt)
{
  assert (receipts != NULL);
  assert (receipt != NULL);

  int retval = -1;

  pthread_rwlock_wrlock (&receipts->lock);
  iterator_t iterator;
  receipt_entry_t* r = receipt_entry_for_to (receipts, to, &iterator);
  if (r != NULL) {
    *receipt = r->receipt;
    index_erase (receipts->index, iterator);
    retval = 0;
  }
  else {
    retval = -1;
  }
  pthread_rwlock_unlock (&receipts->lock);

  return retval;
}

bool
receipts_empty (receipts_t* receipts, aid_t to)
{ 
  assert (receipts != NULL);

  pthread_rwlock_wrlock (&receipts->lock);
  bool retval = receipt_entry_for_to (receipts, to, NULL) == NULL;
  pthread_rwlock_unlock (&receipts->lock);

  return retval;
}

void
receipts_purge_aid (receipts_t* receipts, aid_t to)
{
  assert (receipts != NULL);

  receipt_entry_t key = {
    .to = to
  };

  pthread_rwlock_wrlock (&receipts->lock);
  index_remove (receipts->index,
		index_begin (receipts->index),
		index_end (receipts->index),
		receipt_entry_to_equal,
		&key);
  pthread_rwlock_unlock (&receipts->lock);
}
