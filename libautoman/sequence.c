#include "decls.h"

#include <assert.h>

static bool
si_aid_ptr_equal (const void* x0, const void* y0)
{
  const sequence_item_t* x = x0;
  const sequence_item_t* y = y0;

  return x->aid_ptr == y->aid_ptr;
}

static bool
si_flag_ptr_equal (const void* x0, const void* y0)
{
  const sequence_item_t* x = x0;
  const sequence_item_t* y = y0;

  return x->flag_ptr == y->flag_ptr;
}

bool
si_child_destroyed_aid_equal (const void* x0, const void* y0)
{
  const sequence_item_t* x = x0;
  const receipt_t* y = y0;

  switch (x->order_type) {
  case COMPOSE:
  case DECOMPOSE:
  case DECLARE:
  case RESCIND:
    return false;
  case CREATE:
  case DESTROY:
    return *x->aid_ptr == y->child_destroyed.child;
  }
  /* Not reached. */
  assert (0);
  return false;
}

bool
si_decomposed_input_equal (const void* x0, const void* y0)
{
  const sequence_item_t* x = x0;
  const receipt_t* y = y0;

  switch (x->order_type) {
  case CREATE:
  case DESTROY:
  case DECLARE:
  case RESCIND:
    return false;
  case COMPOSE:
  case DECOMPOSE:
    return
      *x->compose.in_aid_ptr == y->decomposed.in_aid &&
      x->compose.input == y->decomposed.input &&
      x->compose.in_param == y->decomposed.in_param;
  }
  /* Not reached. */
  assert (0);
  return false;
}

static bool
si_param_equal (const void* x0, const void* y0)
{
  const sequence_item_t* x = x0;
  const sequence_item_t* y = y0;

  switch (x->order_type) {
  case CREATE:
  case COMPOSE:
  case DECOMPOSE:
  case DESTROY:
    return false;
  case DECLARE:
  case RESCIND:
    return x->declare.param == y->declare.param;
  }
  /* Not reached. */
  assert (0);
  return false;
}

static bool
si_input_equal (const void* x0, const void* y0)
{
  const sequence_item_t* x = x0;
  const sequence_item_t* y = y0;

  switch (x->order_type) {
  case CREATE:
  case DECLARE:
  case RESCIND:
  case DESTROY:
    return false;
  case COMPOSE:
  case DECOMPOSE:
    return
      x->compose.in_aid_ptr == y->compose.in_aid_ptr &&
      x->compose.input == y->compose.input &&
      x->compose.in_param == y->compose.in_param;
}
  /* Not reached. */
  assert (0);
  return false;
}

static bool
si_parameterized_composition (const void* x0, const void* y0)
{
  const sequence_item_t* x = x0;
  const sequence_item_t* y = y0;

  switch (x->order_type) {
  case CREATE:
  case DECLARE:
  case DECOMPOSE:
  case RESCIND:
  case DESTROY:
    return false;
  case COMPOSE:
    return x->compose.out_param == y->declare.param || x->compose.in_param == y->declare.param;
  }
  /* Not reached. */
  assert (0);
  return false;
}

static bool
si_aid_composition (const void* x0, const void* y0)
{
  const sequence_item_t* x = x0;
  const sequence_item_t* y = y0;
  
  switch (x->order_type) {
  case CREATE:
  case DECLARE:
  case DECOMPOSE:
  case RESCIND:
  case DESTROY:
    return false;
  case COMPOSE:
    return x->compose.out_aid_ptr == y->aid_ptr || x->compose.in_aid_ptr == y->aid_ptr;
  }
  /* Not reached. */
  assert (0);
  return false;
}

static bool
si_closed_aid_ptr (automan_t* automan,
		   aid_t* aid_ptr)
{
  if (aid_ptr == automan->self_ptr) {
    /* Always open with respect to ourself. */
    return false;
  }

  sequence_item_t key;
  key.aid_ptr = aid_ptr;
  
  sequence_item_t* sequence_item = index_rfind_value (automan->si_index,
						      index_rbegin (automan->si_index),
						      index_rend (automan->si_index),
						      si_aid_ptr_equal,
						      &key,
						      NULL);

  if (sequence_item != NULL) {
    /* Found one. */
    return
      sequence_item->order_type == DESTROY ||
      sequence_item->order_type == RESCIND ||
      sequence_item->order_type == DECOMPOSE;
  }
  else {
    /* Closed because it doesn't exist. */
    return true;
  }
}

static bool
si_open_aid_ptr_create (automan_t* automan,
			aid_t* aid_ptr)
{
  if (aid_ptr == automan->self_ptr) {
    /* Always open with respect to ourself. */
    return false;
  }

  sequence_item_t key;
  key.aid_ptr = aid_ptr;
  
  sequence_item_t* sequence_item = index_rfind_value (automan->si_index,
						      index_rbegin (automan->si_index),
						      index_rend (automan->si_index),
						      si_aid_ptr_equal,
						      &key,
						      NULL);

  if (sequence_item != NULL) {
    /* Found one. */
    return
      sequence_item->order_type == CREATE;
  }
  else {
    /* Closed because it doesn't exist. */
    return false;
  }
}

static bool
si_closed_flag_ptr (automan_t* automan,
		    bool* flag_ptr)
{
  if ((aid_t*)flag_ptr == automan->self_ptr) {
    /* Alway open with respect to ourself. */
    return false;
  }

  sequence_item_t key;
  key.flag_ptr = flag_ptr;
  
  sequence_item_t* sequence_item = index_rfind_value (automan->si_index,
						      index_rbegin (automan->si_index),
						      index_rend (automan->si_index),
						      si_flag_ptr_equal,
						      &key,
						      NULL);

  if (sequence_item != NULL) {
    /* Found one. */
    return
      sequence_item->order_type == DESTROY ||
      sequence_item->order_type == RESCIND ||
      sequence_item->order_type == DECOMPOSE;
  }
  else {
    /* Closed because it doesn't exist. */
    return true;
  }
}

static bool
si_open_flag_ptr_declare (automan_t* automan,
			  bool* flag_ptr)
{
  sequence_item_t key;
  key.flag_ptr = flag_ptr;
  
  sequence_item_t* sequence_item = index_rfind_value (automan->si_index,
						      index_rbegin (automan->si_index),
						      index_rend (automan->si_index),
						      si_flag_ptr_equal,
						      &key,
						      NULL);
  
  if (sequence_item != NULL) {
    /* Found one. */
    return
      sequence_item->order_type == DECLARE;
  }
  else {
    /* Closed because it doesn't exist. */
    return false;
  }
}

static bool
si_open_flag_ptr_compose (automan_t* automan,
			  bool* flag_ptr)
{
  sequence_item_t key;
  key.flag_ptr = flag_ptr;
  
  sequence_item_t* sequence_item = index_rfind_value (automan->si_index,
						      index_rbegin (automan->si_index),
						      index_rend (automan->si_index),
						      si_flag_ptr_equal,
						      &key,
						      NULL);
  
  if (sequence_item != NULL) {
    /* Found one. */
    return
      sequence_item->order_type == COMPOSE;
  }
  else {
    /* Closed because it doesn't exist. */
    return false;
  }
}

static bool
si_closed_param (automan_t* automan,
		 void* param)
{
  sequence_item_t key;
  key.declare.param = param;
  
  sequence_item_t* sequence_item = index_rfind_value (automan->si_index,
						      index_rbegin (automan->si_index),
						      index_rend (automan->si_index),
						      si_param_equal,
						      &key,
						      NULL);
  
  if (sequence_item != NULL) {
    /* Found one. */
    return
      sequence_item->order_type == RESCIND;
  }
  else {
    /* Closed because it doesn't exist. */
    return true;
  }
}

static bool
si_open_param (automan_t* automan,
	       void* param)
{
  sequence_item_t key;
  key.declare.param = param;
  
  sequence_item_t* sequence_item = index_rfind_value (automan->si_index,
						      index_rbegin (automan->si_index),
						      index_rend (automan->si_index),
						      si_param_equal,
						      &key,
						      NULL);
  
  if (sequence_item != NULL) {
    /* Found one. */
    return
      sequence_item->order_type == DECLARE;
  }
  else {
    /* Closed because it doesn't exist. */
    return false;
  }
}

static bool
si_closed_input (automan_t* automan,
		 aid_t* in_aid_ptr,
		 input_t input,
		 void* in_param)
{
  sequence_item_t key;
  key.compose.in_aid_ptr = in_aid_ptr;
  key.compose.input = input;
  key.compose.in_param = in_param;
  
  sequence_item_t* sequence_item = index_rfind_value (automan->si_index,
						      index_rbegin (automan->si_index),
						      index_rend (automan->si_index),
						      si_input_equal,
						      &key,
						      NULL);
  
  if (sequence_item != NULL) {
    /* Found one. */
    return
      sequence_item->order_type == DECOMPOSE;
  }
  else {
    /* Closed because it doesn't exist. */
    return true;
  }
}

static void
si_append_create (automan_t* automan,
		  aid_t* aid_ptr,
		  const descriptor_t* descriptor,
		  const void* ctor_arg,
		  automan_handler_t handler,
		  void* hparam)
{
  sequence_item_t sequence_item;
  sequence_item.order_type = CREATE;
  sequence_item.handler = handler;
  sequence_item.hparam = hparam;
  sequence_item.aid_ptr = aid_ptr;
  sequence_item.create.descriptor = descriptor;
  sequence_item.create.ctor_arg = ctor_arg;

  index_push_back (automan->si_index, &sequence_item);
}

static void
si_append_destroy (automan_t* automan,
		   aid_t* aid_ptr,
		automan_handler_t handler,
		void* hparam)
{
  sequence_item_t sequence_item;
  sequence_item.order_type = DESTROY;
  sequence_item.handler = handler;
  sequence_item.hparam = hparam;
  sequence_item.aid_ptr = aid_ptr;

  index_push_back (automan->si_index, &sequence_item);
}

static void
si_append_declare (automan_t* automan,
		   bool* flag_ptr,
		   void* param,
		   automan_handler_t handler,
		   void* hparam)
{
  sequence_item_t sequence_item;
  sequence_item.order_type = DECLARE;
  sequence_item.handler = handler;
  sequence_item.hparam = hparam;
  sequence_item.flag_ptr = flag_ptr;
  sequence_item.declare.param = param;
  
  index_push_back (automan->si_index, &sequence_item);
}

static void
si_append_rescind (automan_t* automan,
		   bool* flag_ptr,
		   void* param,
		   automan_handler_t handler,
		   void* hparam)
{
  sequence_item_t sequence_item;
  sequence_item.order_type = RESCIND;
  sequence_item.handler = handler;
  sequence_item.hparam = hparam;
  sequence_item.flag_ptr = flag_ptr;
  sequence_item.declare.param = param;
  
  index_push_back (automan->si_index, &sequence_item);
}

static void
si_append_compose (automan_t* automan,
		   bool* flag_ptr,
		   aid_t* out_aid_ptr,
		   output_t output,
		   void* out_param,
		   aid_t* in_aid_ptr,
		   input_t input,
		   void* in_param,
		   automan_handler_t handler,
		   void* hparam)
{
  sequence_item_t sequence_item;
  sequence_item.order_type = COMPOSE;
  sequence_item.handler = handler;
  sequence_item.hparam = hparam;
  sequence_item.flag_ptr = flag_ptr;
  sequence_item.compose.out_aid_ptr = out_aid_ptr;
  sequence_item.compose.output = output;
  sequence_item.compose.out_param = out_param;
  sequence_item.compose.in_aid_ptr = in_aid_ptr;
  sequence_item.compose.input = input;
  sequence_item.compose.in_param = in_param;

  index_push_back (automan->si_index, &sequence_item);
}

static void
si_append_decompose (automan_t* automan,
		     bool* flag_ptr,
		     aid_t* out_aid_ptr,
		     output_t output,
		     void* out_param,
		     aid_t* in_aid_ptr,
		     input_t input,
		     void* in_param,
		     automan_handler_t handler,
		     void* hparam)
{
  sequence_item_t sequence_item;
  sequence_item.order_type = DECOMPOSE;
  sequence_item.handler = handler;
  sequence_item.hparam = hparam;
  sequence_item.flag_ptr = flag_ptr;
  sequence_item.compose.out_aid_ptr = out_aid_ptr;
  sequence_item.compose.output = output;
  sequence_item.compose.out_param = out_param;
  sequence_item.compose.in_aid_ptr = in_aid_ptr;
  sequence_item.compose.input = input;
  sequence_item.compose.in_param = in_param;

  index_push_back (automan->si_index, &sequence_item);
}

static bool
si_param_declared (automan_t* automan,
		   void* param)
{
  if (param == NULL) {
    return true;
  }
  
  sequence_item_t key;
  key.declare.param = param;
  
  sequence_item_t* sequence_item = index_find_value (automan->si_index,
						     index_begin (automan->si_index),
						     index_end (automan->si_index),
						     si_param_equal,
						     &key,
						     NULL);
  
  if (sequence_item != NULL) {
    return *sequence_item->flag_ptr;
  }
  else {
    /* Doesn't exist. */
    return false;
  }
}

static sequence_item_t*
si_find_flag_ptr (automan_t* automan,
		  bool* flag_ptr,
		  iterator_t begin,
		  iterator_t end,
		  iterator_t* iterator)
{
  sequence_item_t key;
  key.flag_ptr = flag_ptr;
  
  return index_find_value (automan->si_index,
			   begin,
			   end,
			   si_flag_ptr_equal,
			   &key,
			   iterator);
}

static sequence_item_t*
si_rfind_flag_ptr (automan_t* automan,
		   bool* flag_ptr,
		   riterator_t* riterator)
{
  sequence_item_t key;
  key.flag_ptr = flag_ptr;
  
  return index_rfind_value (automan->si_index,
			    index_rbegin (automan->si_index),
			    index_rend (automan->si_index),
			    si_flag_ptr_equal,
			    &key,
			    riterator);
}

static sequence_item_t*
si_find_aid_ptr (automan_t* automan,
		 aid_t* aid_ptr,
		 iterator_t begin,
		 iterator_t end,
		 iterator_t* iterator)
{
  sequence_item_t key;
  key.aid_ptr = aid_ptr;
  
  return index_find_value (automan->si_index,
			   begin,
			   end,
			   si_aid_ptr_equal,
			   &key,
			   iterator);
}

static sequence_item_t*
si_rfind_aid_ptr (automan_t* automan,
		  aid_t* aid_ptr,
		  riterator_t* riterator)
{
  sequence_item_t key;
  key.aid_ptr = aid_ptr;
  
  return index_rfind_value (automan->si_index,
			    index_rbegin (automan->si_index),
			    index_rend (automan->si_index),
			    si_aid_ptr_equal,
			    &key,
			    riterator);
}

static sequence_item_t*
si_find_composition_param (automan_t* automan,
			   bool* flag_ptr,
			   void* param,
			   iterator_t begin,
			   iterator_t* pos)
{
  sequence_item_t key;
  key.flag_ptr = flag_ptr;
  key.declare.param = param;

  return index_find_value (automan->si_index,
			   begin,
			   index_end (automan->si_index),
			   si_parameterized_composition,
			   &key,
			   pos);
}

static sequence_item_t*
si_find_composition_aid (automan_t* automan,
			 aid_t* aid_ptr,
			 iterator_t begin,
			 iterator_t end,
			 iterator_t* pos)
{
  sequence_item_t key;
  key.aid_ptr = aid_ptr;
  
  return index_find_value (automan->si_index,
			   begin,
			   end,
			   si_aid_composition,
			   &key,
			   pos);
}

static bool
si_process (automan_t* automan)
{
  assert (automan != NULL);

  iterator_t iterator;
  for (iterator = index_begin (automan->si_index);
       iterator_ne (iterator, index_end (automan->si_index));
       iterator = index_advance (automan->si_index, iterator)) {
    sequence_item_t* sequence_item = index_value (automan->si_index, iterator);
    switch (sequence_item->order_type) {
    case CREATE:
      {
	if (*sequence_item->aid_ptr == -1) {
	  /* Create an automaton. */
	  order_create_init (&automan->last_order,
			     sequence_item->create.descriptor,
			     sequence_item->create.ctor_arg);
	  automan->last_sequence = *sequence_item;
	  return true;
	}
      }
      break;
    case DECLARE:
      {
	if (*sequence_item->flag_ptr == false) {
	  /* We need to declare a parameter. */
	  order_declare_init (&automan->last_order,
			      sequence_item->declare.param);
	  automan->last_sequence = *sequence_item;
	  return true;
	}
      }
      break;
    case COMPOSE:
      {
      	if (*sequence_item->flag_ptr == false &&
      	    *sequence_item->compose.out_aid_ptr != -1 &&
      	    si_param_declared (automan, sequence_item->compose.out_param) &&
      	    *sequence_item->compose.in_aid_ptr != -1 &&
      	    si_param_declared (automan, sequence_item->compose.in_param)) {
      	  /* We need to compose. */
      	  order_compose_init (&automan->last_order,
      			      *sequence_item->compose.out_aid_ptr,
      			      sequence_item->compose.output,
      			      sequence_item->compose.out_param,
      			      *sequence_item->compose.in_aid_ptr,
      			      sequence_item->compose.input,
      			      sequence_item->compose.in_param);
      	  automan->last_sequence = *sequence_item;
      	  return true;
      	}
      }
      break;
    case DECOMPOSE:
      {
	if (*sequence_item->flag_ptr == true) {
	  /* We need to decompose. */
	  order_decompose_init (&automan->last_order,
				*sequence_item->compose.out_aid_ptr,
				sequence_item->compose.output,
				sequence_item->compose.out_param,
				*sequence_item->compose.in_aid_ptr,
				sequence_item->compose.input,
				sequence_item->compose.in_param);
	  automan->last_sequence = *sequence_item;
	  return true;
	}
      }
      break;
    case RESCIND:
      {
	if (*sequence_item->flag_ptr == true) {
	  /* We need to rescind a parameter. */
	  order_rescind_init (&automan->last_order,
			      sequence_item->declare.param);
	  automan->last_sequence = *sequence_item;
	  return true;
	}
      }
      break;
    case DESTROY:
      {
	if (*sequence_item->aid_ptr != -1) {
	  /* Create an automaton. */
	  order_destroy_init (&automan->last_order,
			      *sequence_item->aid_ptr);
	  automan->last_sequence = *sequence_item;
	  return true;
	}
      }
      break;
    }
  }

  return false;
}

void
si_balance_compose (automan_t* automan,
		    bool* flag_ptr)
{
  /* Find the compose. */
  iterator_t compose_pos;
  sequence_item_t* compose = si_find_flag_ptr (automan,
					       flag_ptr,
					       index_begin (automan->si_index),
					       index_end (automan->si_index),
					       &compose_pos);
  assert (compose != NULL);
  assert (compose->order_type == COMPOSE);
  
  compose_pos = index_advance (automan->si_index, compose_pos);
  
  /* Find the decompose. */
  sequence_item_t* decompose = si_find_flag_ptr (automan,
						 flag_ptr,
						 compose_pos,
						 index_end (automan->si_index),
						 NULL);
  
  if (decompose == NULL) {
    /* Decompose doesn't exist so add one. */
    assert (automan_decompose (automan,
			       flag_ptr) == 0);
  }
}

iterator_t
si_decompose_flag_ptr (automan_t* automan,
		       bool* flag_ptr,
		       receipt_type_t receipt)
{
  /* Remove the compose. */
  iterator_t compose_pos;
  sequence_item_t* compose = si_find_flag_ptr (automan,
					    flag_ptr,
					    index_begin (automan->si_index),
					    index_end (automan->si_index),
					    &compose_pos);
  assert (compose != NULL);
  assert (compose->order_type == COMPOSE);
  sequence_item_t compose_item = *compose;
  compose_pos = index_erase (automan->si_index, compose_pos);

  /* Remove the decompose. */
  iterator_t decompose_pos;
  sequence_item_t* decompose = si_find_flag_ptr (automan,
					      flag_ptr,
					      compose_pos,
					      index_end (automan->si_index),
					      &decompose_pos);
  assert (decompose != NULL);
  assert (decompose->order_type == DECOMPOSE);
  decompose_pos = index_erase (automan->si_index, decompose_pos);

  /* Set the flag. */
  *compose_item.flag_ptr = false;

  if (compose_item.handler != NULL) {
    compose_item.handler (automan->state, compose_item.hparam, receipt);
  }

  return compose_pos;
}

void
si_balance_create (automan_t* automan,
		   aid_t* aid_ptr)
{
  /* Find the create. */
  iterator_t create_pos;
  sequence_item_t* create = si_find_aid_ptr (automan,
					     aid_ptr,
					     index_begin (automan->si_index),
					     index_end (automan->si_index),
					     &create_pos);
  assert (create != NULL);
  assert (create->order_type == CREATE);
  
  create_pos = index_advance (automan->si_index, create_pos);
  
  /* Find the destroy. */
  sequence_item_t* destroy = si_find_aid_ptr (automan,
					      aid_ptr,
					      create_pos,
					      index_end (automan->si_index),
					      NULL);
  
  if (destroy == NULL) {
    /* Destroy doesn't exist so add one. */
    assert (automan_destroy (automan,
			     aid_ptr) == 0);
  }
}

void
si_destroy_aid_ptr (automan_t* automan,
		    aid_t* aid_ptr)
{
  /* Remove the create. */
  iterator_t create_pos;
  sequence_item_t* create = si_find_aid_ptr (automan,
					     aid_ptr,
					     index_begin (automan->si_index),
					     index_end (automan->si_index),
					     &create_pos);
  assert (create != NULL);
  assert (create->order_type == CREATE);
  sequence_item_t create_item = *create;
  create_pos = index_erase (automan->si_index, create_pos);
  
  /* Remove the destroy. */
  iterator_t destroy_pos;
  sequence_item_t* destroy = si_find_aid_ptr (automan,
					      aid_ptr,
					      create_pos,
					      index_end (automan->si_index),
					      &destroy_pos);
  assert (destroy != NULL);
  assert (destroy->order_type == DESTROY);
  if (iterator_eq (create_pos, destroy_pos)) {
    /* Nothing between create and destroy. Skip over. */
    create_pos = index_advance (automan->si_index, create_pos);
    /* The next line will make them equal again. */
  }
  destroy_pos = index_erase (automan->si_index, destroy_pos);
  
  /* Remove all compositions between create_pos and destroy_pos that use this aid. */
  iterator_t compose_pos = create_pos;
  for (;;) {
    /* Find a composition that uses this aid. */
    sequence_item_t* composition = si_find_composition_aid (automan,
							    create_item.aid_ptr,
							    compose_pos,
							    destroy_pos,
							    &compose_pos);
    if (composition != NULL) {
      /* Found one.  Decompose. */
      compose_pos = si_decompose_flag_ptr (automan,
					   composition->flag_ptr,
					   DECOMPOSED);
    }
    else {
      /* No more compositions use this aid.  Done. */
      break;
    }
  }
  
  /* Set the aid pointer. */
  *create_item.aid_ptr = -1;
  
  if (create_item.handler != NULL) {
    create_item.handler (automan->state, create_item.hparam, CHILD_DESTROYED);
  }
}

int
automan_create (automan_t* automan,
		aid_t* aid_ptr,
		const descriptor_t* descriptor,
		const void* ctor_arg,
		automan_handler_t handler,
		void* hparam)
{
  assert (automan != NULL);
  assert (aid_ptr != NULL);
  assert (descriptor != NULL);
  assert (hparam == NULL || handler != NULL);

  if (!si_closed_aid_ptr (automan, aid_ptr)) {
    /* Duplicate create. */
    return -1;
  }
  
  /* Clear the automaton. */
  *aid_ptr = -1;

  /* Add an action item. */
  si_append_create (automan,
		 aid_ptr,
		 descriptor,
		 ctor_arg,
		 handler,
		 hparam);

  if (*automan->self_ptr != -1) {
    assert (schedule_system_output () == 0);
  }

  return 0;
}

int
automan_declare (automan_t* automan,
		 bool* flag_ptr,
		 void* param,
		 automan_handler_t handler,
		 void* hparam)
{
  assert (automan != NULL);
  assert (flag_ptr != NULL);
  assert (param != NULL);
  assert (hparam == NULL || handler != NULL);

  if (!si_closed_flag_ptr (automan, flag_ptr) ||
      !si_closed_param (automan, param)) {
    /* Duplicate declare. */
    return -1;
  }

  /* Clear the flag. */
  *flag_ptr = false;
  
  /* Add an action item. */
  si_append_declare (automan,
		  flag_ptr,
		  param,
		  handler,
		  hparam);

  if (*automan->self_ptr != -1) {
    assert (schedule_system_output () == 0);
  }
  
  return 0;
}

int
automan_compose (automan_t* automan,
		 bool* flag_ptr,
		 aid_t* out_aid_ptr,
		 output_t output,
		 void* out_param,
		 aid_t* in_aid_ptr,
		 input_t input,
		 void* in_param,
		 automan_handler_t handler,
		 void* hparam)
{
  assert (automan != NULL);
  assert (flag_ptr != NULL);
  assert (out_aid_ptr != NULL);
  assert (output != NULL);
  assert (in_aid_ptr != NULL);
  assert (input != NULL);
  assert (out_param == NULL || in_param == NULL);
  assert (out_param == NULL || automan->self_ptr == out_aid_ptr);
  assert (in_param == NULL || automan->self_ptr == in_aid_ptr);

  assert (hparam == NULL || handler != NULL);
  
  /* Check for parameter management. */
  if (out_param != NULL &&
      !si_open_param (automan, out_param)) {
    return -1;
  }
  else if (in_param != NULL &&
	   !si_open_param (automan, in_param)) {
    return -1;
  }

  if (!si_closed_flag_ptr (automan, flag_ptr) ||
      !si_closed_input (automan, in_aid_ptr, input, in_param)) {
    /* Duplicate composition. */
    return -1;
  }
  
  /* Clear the flag. */
  *flag_ptr = false;
  
  si_append_compose (automan,
		  flag_ptr,
		  out_aid_ptr,
		  output,
		  out_param,
		  in_aid_ptr,
		  input,
		  in_param,
		  handler,
		  hparam);

  if (*automan->self_ptr != -1) {
    assert (schedule_system_output () == 0);
  }
  
  return 0;
}

int
automan_decompose (automan_t* automan,
		   bool* flag_ptr)
{
  assert (automan != NULL);
  assert (flag_ptr != NULL);

  if (!si_open_flag_ptr_compose (automan, flag_ptr)) {
    /* Duplicate decompose. */
    return -1;
  }
  
  /* Find the compose. */
  sequence_item_t* sequence_item = si_rfind_flag_ptr (automan, flag_ptr, NULL);
  
  si_append_decompose (automan,
		    sequence_item->flag_ptr,
		    sequence_item->compose.out_aid_ptr,
		    sequence_item->compose.output,
		    sequence_item->compose.out_param,
		    sequence_item->compose.in_aid_ptr,
		    sequence_item->compose.input,
		    sequence_item->compose.in_param,
		    sequence_item->handler,
		    sequence_item->hparam);

  if (*automan->self_ptr != -1) {
    assert (schedule_system_output () == 0);
  }
  
  return 0;
}

int
automan_rescind (automan_t* automan,
		 bool* flag_ptr)
{
  assert (automan != NULL);
  assert (flag_ptr != NULL);

  if (!si_open_flag_ptr_declare (automan, flag_ptr)) {
    /* Duplicate rescind. */
    return -1;
  }
  
  /* Find the declare. */
  riterator_t declare_pos;
  sequence_item_t* sequence_item = si_rfind_flag_ptr (automan,
						   flag_ptr,
						   &declare_pos);

  /* Every composition that uses the parameter must be paired with a decomposition. */
  iterator_t compose_pos = riterator_reverse (automan->si_index, declare_pos);
  for (;;) {
    /* Find a composition using this parameter. */
    sequence_item_t* composition = si_find_composition_param (automan,
							   flag_ptr,
							   sequence_item->declare.param,
							   compose_pos,
							   &compose_pos);

    if (composition != NULL) {
      /* Found a composition using this parameter. Look for the decomposition. */
      sequence_item_t* decomposition = si_find_flag_ptr (automan,
						      composition->flag_ptr,
						      compose_pos,
						      index_end (automan->si_index),
						      NULL);
      if (decomposition == NULL) {
      	/* No corresponding decomposition so add one. */
	si_append_decompose (automan,
			  composition->flag_ptr,
			  composition->compose.out_aid_ptr,
			  composition->compose.output,
			  composition->compose.out_param,
			  composition->compose.in_aid_ptr,
			  composition->compose.input,
			  composition->compose.in_param,
			  composition->handler,
			  composition->hparam);
	/* Technically, we don't need to add the decompositions.
	   We could just rescind the parameter and let the system tell us about all of the decompositions.
	   However, it would violate the invariant that the user could extend the sequence by leaving unpaired compositions.
	*/
      }
      
      /* Advance. */
      compose_pos = index_advance (automan->si_index, compose_pos);
    }
    else {
      /* No subsequent compositions use this paramter.  Done. */
      break;
    }
  }
  
  /* Add an action item. */
  si_append_rescind (automan,
		  sequence_item->flag_ptr,
		  sequence_item->declare.param,
		  sequence_item->handler,
		  sequence_item->hparam);

  if (*automan->self_ptr != -1) {
    assert (schedule_system_output () == 0);
  }
  
  return 0;
}

int
automan_destroy (automan_t* automan,
		 aid_t* aid_ptr)
{
  assert (automan != NULL);
  assert (aid_ptr != NULL);

  if (!si_open_aid_ptr_create (automan, aid_ptr)) {
    /* Duplicate destroy. */
    return -1;
  }

  /* Find the create. */
  riterator_t create_pos;
  sequence_item_t* sequence_item = si_rfind_aid_ptr (automan,
						  aid_ptr,
						  &create_pos);

  /* Every composition that uses this aid must be paired with a decomposition. */
  iterator_t compose_pos = riterator_reverse (automan->si_index, create_pos);
  for (;;) {
    /* Find a composition using this aid. */
    sequence_item_t* composition = si_find_composition_aid (automan,
							 aid_ptr,
							 compose_pos,
							 index_end (automan->si_index),
							 &compose_pos);
    if (composition != NULL) {
      /* Found a composition using this aid. Look for the decomposition. */
      sequence_item_t* decomposition = si_find_flag_ptr (automan,
						      composition->flag_ptr,
						      compose_pos,
						      index_end (automan->si_index),
						      NULL);
      if (decomposition == NULL) {
      	/* No corresponding decomposition so add one. */
  	si_append_decompose (automan,
  			  composition->flag_ptr,
  			  composition->compose.out_aid_ptr,
  			  composition->compose.output,
  			  composition->compose.out_param,
  			  composition->compose.in_aid_ptr,
  			  composition->compose.input,
  			  composition->compose.in_param,
  			  composition->handler,
  			  composition->hparam);
  	/* Technically, we don't need to add the decompositions.
  	   We could just destroy the automaton and let the system tell us about all of the decompositions.
  	   However, it would violate the invariant that the user could extend the sequence by leaving unpaired compositions.
  	*/
      }
      
      /* Advance. */
      compose_pos = index_advance (automan->si_index, compose_pos);
    }
    else {
      /* No subsequent compositions use this paramter.  Done. */
      break;
    }
  }
  
  /* Add an action item. */
  si_append_destroy (automan,
		  sequence_item->aid_ptr,
		  sequence_item->handler,
		  sequence_item->hparam);

  if (*automan->self_ptr != -1) {
    assert (schedule_system_output () == 0);
  }

  return 0;
}

bid_t
automan_action (automan_t* automan)
{
  assert (automan != NULL);

  switch (automan->sequence_status) {
  case NORMAL:
    {
      if (si_process (automan)) {
	bid_t bid = buffer_alloc (sizeof (order_t));
	order_t* order = buffer_write_ptr (bid);
	*order = automan->last_order;
	automan->sequence_status = OUTSTANDING;
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
