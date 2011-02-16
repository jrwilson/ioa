#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <ueioa.h>

#include "hashtable.h"

typedef struct {
  aid_t* automaton;
  descriptor_t* descriptor;
} automata_key_t;

static bool
automata_key_equal (const void* x0, const void* y0)
{
  const automata_key_t* x = x0;
  const automata_key_t* y = y0;
  return x->automaton == y->automaton;
}

static size_t
automata_key_hash (const void* x0)
{
  const automata_key_t* x = x0;
  
  return (size_t)x->automaton;
}

typedef struct {
  aid_t* out_automaton;
  output_t output;
  void* out_param;
  aid_t* in_automaton;
  input_t input;
  void* in_param;
  bool composed;
} compositions_key_t;

static bool
compositions_key_equal (const void* x0, const void* y0)
{
  const compositions_key_t* x = x0;
  const compositions_key_t* y = y0;
  return
    x->out_automaton == y->out_automaton &&
    x->output == y->output &&
    x->in_automaton == y->in_automaton &&
    x->input == y->input;
}

static size_t
compositions_key_hash (const void* x0)
{
  const compositions_key_t* x = x0;

  return
    (size_t)x->out_automaton +
    (size_t)x->output +
    (size_t)x->in_automaton +
    (size_t)x->input;
}

typedef struct {
  output_t output;
  void* out_param;
  bool* flag;
} outputs_key_t;

static bool
outputs_key_equal (const void* x0, const void* y0)
{
  const outputs_key_t* x = x0;
  const outputs_key_t* y = y0;
  return
    x->output == y->output &&
    x->out_param == y->out_param;
}

static size_t
outputs_key_hash (const void* x0)
{
  const outputs_key_t* x = x0;

  return
    (size_t)x->output +
    (size_t)x->out_param;
}

typedef enum {
  NORMAL,
  OUTSTANDING
} status_t;

struct manager_struct {
  status_t status;
  order_t last_order;
  automata_key_t last_automaton;
  compositions_key_t last_composition;
  aid_t* self;
  aid_t* parent;
  hashtable_t* automata;
  hashtable_t* compositions;
  hashtable_t* outputs;
};

manager_t*
manager_create (void)
{
  manager_t* manager = malloc (sizeof (manager_t));
  manager->status = NORMAL;
  manager->automata = hashtable_create (sizeof (automata_key_t), automata_key_equal, automata_key_hash);
  manager->compositions = hashtable_create (sizeof (compositions_key_t), compositions_key_equal, compositions_key_hash);
  manager->outputs = hashtable_create (sizeof (outputs_key_t), outputs_key_equal, outputs_key_hash);

  return manager;
}

void
manager_self_set (manager_t* manager, aid_t* self)
{
  assert (manager != NULL);
  manager->self = self;
}

void
manager_parent_set (manager_t* manager, aid_t* parent)
{
  assert (manager != NULL);
  manager->parent = parent;
}

void
manager_automaton_add (manager_t* manager, aid_t* automaton, descriptor_t* descriptor)
{
  assert (manager != NULL);
  assert (automaton != NULL);
  assert (descriptor != NULL);
  assert (descriptor_check (descriptor));

  /* Clear the automaton. */
  *automaton = -1;

  automata_key_t key = {
    .automaton = automaton,
    .descriptor = descriptor,
  };
  assert (!hashtable_contains_key (manager->automata, &key));
  hashtable_insert (manager->automata, &key);
}

void
manager_composition_add (manager_t* manager, aid_t* out_automaton, output_t output, void* out_param, aid_t* in_automaton, input_t input, void* in_param)
{
  assert (manager != NULL);
  assert (out_automaton != NULL);
  assert (output != NULL);
  assert (in_automaton != NULL);
  assert (input != NULL);

  compositions_key_t key = {
    .out_automaton = out_automaton,
    .output = output,
    .out_param = out_param,
    .in_automaton = in_automaton,
    .input = input,
    .in_param = in_param,
    .composed = false,
  };
  assert (!hashtable_contains_key (manager->compositions, &key));
  hashtable_insert (manager->compositions, &key);
}

void
manager_output_add (manager_t* manager, bool* flag, output_t output, void* out_param)
{
  assert (manager != NULL);
  assert (flag != NULL);
  assert (output != NULL);

  *flag = false;

  outputs_key_t key = {
    .flag = flag,
    .output = output,
    .out_param = out_param,
  };
  assert (!hashtable_contains_key (manager->outputs, &key));
  hashtable_insert (manager->outputs, &key);
}

bool
manager_apply (manager_t* manager, const receipt_t* receipt)
{
  assert (manager != NULL);
  assert (receipt != NULL);

  bool something_changed = false;

  switch (receipt->type) {
  case BAD_ORDER:
    /* TODO */
    assert (0);
    break;
  case SELF_CREATED:
    {
      if (manager->self != NULL) {
	*manager->self = receipt->self_created.self;
      }
      if (manager->parent != NULL) {
	*manager->parent = receipt->self_created.parent;
      }
      something_changed = true;
    }
    break;
  case CHILD_CREATED:
    {
      assert (manager->status == OUTSTANDING && manager->last_order.type == CREATE);
      *manager->last_automaton.automaton = receipt->child_created.child;
      something_changed = true;
      manager->status = NORMAL;
    }
    break;
  case BAD_DESCRIPTOR:
    /* TODO */
    assert (0);
    break;
  case DECLARED:
    /* TODO */
    assert (0);
    break;
  case OUTPUT_DNE:
    /* TODO */
    assert (0);
    break;
  case INPUT_DNE:
    /* TODO */
    assert (0);
    break;
  case OUTPUT_UNAVAILABLE:
    /* TODO */
    assert (0);
    break;
  case INPUT_UNAVAILABLE:
    /* TODO */
    assert (0);
    break;
  case COMPOSED:
    {
      assert (manager->status == OUTSTANDING && manager->last_order.type == COMPOSE);
      compositions_key_t* key = hashtable_lookup (manager->compositions, &manager->last_composition);
      key->composed = true;
      something_changed = true;
      manager->status = NORMAL;
    }
    break;
  case INPUT_COMPOSED:
    /* TODO */
    /* assert (0); */
    break;
  case OUTPUT_COMPOSED:
    {
      const outputs_key_t key = {
	.output = receipt->output_composed.output,
	.out_param = receipt->output_composed.out_param,
      };
      outputs_key_t* value = hashtable_lookup (manager->outputs, &key);
      if (value != NULL) {
	*value->flag = true;
	schedule_output (receipt->output_composed.output, receipt->output_composed.out_param);
      }
    }
    break;
  case NOT_COMPOSED:
    /* TODO */
    assert (0);
    break;
  case DECOMPOSED:
    /* TODO */
    assert (0);
    break;
  case INPUT_DECOMPOSED:
    /* TODO */
    assert (0);
    break;
  case OUTPUT_DECOMPOSED:
    /* TODO */
    assert (0);
    break;
  case RESCINDED:
    /* TODO */
    assert (0);
    break;
  case AUTOMATON_DNE:
    /* TODO */
    assert (0);
    break;
  case NOT_OWNER:
    /* TODO */
    assert (0);
    break;
  case CHILD_DESTROYED:
    /* TODO */
    assert (0);
    break;
  case ALARM:
    /* TODO */
    assert (0);
    break;
  case WRITE_READY:
    /* TODO */
    assert (0);
    break;
  case READ_READY:
    /* TODO */
    assert (0);
    break;
  }

  return (manager->status == NORMAL) && something_changed;
}

static bool
fire (manager_t* manager)
{
  assert (manager != NULL);

  size_t idx;

  /* Go through the automata. */
  for (idx = hashtable_first (manager->automata);
       idx != hashtable_last (manager->automata);
       idx = hashtable_next (manager->automata, idx)) {
    const automata_key_t* key = hashtable_key_at (manager->automata, idx);
    if (*key->automaton == -1) {
      /* We need to create an automaton. */
      const automata_key_t* key = hashtable_key_at (manager->automata, idx);
      order_create_init (&manager->last_order, key->descriptor);
      manager->last_automaton = *key;
      return true;
    }
  }

  /* Go through the compositions. */
  for (idx = hashtable_first (manager->compositions);
       idx != hashtable_last (manager->compositions);
       idx = hashtable_next (manager->compositions, idx)) {
    const compositions_key_t* key = hashtable_key_at (manager->compositions, idx);
    if (*key->out_automaton != -1 &&
	*key->in_automaton != -1 &&
	!key->composed) {
      /* We need to compose. */
      order_compose_init (&manager->last_order, *key->out_automaton, key->output, key->out_param, *key->in_automaton, key->input, key->in_param);
      manager->last_composition = *key;
      return true;
    }
  }

  return false;
}

bid_t
manager_action (manager_t* manager)
{
  assert (manager != NULL);

  switch (manager->status) {
  case NORMAL:
    {
      if (fire (manager)) {
	bid_t bid = buffer_alloc (sizeof (order_t));
	order_t* order = buffer_write_ptr (bid);
	*order = manager->last_order;
	manager->status = OUTSTANDING;
	return bid;
      }
      else {
	return -1;
      }
    }
    break;
  case OUTSTANDING:
    return -1;
    break;
  }

  /* Not reached. */
  assert (0);
  return -1;
}
