#include "receipts.h"

#include <assert.h>
#include <pthread.h>
#include "xstdlib.h"
#include "table.h"

struct receipts_struct {
  pthread_rwlock_t lock;
  table_t* table;
  index_t* index;
};

typedef struct {
  aid_t to;
  system_receipt_t receipt;
} receipt_t;

static bool
receipt_to_equal (const void* x0, const void* y0)
{
  const receipt_t* x = x0;
  const receipt_t* y = y0;

  return x->to == y->to;
}

static receipt_t*
receipt_for_to (receipts_t* receipts, aid_t to, iterator_t* ptr)
{
  receipt_t key = {
    .to = to
  };

  return index_find_value (receipts->index,
			   index_begin (receipts->index),
			   index_end (receipts->index),
			   receipt_to_equal,
			   &key,
			   ptr);
}

receipts_t*
receipts_create (void)
{
  receipts_t* receipts = xmalloc (sizeof (receipts_t));
  pthread_rwlock_init (&receipts->lock, NULL);
  receipts->table = table_create (sizeof (receipt_t));
  receipts->index = index_create_list (receipts->table);
  return receipts;
}

void
receipts_destroy (receipts_t* receipts)
{
  assert (receipts != NULL);

  pthread_rwlock_destroy (&receipts->lock);
  table_destroy (receipts->table);
  xfree (receipts);
}

static void
push (receipts_t* receipts, receipt_t* receipt)
{
  pthread_rwlock_wrlock (&receipts->lock);
  index_push_back (receipts->index, receipt);
  pthread_rwlock_unlock (&receipts->lock);
}

void
receipts_push_self_created (receipts_t* receipts, aid_t to, aid_t self, aid_t parent)
{
  assert (receipts != NULL);

  receipt_t receipt;
  receipt.to = to;
  receipt.receipt.type = SYS_SELF_CREATED;
  receipt.receipt.self_created.self = self;
  receipt.receipt.self_created.parent = parent;

  push (receipts, &receipt);
}

void
receipts_push_child_created (receipts_t* receipts, aid_t to, aid_t child)
{ 
  assert (receipts != NULL);

  receipt_t receipt;
  receipt.to = to;
  receipt.receipt.type = SYS_CHILD_CREATED;
  receipt.receipt.child_created.child = child;

  push (receipts, &receipt);
}

void
receipts_push_bad_descriptor (receipts_t* receipts, aid_t to)
{ 
  assert (receipts != NULL);

  receipt_t receipt;
  receipt.to = to;
  receipt.receipt.type = SYS_BAD_DESCRIPTOR;

  push (receipts, &receipt);
}

void
receipts_push_output_dne (receipts_t* receipts, aid_t to)
{ 
  assert (receipts != NULL);

  receipt_t receipt;
  receipt.to = to;
  receipt.receipt.type = SYS_OUTPUT_DNE;

  push (receipts, &receipt);
}

void
receipts_push_input_dne (receipts_t* receipts, aid_t to)
{ 
  assert (receipts != NULL);

  receipt_t receipt;
  receipt.to = to;
  receipt.receipt.type = SYS_INPUT_DNE;

  push (receipts, &receipt);
}

void
receipts_push_output_unavailable (receipts_t* receipts, aid_t to)
{ 
  assert (receipts != NULL);

  receipt_t receipt;
  receipt.to = to;
  receipt.receipt.type = SYS_OUTPUT_UNAVAILABLE;

  push (receipts, &receipt);
}

void
receipts_push_input_unavailable (receipts_t* receipts, aid_t to)
{ 
  assert (receipts != NULL);

  receipt_t receipt;
  receipt.to = to;
  receipt.receipt.type = SYS_INPUT_UNAVAILABLE;

  push (receipts, &receipt);
}

void
receipts_push_composed (receipts_t* receipts, aid_t to)
{ 
  assert (receipts != NULL);

  receipt_t receipt;
  receipt.to = to;
  receipt.receipt.type = SYS_COMPOSED;

  push (receipts, &receipt);
}

void
receipts_push_output_composed (receipts_t* receipts, aid_t to, output_t output)
{ 
  assert (receipts != NULL);

  receipt_t receipt;
  receipt.to = to;
  receipt.receipt.type = SYS_OUTPUT_COMPOSED;
  receipt.receipt.output_composed.output = output;

  push (receipts, &receipt);
}

void
receipts_push_decomposed (receipts_t* receipts, aid_t to)
{ 
  assert (0);
}

/* void */
/* system_receipt_bad_descriptor (system_receipt_t* receipt) */
/* { */
/*   assert (receipt != NULL); */

/*   receipt->type = SYS_BAD_DESCRIPTOR; */
/* } */

void
receipts_push_output_decomposed (receipts_t* receipts, aid_t to, output_t output)
{ 
  assert (0);
}

void
receipts_push_not_composed (receipts_t* receipts, aid_t to)
{ 
  assert (0);
}

void
receipts_push_automaton_dne (receipts_t* receipts, aid_t to)
{ 
  assert (0);
}

void
receipts_push_not_owner (receipts_t* receipts, aid_t to)
{ 
  assert (0);
}

void
receipts_push_child_destroyed (receipts_t* receipts, aid_t to, aid_t child)
{ 
  assert (0);
}

int
receipts_pop (receipts_t* receipts, aid_t to, system_receipt_t* receipt)
{
  assert (receipts != NULL);
  assert (receipt != NULL);

  int retval = -1;

  pthread_rwlock_wrlock (&receipts->lock);
  iterator_t iterator;
  receipt_t* r = receipt_for_to (receipts, to, &iterator);
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
  bool retval = receipt_for_to (receipts, to, NULL) == NULL;
  pthread_rwlock_unlock (&receipts->lock);

  return retval;
}
