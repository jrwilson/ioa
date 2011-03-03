#include <assert.h>
#include <stdlib.h>
#include <ueioa.h>

#include "hashtable.h"

static bid_t
proxy_request_create (bid_t bid,
		      aid_t callback_aid,
		      input_t callback_free_input)
{
  bid_t b = buffer_alloc (sizeof (proxy_request_t));
  proxy_request_t* proxy_request = buffer_write_ptr (b);
  proxy_request->bid = bid;
  if (bid != -1) {
    buffer_add_child (b, bid);
  }
  proxy_request->callback_aid = callback_aid;
  proxy_request->callback_free_input = callback_free_input;
  return b;
}

bid_t
proxy_receipt_create (aid_t proxy_aid,
		      bid_t bid)
{
  assert (proxy_aid != -1);

  bid_t b = buffer_alloc (sizeof (proxy_receipt_t));
  proxy_receipt_t* proxy_receipt = buffer_write_ptr (b);
  proxy_receipt->proxy_aid = proxy_aid;
  proxy_receipt->bid = bid;
  if (bid != -1) {
    buffer_add_child (b, bid);
  }
  return b;
}

typedef struct {
  aid_t* child_aid_ptr;
  descriptor_t* descriptor;
  void* ctor_arg;
  internal_t internal;
  void* param;
} children_key_t;

static bool
children_key_equal (const void* x0,
		    const void* y0)
{
  const children_key_t* x = x0;
  const children_key_t* y = y0;
  return x->child_aid_ptr == y->child_aid_ptr;
}

static size_t
children_key_hash (const void* x0)
{
  const children_key_t* x = x0;
  
  return (size_t)x->child_aid_ptr;
}

typedef struct proxies_key_struct {
  aid_t* proxy_aid_ptr;
  aid_t* source_aid_ptr;
  input_t source_free_input;
  input_t callback;
  bid_t bid;
} proxies_key_t;

static bool
proxies_key_equal (const void* x0,
		   const void* y0)
{
  const proxies_key_t* x = x0;
  const proxies_key_t* y = y0;
  return x->proxy_aid_ptr == y->proxy_aid_ptr;
}

static size_t
proxies_key_hash (const void* x0)
{
  const proxies_key_t* x = x0;
  
  return (size_t)x->proxy_aid_ptr;
}

typedef struct {
  void* param;
  bool declared;
} params_key_t;

static bool
params_key_equal (const void* x0,
		  const void* y0)
{
  const params_key_t* x = x0;
  const params_key_t* y = y0;
  return x->param == y->param;
}

static size_t
params_key_hash (const void* x0)
{
  const params_key_t* x = x0;
  
  return (size_t)x->param;
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
compositions_key_equal (const void* x0,
			const void* y0)
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

typedef enum {
  DEP_START,
  DEP_WAITING,
  DEP_FINISHED,
} dependency_state_t;

typedef struct {
  aid_t* child;
  aid_t* dependent;
  input_t free_input;
  bid_t bid;
  dependency_state_t state;
} dependencies_key_t;

static bool
dependencies_key_equal (const void* x0,
			const void* y0)
{
  const dependencies_key_t* x = x0;
  const dependencies_key_t* y = y0;
  return
    x->child == y->child &&
    x->dependent == y->dependent;
}

static size_t
dependencies_key_hash (const void* x0)
{
  const dependencies_key_t* x = x0;

  return
    (size_t)x->child +
    (size_t)x->dependent;
}

typedef struct {
  input_t input;
  void* in_param;
  bool* flag;
} inputs_key_t;

static bool
inputs_key_equal (const void* x0,
		  const void* y0)
{
  const inputs_key_t* x = x0;
  const inputs_key_t* y = y0;
  return
    x->input == y->input &&
    x->in_param == y->in_param;
}

static size_t
inputs_key_hash (const void* x0)
{
  const inputs_key_t* x = x0;

  return
    (size_t)x->input +
    (size_t)x->in_param;
}

typedef struct {
  output_t output;
  void* out_param;
  bool* flag;
} outputs_key_t;

static bool
outputs_key_equal (const void* x0,
		   const void* y0)
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
  status_t syscall_status;
  status_t proxy_status;
  order_t last_order;
  children_key_t last_child;
  params_key_t last_param;
  proxies_key_t last_proxy;
  compositions_key_t last_composition;
  aid_t self;
  aid_t* self_ptr;
  aid_t* parent_ptr;
  hashtable_t* children;
  hashtable_t* proxies;
  hashtable_t* params;
  hashtable_t* compositions;
  hashtable_t* dependencies;
  hashtable_t* inputs;
  hashtable_t* outputs;
};

manager_t*
manager_create (void)
{
  manager_t* manager = malloc (sizeof (manager_t));
  manager->syscall_status = NORMAL;
  manager->proxy_status = NORMAL;
  manager->self = -1;
  manager->self_ptr = NULL;
  manager->parent_ptr = NULL;
  manager->children = hashtable_create (sizeof (children_key_t), children_key_equal, children_key_hash);
  manager->proxies = hashtable_create (sizeof (proxies_key_t), proxies_key_equal, proxies_key_hash);
  manager->params = hashtable_create (sizeof (params_key_t), params_key_equal, params_key_hash);
  manager->compositions = hashtable_create (sizeof (compositions_key_t), compositions_key_equal, compositions_key_hash);
  manager->dependencies = hashtable_create (sizeof (dependencies_key_t), dependencies_key_equal, dependencies_key_hash);
  manager->inputs = hashtable_create (sizeof (inputs_key_t), inputs_key_equal, inputs_key_hash);
  manager->outputs = hashtable_create (sizeof (outputs_key_t), outputs_key_equal, outputs_key_hash);

  return manager;
}

void
manager_self_set (manager_t* manager,
		  aid_t* self)
{
  assert (manager != NULL);
  manager->self_ptr = self;
}

void
manager_parent_set (manager_t* manager,
		    aid_t* parent)
{
  assert (manager != NULL);
  manager->parent_ptr = parent;
}

void
manager_child_add (manager_t* manager,
		   aid_t* child_aid_ptr,
		   descriptor_t* descriptor,
		   void* ctor_arg,
		   internal_t internal,
		   void* param)
{
  assert (manager != NULL);
  assert (child_aid_ptr != NULL);
  assert (descriptor != NULL);

  /* Check that we are managing the parameter. */
  if (internal != NULL && param != NULL) {
    /* Check that we are managing it. */
    params_key_t key = {
      .param = param,
    };
    assert (hashtable_contains_key (manager->params, &key));
  }

  children_key_t key = {
    .child_aid_ptr = child_aid_ptr,
    .descriptor = descriptor,
    .ctor_arg = ctor_arg,
    .internal = internal,
    .param = param
  };
  assert (!hashtable_contains_key (manager->children, &key));
  hashtable_insert (manager->children, &key);

  /* Clear the automaton. */
  *child_aid_ptr = -1;
}

void
manager_proxy_add (manager_t* manager,
		   aid_t* proxy_aid_ptr,
		   aid_t* source_aid_ptr,
		   input_t source_free_input,
		   input_t callback,
		   bid_t bid)
{
  assert (manager != NULL);
  assert (proxy_aid_ptr != NULL);
  assert (source_aid_ptr != NULL);
  assert (source_free_input != NULL);
  assert (callback != NULL);
  
  proxies_key_t key = {
    .proxy_aid_ptr = proxy_aid_ptr,
    .source_aid_ptr = source_aid_ptr,
    .source_free_input = source_free_input,
    .callback = callback,
    .bid = bid,
  };
  assert (!hashtable_contains_key (manager->proxies, &key));
  hashtable_insert (manager->proxies, &key);

  /* Clear the automaton. */
  *proxy_aid_ptr = -1;

  /* Increment the reference count. */
  if (bid != -1) {
    buffer_incref (bid);
  }
}

void
manager_param_add (manager_t* manager,
		   void* param)
{
  assert (manager != NULL);
  assert (param != NULL);

  params_key_t key = {
    .param = param,
    .declared = false,
  };
  assert (!hashtable_contains_key (manager->params, &key));
  hashtable_insert (manager->params, &key);
}

void
manager_composition_add (manager_t* manager,
			 aid_t* out_automaton,
			 output_t output,
			 void* out_param,
			 aid_t* in_automaton,
			 input_t input,
			 void* in_param)
{
  assert (manager != NULL);
  assert (out_automaton != NULL);
  assert (output != NULL);
  assert (in_automaton != NULL);
  assert (input != NULL);

  if (out_param != NULL) {
    /* Check that we are managing it. */
    params_key_t key = {
      .param = out_param,
    };
    assert (hashtable_contains_key (manager->params, &key));
  }

  if (in_param != NULL) {
    /* Check that we are managing it. */
    params_key_t key = {
      .param = in_param,
    };
    assert (hashtable_contains_key (manager->params, &key));
  }

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

void manager_dependency_add (manager_t* manager,
			     aid_t* child,
			     aid_t* dependent,
			     input_t free_input,
			     bid_t bid)
{
  assert (manager != NULL);
  assert (child != NULL);
  assert (dependent != NULL);
  assert (free_input != NULL);
  assert (bid != -1);

  dependency_state_t state;
  if (*child != -1) {
    state = DEP_FINISHED;
  }
  else if (*dependent != -1) {
    state = DEP_WAITING;
  }
  else {
    state = DEP_START;
  }

  dependencies_key_t key = {
    .child = child,
    .dependent = dependent,
    .free_input = free_input,
    .bid = bid,
    .state = state,
  };
  assert (!hashtable_contains_key (manager->dependencies, &key));
  hashtable_insert (manager->dependencies, &key);

  buffer_incref (bid);
}

void
manager_input_add (manager_t* manager,
		   bool* flag,
		   input_t input,
		   void* in_param)
{
  assert (manager != NULL);
  assert (flag != NULL);
  assert (input != NULL);

  if (in_param != NULL) {
    /* Check that we are managing it. */
    params_key_t key = {
      .param = in_param,
    };
    assert (hashtable_contains_key (manager->params, &key));
  }

  inputs_key_t key = {
    .flag = flag,
    .input = input,
    .in_param = in_param,
  };
  assert (!hashtable_contains_key (manager->inputs, &key));
  hashtable_insert (manager->inputs, &key);

  *flag = false;
}

void
manager_output_add (manager_t* manager,
		    bool* flag,
		    output_t output,
		    void* out_param)
{
  assert (manager != NULL);
  assert (flag != NULL);
  assert (output != NULL);

  if (out_param != NULL) {
    /* Check that we are managing it. */
    params_key_t key = {
      .param = out_param,
    };
    assert (hashtable_contains_key (manager->params, &key));
  }

  outputs_key_t key = {
    .flag = flag,
    .output = output,
    .out_param = out_param,
  };
  assert (!hashtable_contains_key (manager->outputs, &key));
  hashtable_insert (manager->outputs, &key);

  *flag = false;
}

static void
proxy_fire (manager_t* manager)
{
  assert (manager->self != -1);
  if (manager->proxy_status == NORMAL) {
    size_t idx;
    /* Go through the proxies. */
    for (idx = hashtable_first (manager->proxies);
	 idx != hashtable_last (manager->proxies);
	 idx = hashtable_next (manager->proxies, idx)) {
      const proxies_key_t* key = hashtable_key_at (manager->proxies, idx);
      if (*key->proxy_aid_ptr == -1 && *key->source_aid_ptr != -1) {
        /* Request a proxy. */
	bid_t bid = proxy_request_create (key->bid, manager->self, key->callback);
	assert (schedule_free_input (*key->source_aid_ptr, *key->source_free_input, bid) == 0);
	manager->last_proxy = *key;
	manager->proxy_status = OUTSTANDING;
	break;
      }
    }
  }
}

void
manager_apply (manager_t* manager,
	       const receipt_t* receipt)
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
      manager->self = receipt->self_created.self;
      if (manager->self_ptr != NULL) {
	*manager->self_ptr = receipt->self_created.self;
      }
      if (manager->parent_ptr != NULL) {
	*manager->parent_ptr = receipt->self_created.parent;
      }
      something_changed = true;
    }
    break;
  case CHILD_CREATED:
    {
      assert (manager->syscall_status == OUTSTANDING && manager->last_order.type == CREATE);
      *manager->last_child.child_aid_ptr = receipt->child_created.child;
      if (manager->last_child.internal != NULL) {
	assert (schedule_internal (manager->last_child.internal, manager->last_child.param) == 0);
      }
      something_changed = true;
      manager->syscall_status = NORMAL;
    }
    break;
  case BAD_DESCRIPTOR:
    /* TODO */
    assert (0);
    break;
  case DECLARED:
    {
      assert (manager->syscall_status == OUTSTANDING && manager->last_order.type == DECLARE);
      params_key_t* key = hashtable_lookup (manager->params, &manager->last_param);
      assert (key->declared == false);
      key->declared = true;
      something_changed = true;
      manager->syscall_status = NORMAL;
    }
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
      assert (manager->syscall_status == OUTSTANDING && manager->last_order.type == COMPOSE);
      compositions_key_t* key = hashtable_lookup (manager->compositions, &manager->last_composition);
      assert (key->composed == false);
      key->composed = true;
      something_changed = true;
      manager->syscall_status = NORMAL;
    }
    break;
  case INPUT_COMPOSED:
    {
      const inputs_key_t key = {
	.input = receipt->input_composed.input,
	.in_param = receipt->input_composed.in_param,
      };
      inputs_key_t* value = hashtable_lookup (manager->inputs, &key);
      if (value != NULL) {
	*value->flag = true;
      }
    }
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
	assert (schedule_output (receipt->output_composed.output, receipt->output_composed.out_param) == 0);
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
  }

  /* Fire the proxy. */
  proxy_fire (manager);

  if ((manager->syscall_status == NORMAL) && something_changed) {
    assert (schedule_system_output () == 0);
  }
}

static bool
syscall_fire (manager_t* manager)
{
  assert (manager != NULL);

  size_t idx;

  /* Go through the parameters.
     Before the children incase we need to schedule a parameterized output.
     Before the compositions to ensure non-NULL parameters declared. */
  for (idx = hashtable_first (manager->params);
       idx != hashtable_last (manager->params);
       idx = hashtable_next (manager->params, idx)) {
    const params_key_t* key = hashtable_key_at (manager->params, idx);
    if (!key->declared) {
      /* We need to declare a parameter. */
      order_declare_init (&manager->last_order, key->param);
      manager->last_param = *key;
      return true;
    }
  }

  /* Go through the automata.
     Before the compositions because the compositions can depend on these. */
  for (idx = hashtable_first (manager->children);
       idx != hashtable_last (manager->children);
       idx = hashtable_next (manager->children, idx)) {
    const children_key_t* key = hashtable_key_at (manager->children, idx);
    if (*key->child_aid_ptr == -1) {
      /* We need to create an automaton. */
      order_create_init (&manager->last_order, key->descriptor, key->ctor_arg);
      manager->last_child = *key;
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

static void
dependencies_fire (manager_t* manager)
{
  size_t idx;

  /* Go through the dependencies. */
  for (idx = hashtable_first (manager->dependencies);
       idx != hashtable_last (manager->dependencies);
       idx = hashtable_next (manager->dependencies, idx)) {
    dependencies_key_t* key = hashtable_key_at (manager->dependencies, idx);
    switch (key->state) {
    case DEP_START:
      {
	/* Only one thing can happen at a time. */
	if (*key->child != -1) {
	  /* Child created before dependent.  Finished. */
	  key->state = DEP_FINISHED;
	}
	else if (*key->dependent != -1) {
	  /* Dependent created before child.  Wait. */
	  key->state = DEP_WAITING;
	}
      }
      break;
    case DEP_WAITING:
      {
	if (*key->child != -1) {
	  assert (schedule_free_input (*key->dependent, key->free_input, key->bid) == 0);
	  key->state = DEP_FINISHED;
	}
      }
      break;
    case DEP_FINISHED:
      /* Done. */
      break;
    }
  }

}

bid_t
manager_action (manager_t* manager)
{
  assert (manager != NULL);

  proxy_fire (manager);
  dependencies_fire (manager);

  switch (manager->syscall_status) {
  case NORMAL:
    {
      if (syscall_fire (manager)) {
	bid_t bid = buffer_alloc (sizeof (order_t));
	order_t* order = buffer_write_ptr (bid);
	*order = manager->last_order;
	manager->syscall_status = OUTSTANDING;
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

void
manager_proxy_receive (manager_t* manager,
		       const proxy_receipt_t* proxy_receipt)
{
  assert (manager != NULL);
  assert (proxy_receipt != NULL);

  assert (manager->proxy_status == OUTSTANDING);

  *manager->last_proxy.proxy_aid_ptr = proxy_receipt->proxy_aid;
  manager->proxy_status = NORMAL;

  proxy_fire (manager);

  assert (schedule_system_output () == 0);
}
