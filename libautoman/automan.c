#include <automan.h>

#include <assert.h>
#include <stdlib.h>

#include <table.h>

/******************************************************************************************
 * TYPES
 ******************************************************************************************/

typedef enum {
  NORMAL,
  OUTSTANDING
} status_t;

typedef struct {
  order_type_t order_type;
  union {
    struct {
      aid_t* aid_ptr;
      const descriptor_t* descriptor;
      const void* ctor_arg;
      internal_t internal;
      void* param;
    } create;
    struct {
      void* param;
      internal_t internal;
      bool declared;
    } declare;
    struct {
      int a;
    } compose;
    struct {
      int a;
    } decompose;
    struct {
      void* param;
    } rescind;
    struct {
      aid_t* aid_ptr;
    } destroy;
  };
} action_item_t;

typedef struct {
  aid_t* aid_ptr;
} aid_ptr_key_t;

typedef struct {
  void* param;
} param_key_t;

struct automan_struct {
  aid_t* self_ptr;
  status_t syscall_status;
  order_t last_order;
  action_item_t last_action;
  table_t* ai_table;
  index_t* ai_index;

/*   status_t proxy_status; */
/*   create_key_t last_create; */
/*   params_key_t last_param; */
/*   proxies_key_t last_proxy; */
/*   compositions_key_t last_composition; */
/*   hashtable_t* create; */
/*   hashtable_t* proxies; */
/*   hashtable_t* params; */
/*   hashtable_t* compositions; */
/*   hashtable_t* dependencies; */
/*   hashtable_t* inputs; */
/*   hashtable_t* outputs; */
};

/******************************************************************************************
 * PRIVATE FUNCTIONS
 ******************************************************************************************/

static bool
action_item_equal (const void* x0, void* y0)
{
  const action_item_t* x = x0;
  const action_item_t* y = y0;

  if (x->order_type != y->order_type) {
    return false;
  }

  switch (x->order_type) {
  case CREATE:
    return x->create.aid_ptr == y->create.aid_ptr;
  case DECLARE:
    return x->declare.param == y->declare.param;
  case COMPOSE:
    assert (0);
  case DECOMPOSE:
    assert (0);
  case RESCIND:
    return x->rescind.param == y->rescind.param;
  case DESTROY:
    return x->destroy.aid_ptr == y->destroy.aid_ptr;
  }
  /* Not reached. */
  assert (0);
  return false;
}

static bool
aid_ptr_equal (const void* x0, void* y0)
{
  const action_item_t* action_item = x0;
  const aid_ptr_key_t* aid_ptr = y0;

  switch (action_item->order_type) {
  case DECLARE:
  case COMPOSE:
  case DECOMPOSE:
  case RESCIND:
    return false;
  case CREATE:
    return action_item->create.aid_ptr == aid_ptr->aid_ptr;
  case DESTROY:
    return action_item->destroy.aid_ptr == aid_ptr->aid_ptr;
  }
  /* Not reached. */
  assert (0);
  return false;
}

static bool
param_equal (const void* x0, void* y0)
{
  const action_item_t* action_item = x0;
  const param_key_t* param = y0;

  switch (action_item->order_type) {
  case CREATE:
  case COMPOSE:
  case DECOMPOSE:
  case DESTROY:
    return false;
  case DECLARE:
    return action_item->declare.param == param->param;
  case RESCIND:
    return action_item->rescind.param == param->param;
  }
  /* Not reached. */
  assert (0);
  return false;
}

static bool
check_child_create (automan_t* automan,
		    aid_t* aid_ptr)
{
  aid_ptr_key_t key = {
    .aid_ptr = aid_ptr,
  };
  
  /* Find creates and destroys starting at the back. */
  action_item_t* action_item = index_rfind_value (automan->ai_index,
						  index_rbegin (automan->ai_index),
						  index_rend (automan->ai_index),
						  aid_ptr_equal,
						  &key,
						  NULL);

  if (action_item != NULL) {
    /* Found one.  True if created. */
    return action_item->order_type == CREATE;
  }
  else {
    /* Not created. */
    return false;
  }
}

static bool
check_child_destroy (automan_t* automan,
		     aid_t* aid_ptr)
{
  aid_ptr_key_t key = {
    .aid_ptr = aid_ptr,
  };
  
  /* Find creates and destroys starting at the back. */
  action_item_t* action_item = index_rfind_value (automan->ai_index,
						  index_rbegin (automan->ai_index),
						  index_rend (automan->ai_index),
						  aid_ptr_equal,
						  &key,
						  NULL);

  if (action_item != NULL) {
    /* Found one.  True if destroyed. */
    return action_item->order_type == DESTROY;
  }
  else {
    /* Destroyed. */
    return true;
  }
}

static bool
check_param_declare (automan_t* automan,
		     void* param)
{
  param_key_t key = {
    .param = param,
  };
  
  /* Find declares and rescinds starting at the back. */
  action_item_t* action_item = index_rfind_value (automan->ai_index,
						  index_rbegin (automan->ai_index),
						  index_rend (automan->ai_index),
						  param_equal,
						  &key,
						  NULL);

  if (action_item != NULL) {
    /* Found one.  True if declared. */
    return action_item->order_type == DECLARE;
  }
  else {
    /* Not declared. */
    return false;
  }
}

static bool
check_param_rescind (automan_t* automan,
		     void* param)
{
  param_key_t key = {
    .param = param,
  };
  
  /* Find declares and rescinds starting at the back. */
  action_item_t* action_item = index_rfind_value (automan->ai_index,
						  index_rbegin (automan->ai_index),
						  index_rend (automan->ai_index),
						  param_equal,
						  &key,
						  NULL);

  if (action_item != NULL) {
    /* Found one.  True if rescinded. */
    return action_item->order_type == RESCIND;
  }
  else {
    /* Rescinded. */
    return true;
  }
}

static void
push_child_create (automan_t* automan,
		   aid_t* aid_ptr,
		   const descriptor_t* descriptor,
		   const void* ctor_arg,
		   internal_t internal,
		   void* param)
{
  action_item_t action_item;
  action_item.order_type = CREATE;
  action_item.create.aid_ptr = aid_ptr;
  action_item.create.descriptor = descriptor;
  action_item.create.ctor_arg = ctor_arg;
  action_item.create.internal = internal;
  action_item.create.param = param;

  index_push_back (automan->ai_index, &action_item);
}

static void
push_param_declare (automan_t* automan,
		    void* param,
		    internal_t internal)
{
  action_item_t action_item;
  action_item.order_type = DECLARE;
  action_item.declare.param = param;
  action_item.declare.internal = internal;
  action_item.declare.declared = false;

  index_push_back (automan->ai_index, &action_item);
}

static bool
param_declared (automan_t* automan,
		void* param)
{
  if (param == NULL) {
    return true;
  }

  action_item_t key;
  key.order_type = DECLARE;
  key.declare.param = param;

  action_item_t* action_item = index_find_value (automan->ai_index,
						 index_begin (automan->ai_index),
						 index_end (automan->ai_index),
						 action_item_equal,
						 &key,
						 NULL);

  if (action_item != NULL) {
    return action_item->declare.declared;
  }
  else {
    /* Doesn't exist. */
    return false;
  }
  
}

static action_item_t*
find_first_action_item (automan_t* automan, action_item_t* key)
{
  return index_find_value (automan->ai_index,
			   index_begin (automan->ai_index),
			   index_end (automan->ai_index),
			   action_item_equal,
			   key,
			   NULL);
}

static bool
process_action_items (automan_t* automan)
{
  assert (automan != NULL);

  iterator_t iterator;
  for (iterator = index_begin (automan->ai_index);
       iterator_ne (iterator, index_end (automan->ai_index));
       iterator = index_advance (automan->ai_index, iterator)) {
    action_item_t* action_item = index_value (automan->ai_index, iterator);
    switch (action_item->order_type) {
    case CREATE:
      {
	if (*action_item->create.aid_ptr == -1 &&
	    param_declared (automan, action_item->create.param)) {
	  /* Create an automaton. */
	  order_create_init (&automan->last_order,
			     action_item->create.descriptor,
			     action_item->create.ctor_arg);
	  automan->last_action = *action_item;
	  return true;
	}
      }
      break;
    case DECLARE:
      {
	if (!action_item->declare.declared) {
	  /* We need to declare a parameter. */
	  order_declare_init (&automan->last_order, action_item->declare.param);
	  automan->last_action = *action_item;
	  return true;
	}
      }
      break;
    case COMPOSE:
      assert (0);
      break;
    case DECOMPOSE:
      assert (0);
      break;
    case RESCIND:
      assert (0);
      break;
    case DESTROY:
      assert (0);
      break;
    }
  }

  /* /\* Go through the compositions. *\/ */
  /* for (idx = hashtable_first (automan->compositions); */
  /*      idx != hashtable_last (automan->compositions); */
  /*      idx = hashtable_next (automan->compositions, idx)) { */
  /*   const compositions_key_t* key = hashtable_key_at (automan->compositions, idx); */
  /*   if (*key->out_automaton != -1 && */
  /* 	*key->in_automaton != -1 && */
  /* 	!key->composed) { */
  /*     /\* We need to compose. *\/ */
  /*     order_compose_init (&automan->last_order, *key->out_automaton, key->output, key->out_param, *key->in_automaton, key->input, key->in_param); */
  /*     automan->last_composition = *key; */
  /*     return true; */
  /*   } */
  /* } */

  return false;
}

/******************************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************************/

automan_t*
automan_create (aid_t* self_ptr)
{
  assert (self_ptr != NULL);

  automan_t* automan = malloc (sizeof (automan_t));

  automan->self_ptr = self_ptr;
  *self_ptr = -1;
  automan->syscall_status = NORMAL;
  automan->ai_table = table_create (sizeof (action_item_t));
  automan->ai_index = index_create_list (automan->ai_table);
  /* automan->proxy_status = NORMAL; */
  /* automan->create = hashtable_create (sizeof (create_key_t), create_key_equal, create_key_hash); */
  /* automan->proxies = hashtable_create (sizeof (proxies_key_t), proxies_key_equal, proxies_key_hash); */
  /* automan->params = hashtable_create (sizeof (params_key_t), params_key_equal, params_key_hash); */
  /* automan->compositions = hashtable_create (sizeof (compositions_key_t), compositions_key_equal, compositions_key_hash); */
  /* automan->dependencies = hashtable_create (sizeof (dependencies_key_t), dependencies_key_equal, dependencies_key_hash); */
  /* automan->inputs = hashtable_create (sizeof (inputs_key_t), inputs_key_equal, inputs_key_hash); */
  /* automan->outputs = hashtable_create (sizeof (outputs_key_t), outputs_key_equal, outputs_key_hash); */

  return automan;
}

void
automan_apply (automan_t* automan,
	       const receipt_t* receipt)
{
  assert (automan != NULL);
  assert (receipt != NULL);

  bool something_changed = false;

  switch (receipt->type) {
  case BAD_ORDER:
    /* TODO */
    assert (0);
    break;
  case SELF_CREATED:
    {
      *automan->self_ptr = receipt->self_created.self;
      something_changed = true;
    }
    break;
  case CHILD_CREATED:
    {
      assert (automan->syscall_status == OUTSTANDING && automan->last_order.type == CREATE);
      *automan->last_action.create.aid_ptr = receipt->child_created.child;
      if (automan->last_action.create.internal != NULL) {
    	assert (schedule_internal (automan->last_action.create.internal,
				   automan->last_action.create.param) == 0);
      }
      something_changed = true;
      automan->syscall_status = NORMAL;
    }
    break;
  case BAD_DESCRIPTOR:
    /* TODO */
    assert (0);
    break;
  case DECLARED:
    {
      assert (automan->syscall_status == OUTSTANDING && automan->last_order.type == DECLARE);
      action_item_t* action_item = find_first_action_item (automan, &automan->last_action);
      assert (action_item->declare.declared == false);
      action_item->declare.declared = true;
      if (automan->last_action.declare.internal != NULL) {
    	assert (schedule_internal (automan->last_action.declare.internal,
				   automan->last_action.declare.param) == 0);
      }
      something_changed = true;
      automan->syscall_status = NORMAL;
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
    assert (0);
    /* { */
    /*   assert (automan->syscall_status == OUTSTANDING && automan->last_order.type == COMPOSE); */
    /*   compositions_key_t* key = hashtable_lookup (automan->compositions, &automan->last_composition); */
    /*   assert (key->composed == false); */
    /*   key->composed = true; */
    /*   something_changed = true; */
    /*   automan->syscall_status = NORMAL; */
    /* } */
    break;
  case INPUT_COMPOSED:
    assert (0);
    /* { */
    /*   const inputs_key_t key = { */
    /* 	.input = receipt->input_composed.input, */
    /* 	.in_param = receipt->input_composed.in_param, */
    /*   }; */
    /*   inputs_key_t* value = hashtable_lookup (automan->inputs, &key); */
    /*   if (value != NULL) { */
    /* 	*value->flag = true; */
    /*   } */
    /* } */
    break;
  case OUTPUT_COMPOSED:
    assert (0);
    /* { */
    /*   const outputs_key_t key = { */
    /* 	.output = receipt->output_composed.output, */
    /* 	.out_param = receipt->output_composed.out_param, */
    /*   }; */
    /*   outputs_key_t* value = hashtable_lookup (automan->outputs, &key); */
    /*   if (value != NULL) { */
    /* 	*value->flag = true; */
    /* 	assert (schedule_output (receipt->output_composed.output, receipt->output_composed.out_param) == 0); */
    /*   } */
    /* } */
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

  /* /\* Fire the proxy. *\/ */
  /* proxy_fire (automan); */

  if ((automan->syscall_status == NORMAL) && something_changed) {
    assert (schedule_system_output () == 0);
  }
}

bid_t
automan_action (automan_t* automan)
{
  assert (automan != NULL);

/*   proxy_fire (automan); */
/*   dependencies_fire (automan); */

  switch (automan->syscall_status) {
  case NORMAL:
    {
      if (process_action_items (automan)) {
	bid_t bid = buffer_alloc (sizeof (order_t));
	order_t* order = buffer_write_ptr (bid);
	*order = automan->last_order;
	automan->syscall_status = OUTSTANDING;
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

int
automan_child_create (automan_t* automan,
		      aid_t* child_aid_ptr,
		      const descriptor_t* descriptor,
		      const void* ctor_arg,
		      internal_t internal,
		      void* param)
{
  assert (automan != NULL);
  assert (child_aid_ptr != NULL);
  assert (descriptor != NULL);
  assert (param == NULL || internal != NULL);

  if (internal != NULL && param != NULL) {
    /* Check that the parameter will be declared. */
    if (!check_param_declare (automan, param)) {
      return -1;
    }
  }

  if (!check_child_destroy (automan, child_aid_ptr)) {
    /* Check that the aid_ptr will be destroyed. */
    return -1;
  }

  /* Clear the automaton. */
  *child_aid_ptr = -1;

  /* Add an action item. */
  push_child_create (automan,
		     child_aid_ptr,
		     descriptor,
		     ctor_arg,
		     internal,
		     param);

  return 0;
}

int
automan_param_declare (automan_t* automan,
		       void* param,
		       internal_t internal)
{
  assert (automan != NULL);
  assert (param != NULL);

  if (!check_param_rescind (automan, param)) {
    /* Check that the param will be rescinded. */
    return -1;
  }

  /* Add an action item. */
  push_param_declare (automan,
		      param,
		      internal);

  return 0;
}































/* void */
/* automan_input_add (automan_t* automan, */
/* 		   bool* flag, */
/* 		   input_t input, */
/* 		   void* in_param, */
/* 		   internal_t internal) */
/* { */
/*   assert (automan != NULL); */
/*   assert (flag != NULL); */
/*   assert (input != NULL); */

/*   if (in_param != NULL) { */
/*     /\* Check that we are managing it. *\/ */
/*     params_key_t key = { */
/*       .param = in_param, */
/*     }; */
/*     assert (hashtable_contains_key (automan->params, &key)); */
/*   } */

/*   inputs_key_t key = { */
/*     .flag = flag, */
/*     .input = input, */
/*     .in_param = in_param, */
/*   }; */
/*   assert (!hashtable_contains_key (automan->inputs, &key)); */
/*   hashtable_insert (automan->inputs, &key); */

/*   *flag = false; */
/* } */


















/* void */
/* automan_child_remove (automan_t* automan, */
/* 		      aid_t* child_aid_ptr) */
/* { */
/*   assert (automan != NULL); */
/*   assert (child_aid_ptr != NULL); */

/*   create_key_t key = { */
/*     .child_aid_ptr = child_aid_ptr, */
/*   }; */
/*   create_key_t* k = hashtable_lookup (automan->create, &key); */
/*   assert (k != NULL); */
/*   assert (0); */
/* } */

/* void */
/* automan_proxy_add (automan_t* automan, */
/* 		   aid_t* proxy_aid_ptr, */
/* 		   aid_t* source_aid_ptr, */
/* 		   input_t source_free_input, */
/* 		   input_t callback, */
/* 		   bid_t bid) */
/* { */
/*   assert (automan != NULL); */
/*   assert (proxy_aid_ptr != NULL); */
/*   assert (source_aid_ptr != NULL); */
/*   assert (source_free_input != NULL); */
/*   assert (callback != NULL); */
  
/*   proxies_key_t key = { */
/*     .proxy_aid_ptr = proxy_aid_ptr, */
/*     .source_aid_ptr = source_aid_ptr, */
/*     .source_free_input = source_free_input, */
/*     .callback = callback, */
/*     .bid = bid, */
/*   }; */
/*   assert (!hashtable_contains_key (automan->proxies, &key)); */
/*   hashtable_insert (automan->proxies, &key); */

/*   /\* Clear the automaton. *\/ */
/*   *proxy_aid_ptr = -1; */

/*   /\* Increment the reference count. *\/ */
/*   if (bid != -1) { */
/*     buffer_incref (bid); */
/*   } */
/* } */

/* void */
/* automan_param_add (automan_t* automan, */
/* 		   void* param) */
/* { */
/*   assert (automan != NULL); */
/*   assert (param != NULL); */

/*   params_key_t key = { */
/*     .param = param, */
/*     .declared = false, */
/*   }; */
/*   assert (!hashtable_contains_key (automan->params, &key)); */
/*   hashtable_insert (automan->params, &key); */
/* } */

/* void */
/* automan_composition_add (automan_t* automan, */
/* 			 aid_t* out_automaton, */
/* 			 output_t output, */
/* 			 void* out_param, */
/* 			 aid_t* in_automaton, */
/* 			 input_t input, */
/* 			 void* in_param) */
/* { */
/*   assert (automan != NULL); */
/*   assert (out_automaton != NULL); */
/*   assert (output != NULL); */
/*   assert (in_automaton != NULL); */
/*   assert (input != NULL); */

/*   if (out_param != NULL) { */
/*     /\* Check that we are managing it. *\/ */
/*     params_key_t key = { */
/*       .param = out_param, */
/*     }; */
/*     assert (hashtable_contains_key (automan->params, &key)); */
/*   } */

/*   if (in_param != NULL) { */
/*     /\* Check that we are managing it. *\/ */
/*     params_key_t key = { */
/*       .param = in_param, */
/*     }; */
/*     assert (hashtable_contains_key (automan->params, &key)); */
/*   } */

/*   compositions_key_t key = { */
/*     .out_automaton = out_automaton, */
/*     .output = output, */
/*     .out_param = out_param, */
/*     .in_automaton = in_automaton, */
/*     .input = input, */
/*     .in_param = in_param, */
/*     .composed = false, */
/*   }; */
/*   assert (!hashtable_contains_key (automan->compositions, &key)); */
/*   hashtable_insert (automan->compositions, &key); */
/* } */

/* void */
/* automan_dependency_add (automan_t* automan, */
/* 			aid_t* child, */
/* 			aid_t* dependent, */
/* 			input_t free_input, */
/* 			bid_t bid) */
/* { */
/*   assert (automan != NULL); */
/*   assert (child != NULL); */
/*   assert (dependent != NULL); */
/*   assert (free_input != NULL); */
/*   assert (bid != -1); */

/*   dependency_state_t state; */
/*   if (*child != -1) { */
/*     state = DEP_FINISHED; */
/*   } */
/*   else if (*dependent != -1) { */
/*     state = DEP_WAITING; */
/*   } */
/*   else { */
/*     state = DEP_START; */
/*   } */

/*   dependencies_key_t key = { */
/*     .child = child, */
/*     .dependent = dependent, */
/*     .free_input = free_input, */
/*     .bid = bid, */
/*     .state = state, */
/*   }; */
/*   assert (!hashtable_contains_key (automan->dependencies, &key)); */
/*   hashtable_insert (automan->dependencies, &key); */

/*   buffer_incref (bid); */
/* } */

/* void */
/* automan_dependency_remove (automan_t* automan, */
/* 			   aid_t* child, */
/* 			   aid_t* dependent) */
/* { */
/*   assert (automan != NULL); */
/*   assert (child != NULL); */
/*   assert (dependent != NULL); */

/*   dependencies_key_t key = { */
/*     .child = child, */
/*     .dependent = dependent, */
/*   }; */
/*   dependencies_key_t* k = hashtable_lookup (automan->dependencies, &key); */
/*   assert (k != NULL); */
/*   buffer_decref (k->bid); */
/*   hashtable_remove (automan->dependencies, &key); */
/* } */


/* void */
/* automan_output_add (automan_t* automan, */
/* 		    bool* flag, */
/* 		    output_t output, */
/* 		    void* out_param) */
/* { */
/*   assert (automan != NULL); */
/*   assert (flag != NULL); */
/*   assert (output != NULL); */

/*   if (out_param != NULL) { */
/*     /\* Check that we are managing it. *\/ */
/*     params_key_t key = { */
/*       .param = out_param, */
/*     }; */
/*     assert (hashtable_contains_key (automan->params, &key)); */
/*   } */

/*   outputs_key_t key = { */
/*     .flag = flag, */
/*     .output = output, */
/*     .out_param = out_param, */
/*   }; */
/*   assert (!hashtable_contains_key (automan->outputs, &key)); */
/*   hashtable_insert (automan->outputs, &key); */

/*   *flag = false; */
/* } */

/* static void */
/* proxy_fire (automan_t* automan) */
/* { */
/*   assert (*automan->self_ptr != -1); */
/*   if (automan->proxy_status == NORMAL) { */
/*     size_t idx; */
/*     /\* Go through the proxies. *\/ */
/*     for (idx = hashtable_first (automan->proxies); */
/* 	 idx != hashtable_last (automan->proxies); */
/* 	 idx = hashtable_next (automan->proxies, idx)) { */
/*       const proxies_key_t* key = hashtable_key_at (automan->proxies, idx); */
/*       if (*key->proxy_aid_ptr == -1 && *key->source_aid_ptr != -1) { */
/*         /\* Request a proxy. *\/ */
/* 	bid_t bid = proxy_request_create (key->bid, *automan->self_ptr, key->callback); */
/* 	assert (schedule_free_input (*key->source_aid_ptr, *key->source_free_input, bid) == 0); */
/* 	automan->last_proxy = *key; */
/* 	automan->proxy_status = OUTSTANDING; */
/* 	break; */
/*       } */
/*     } */
/*   } */
/* } */

/* static void */
/* dependencies_fire (automan_t* automan) */
/* { */
/*   size_t idx; */

/*   /\* Go through the dependencies. *\/ */
/*   for (idx = hashtable_first (automan->dependencies); */
/*        idx != hashtable_last (automan->dependencies); */
/*        idx = hashtable_next (automan->dependencies, idx)) { */
/*     dependencies_key_t* key = hashtable_key_at (automan->dependencies, idx); */
/*     switch (key->state) { */
/*     case DEP_START: */
/*       { */
/* 	/\* Only one thing can happen at a time. *\/ */
/* 	if (*key->child != -1) { */
/* 	  /\* Child created before dependent.  Finished. *\/ */
/* 	  key->state = DEP_FINISHED; */
/* 	} */
/* 	else if (*key->dependent != -1) { */
/* 	  /\* Dependent created before child.  Wait. *\/ */
/* 	  key->state = DEP_WAITING; */
/* 	} */
/*       } */
/*       break; */
/*     case DEP_WAITING: */
/*       { */
/* 	if (*key->child != -1) { */
/* 	  assert (schedule_free_input (*key->dependent, key->free_input, key->bid) == 0); */
/* 	  key->state = DEP_FINISHED; */
/* 	} */
/*       } */
/*       break; */
/*     case DEP_FINISHED: */
/*       /\* Done. *\/ */
/*       break; */
/*     } */
/*   } */

/* } */

/* void */
/* automan_proxy_receive (automan_t* automan, */
/* 		       const proxy_receipt_t* proxy_receipt) */
/* { */
/*   assert (automan != NULL); */
/*   assert (proxy_receipt != NULL); */

/*   assert (automan->proxy_status == OUTSTANDING); */

/*   *automan->last_proxy.proxy_aid_ptr = proxy_receipt->proxy_aid; */
/*   automan->proxy_status = NORMAL; */

/*   proxy_fire (automan); */

/*   assert (schedule_system_output () == 0); */
/* } */

/* static bid_t */
/* proxy_request_create (bid_t bid, */
/* 		      aid_t callback_aid, */
/* 		      input_t callback_free_input) */
/* { */
/*   bid_t b = buffer_alloc (sizeof (proxy_request_t)); */
/*   proxy_request_t* proxy_request = buffer_write_ptr (b); */
/*   proxy_request->bid = bid; */
/*   if (bid != -1) { */
/*     buffer_add_child (b, bid); */
/*   } */
/*   proxy_request->callback_aid = callback_aid; */
/*   proxy_request->callback_free_input = callback_free_input; */
/*   return b; */
/* } */

/* bid_t */
/* proxy_receipt_create (aid_t proxy_aid, */
/* 		      bid_t bid) */
/* { */
/*   assert (proxy_aid != -1); */

/*   bid_t b = buffer_alloc (sizeof (proxy_receipt_t)); */
/*   proxy_receipt_t* proxy_receipt = buffer_write_ptr (b); */
/*   proxy_receipt->proxy_aid = proxy_aid; */
/*   proxy_receipt->bid = bid; */
/*   if (bid != -1) { */
/*     buffer_add_child (b, bid); */
/*   } */
/*   return b; */
/* } */

/* typedef struct { */
/*   aid_t* child_aid_ptr; */
/*   const descriptor_t* descriptor; */
/*   const void* ctor_arg; */
/*   internal_t internal; */
/*   void* param; */
/* } create_key_t; */

/* static bool */
/* create_key_equal (const void* x0, */
/* 		    const void* y0) */
/* { */
/*   const create_key_t* x = x0; */
/*   const create_key_t* y = y0; */
/*   return x->child_aid_ptr == y->child_aid_ptr; */
/* } */

/* static size_t */
/* create_key_hash (const void* x0) */
/* { */
/*   const create_key_t* x = x0; */
  
/*   return (size_t)x->child_aid_ptr; */
/* } */

/* typedef struct proxies_key_struct { */
/*   aid_t* proxy_aid_ptr; */
/*   aid_t* source_aid_ptr; */
/*   input_t source_free_input; */
/*   input_t callback; */
/*   bid_t bid; */
/* } proxies_key_t; */

/* static bool */
/* proxies_key_equal (const void* x0, */
/* 		   const void* y0) */
/* { */
/*   const proxies_key_t* x = x0; */
/*   const proxies_key_t* y = y0; */
/*   return x->proxy_aid_ptr == y->proxy_aid_ptr; */
/* } */

/* static size_t */
/* proxies_key_hash (const void* x0) */
/* { */
/*   const proxies_key_t* x = x0; */
  
/*   return (size_t)x->proxy_aid_ptr; */
/* } */

/* typedef struct { */
/*   void* param; */
/*   bool declared; */
/* } params_key_t; */

/* static bool */
/* params_key_equal (const void* x0, */
/* 		  const void* y0) */
/* { */
/*   const params_key_t* x = x0; */
/*   const params_key_t* y = y0; */
/*   return x->param == y->param; */
/* } */

/* static size_t */
/* params_key_hash (const void* x0) */
/* { */
/*   const params_key_t* x = x0; */
  
/*   return (size_t)x->param; */
/* } */

/* typedef struct { */
/*   aid_t* out_automaton; */
/*   output_t output; */
/*   void* out_param; */
/*   aid_t* in_automaton; */
/*   input_t input; */
/*   void* in_param; */
/*   bool composed; */
/* } compositions_key_t; */

/* static bool */
/* compositions_key_equal (const void* x0, */
/* 			const void* y0) */
/* { */
/*   const compositions_key_t* x = x0; */
/*   const compositions_key_t* y = y0; */
/*   return */
/*     x->out_automaton == y->out_automaton && */
/*     x->output == y->output && */
/*     x->in_automaton == y->in_automaton && */
/*     x->input == y->input; */
/* } */

/* static size_t */
/* compositions_key_hash (const void* x0) */
/* { */
/*   const compositions_key_t* x = x0; */

/*   return */
/*     (size_t)x->out_automaton + */
/*     (size_t)x->output + */
/*     (size_t)x->in_automaton + */
/*     (size_t)x->input; */
/* } */

/* typedef enum { */
/*   DEP_START, */
/*   DEP_WAITING, */
/*   DEP_FINISHED, */
/* } dependency_state_t; */

/* typedef struct { */
/*   aid_t* child; */
/*   aid_t* dependent; */
/*   input_t free_input; */
/*   bid_t bid; */
/*   dependency_state_t state; */
/* } dependencies_key_t; */

/* static bool */
/* dependencies_key_equal (const void* x0, */
/* 			const void* y0) */
/* { */
/*   const dependencies_key_t* x = x0; */
/*   const dependencies_key_t* y = y0; */
/*   return */
/*     x->child == y->child && */
/*     x->dependent == y->dependent; */
/* } */

/* static size_t */
/* dependencies_key_hash (const void* x0) */
/* { */
/*   const dependencies_key_t* x = x0; */

/*   return */
/*     (size_t)x->child + */
/*     (size_t)x->dependent; */
/* } */

/* typedef struct { */
/*   input_t input; */
/*   void* in_param; */
/*   bool* flag; */
/* } inputs_key_t; */

/* static bool */
/* inputs_key_equal (const void* x0, */
/* 		  const void* y0) */
/* { */
/*   const inputs_key_t* x = x0; */
/*   const inputs_key_t* y = y0; */
/*   return */
/*     x->input == y->input && */
/*     x->in_param == y->in_param; */
/* } */

/* static size_t */
/* inputs_key_hash (const void* x0) */
/* { */
/*   const inputs_key_t* x = x0; */

/*   return */
/*     (size_t)x->input + */
/*     (size_t)x->in_param; */
/* } */

/* typedef struct { */
/*   output_t output; */
/*   void* out_param; */
/*   bool* flag; */
/* } outputs_key_t; */

/* static bool */
/* outputs_key_equal (const void* x0, */
/* 		   const void* y0) */
/* { */
/*   const outputs_key_t* x = x0; */
/*   const outputs_key_t* y = y0; */
/*   return */
/*     x->output == y->output && */
/*     x->out_param == y->out_param; */
/* } */

/* static size_t */
/* outputs_key_hash (const void* x0) */
/* { */
/*   const outputs_key_t* x = x0; */

/*   return */
/*     (size_t)x->output + */
/*     (size_t)x->out_param; */
/* } */
