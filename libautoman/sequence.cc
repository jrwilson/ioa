#include "automan.hh"

#include <cassert>
#include <algorithm>

// static bool
// si_input_equal (const void* x0, const void* y0)
// {
//   const sequence_item_t* x = (const sequence_item_t*)x0;
//   const sequence_item_t* y = (const sequence_item_t*)y0;

//   switch (x->order_type) {
//   case CREATE:
//   case DECLARE:
//   case RESCIND:
//   case DESTROY:
//     return false;
//   case COMPOSE:
//   case DECOMPOSE:
//     return
//       x->compose.in_aid_ptr == y->compose.in_aid_ptr &&
//       x->compose.input == y->compose.input &&
//       x->compose.in_param == y->compose.in_param;
// }
//   /* Not reached. */
//   assert (0);
//   return false;
// }

// static bool
// si_parameterized_composition (const void* x0, const void* y0)
// {
//   const sequence_item_t* x = (const sequence_item_t*)x0;
//   const sequence_item_t* y = (const sequence_item_t*)y0;

//   switch (x->order_type) {
//   case CREATE:
//   case DECLARE:
//   case DECOMPOSE:
//   case RESCIND:
//   case DESTROY:
//     return false;
//   case COMPOSE:
//     return x->compose.out_param == y->declare.param || x->compose.in_param == y->declare.param;
//   }
//   /* Not reached. */
//   assert (0);
//   return false;
// }

// static bool
// si_aid_composition (const void* x0, const void* y0)
// {
//   const sequence_item_t* x = (const sequence_item_t*)x0;
//   const sequence_item_t* y = (const sequence_item_t*)y0;
  
//   switch (x->order_type) {
//   case CREATE:
//   case DECLARE:
//   case DECOMPOSE:
//   case RESCIND:
//   case DESTROY:
//     return false;
//   case COMPOSE:
//     return x->compose.out_aid_ptr == y->aid_ptr || x->compose.in_aid_ptr == y->aid_ptr;
//   }
//   /* Not reached. */
//   assert (0);
//   return false;
// }

bool
automan::si_closed_aid_ptr (aid_t* aid_ptr)
{
  if (aid_ptr == m_self_ptr) {
    /* Always open with respect to ourself. */
    return false;
  }

  si_list::reverse_iterator pos = std::find_if (m_si_index.rbegin (),
						m_si_index.rend (),
						si_aid_ptr_equal (aid_ptr));

  if (pos != m_si_index.rend ()) {
    /* Found one. */
    return
      pos->order_type == DESTROY ||
      pos->order_type == RESCIND ||
      pos->order_type == DECOMPOSE;
  }
  else {
    /* Closed because it doesn't exist. */
    return true;
  }
}

bool
automan::si_open_aid_ptr_create (aid_t* aid_ptr)
{
  if (aid_ptr == m_self_ptr) {
    /* Always open with respect to ourself. */
    return false;
  }

  si_list::reverse_iterator pos = std::find_if (m_si_index.rbegin (),
						m_si_index.rend (),
						si_aid_ptr_equal (aid_ptr));
  if (pos != m_si_index.rend ()) {
    /* Found one. */
    return
      pos->order_type == CREATE;
  }
  else {
    /* Closed because it doesn't exist. */
    return false;
  }
}

// static bool
// si_closed_flag_ptr (automan_t* automan,
// 		    bool* flag_ptr)
// {
//   if ((aid_t*)flag_ptr == m_self_ptr) {
//     /* Alway open with respect to ourself. */
//     return false;
//   }

//   sequence_item_t key;
//   key.flag_ptr = flag_ptr;
  
//   sequence_item_t* sequence_item = (sequence_item_t*)index_rfind_value (m_si_index,
// 						      index_rbegin (m_si_index),
// 						      index_rend (m_si_index),
// 						      si_flag_ptr_equal,
// 						      &key,
// 						      NULL);

//   if (sequence_item != NULL) {
//     /* Found one. */
//     return
//       sequence_item->order_type == DESTROY ||
//       sequence_item->order_type == RESCIND ||
//       sequence_item->order_type == DECOMPOSE;
//   }
//   else {
//     /* Closed because it doesn't exist. */
//     return true;
//   }
// }

// static bool
// si_open_flag_ptr_declare (automan_t* automan,
// 			  bool* flag_ptr)
// {
//   sequence_item_t key;
//   key.flag_ptr = flag_ptr;
  
//   sequence_item_t* sequence_item = (sequence_item_t*)index_rfind_value (m_si_index,
// 						      index_rbegin (m_si_index),
// 						      index_rend (m_si_index),
// 						      si_flag_ptr_equal,
// 						      &key,
// 						      NULL);
  
//   if (sequence_item != NULL) {
//     /* Found one. */
//     return
//       sequence_item->order_type == DECLARE;
//   }
//   else {
//     /* Closed because it doesn't exist. */
//     return false;
//   }
// }

bool
automan::si_open_flag_ptr_compose (bool* flag_ptr)
{
  si_list::reverse_iterator pos = std::find_if (m_si_index.rbegin (),
						m_si_index.rend (),
						si_flag_ptr_equal (flag_ptr));
  if (pos != m_si_index.rend ()) {
    /* Found one. */
    return
      pos->order_type == COMPOSE;
  }
  else {
    /* Closed because it doesn't exist. */
    return false;
  }
}

// static bool
// si_closed_param (automan_t* automan,
// 		 void* param)
// {
//   sequence_item_t key;
//   key.declare.param = param;
  
//   sequence_item_t* sequence_item = (sequence_item_t*)index_rfind_value (m_si_index,
// 						      index_rbegin (m_si_index),
// 						      index_rend (m_si_index),
// 						      si_param_equal,
// 						      &key,
// 						      NULL);
  
//   if (sequence_item != NULL) {
//     /* Found one. */
//     return
//       sequence_item->order_type == RESCIND;
//   }
//   else {
//     /* Closed because it doesn't exist. */
//     return true;
//   }
// }

// static bool
// si_open_param (automan_t* automan,
// 	       void* param)
// {
//   sequence_item_t key;
//   key.declare.param = param;
  
//   sequence_item_t* sequence_item = (sequence_item_t*)index_rfind_value (m_si_index,
// 						      index_rbegin (m_si_index),
// 						      index_rend (m_si_index),
// 						      si_param_equal,
// 						      &key,
// 						      NULL);
  
//   if (sequence_item != NULL) {
//     /* Found one. */
//     return
//       sequence_item->order_type == DECLARE;
//   }
//   else {
//     /* Closed because it doesn't exist. */
//     return false;
//   }
// }

// static bool
// si_closed_input (automan_t* automan,
// 		 aid_t* in_aid_ptr,
// 		 input_t input,
// 		 void* in_param)
// {
//   sequence_item_t key;
//   key.compose.in_aid_ptr = in_aid_ptr;
//   key.compose.input = input;
//   key.compose.in_param = in_param;
  
//   sequence_item_t* sequence_item = (sequence_item_t*)index_rfind_value (m_si_index,
// 						      index_rbegin (m_si_index),
// 						      index_rend (m_si_index),
// 						      si_input_equal,
// 						      &key,
// 						      NULL);
  
//   if (sequence_item != NULL) {
//     /* Found one. */
//     return
//       sequence_item->order_type == DECOMPOSE;
//   }
//   else {
//     /* Closed because it doesn't exist. */
//     return true;
//   }
// }

void
automan::si_append_create (aid_t* aid_ptr,
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
  
  m_si_index.push_back (sequence_item);
}

void
automan::si_append_destroy (aid_t* aid_ptr,
			    automan_handler_t handler,
			    void* hparam)
{
  sequence_item_t sequence_item;
  sequence_item.order_type = DESTROY;
  sequence_item.handler = handler;
  sequence_item.hparam = hparam;
  sequence_item.aid_ptr = aid_ptr;

  m_si_index.push_back (sequence_item);
}

// static void
// si_append_declare (automan_t* automan,
// 		   bool* flag_ptr,
// 		   void* param,
// 		   automan_handler_t handler,
// 		   void* hparam)
// {
//   sequence_item_t sequence_item;
//   sequence_item.order_type = DECLARE;
//   sequence_item.handler = handler;
//   sequence_item.hparam = hparam;
//   sequence_item.flag_ptr = flag_ptr;
//   sequence_item.declare.param = param;
  
//   index_push_back (m_si_index, &sequence_item);
// }

// static void
// si_append_rescind (automan_t* automan,
// 		   bool* flag_ptr,
// 		   void* param,
// 		   automan_handler_t handler,
// 		   void* hparam)
// {
//   sequence_item_t sequence_item;
//   sequence_item.order_type = RESCIND;
//   sequence_item.handler = handler;
//   sequence_item.hparam = hparam;
//   sequence_item.flag_ptr = flag_ptr;
//   sequence_item.declare.param = param;
  
//   index_push_back (m_si_index, &sequence_item);
// }

// static void
// si_append_compose (automan_t* automan,
// 		   bool* flag_ptr,
// 		   aid_t* out_aid_ptr,
// 		   output_t output,
// 		   void* out_param,
// 		   aid_t* in_aid_ptr,
// 		   input_t input,
// 		   void* in_param,
// 		   automan_handler_t handler,
// 		   void* hparam)
// {
//   sequence_item_t sequence_item;
//   sequence_item.order_type = COMPOSE;
//   sequence_item.handler = handler;
//   sequence_item.hparam = hparam;
//   sequence_item.flag_ptr = flag_ptr;
//   sequence_item.compose.out_aid_ptr = out_aid_ptr;
//   sequence_item.compose.output = output;
//   sequence_item.compose.out_param = out_param;
//   sequence_item.compose.in_aid_ptr = in_aid_ptr;
//   sequence_item.compose.input = input;
//   sequence_item.compose.in_param = in_param;

//   index_push_back (m_si_index, &sequence_item);
// }

void
automan::si_append_decompose (bool* flag_ptr,
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

  m_si_index.push_back (sequence_item);
}

bool
automan::si_param_declared (void* param)
{
  if (param == NULL) {
    return true;
  }
  
  si_list::iterator pos = std::find_if (m_si_index.begin (),
					m_si_index.end (),
					si_param_equal (param));

  if (pos != m_si_index.end ()) {
    return *pos->flag_ptr;
  }
  else {
    /* Doesn't exist. */
    return false;
  }
}

// static sequence_item_t*
// si_find_flag_ptr (automan_t* automan,
// 		  bool* flag_ptr,
// 		  iterator_t begin,
// 		  iterator_t end,
// 		  iterator_t* iterator)
// {
//   sequence_item_t key;
//   key.flag_ptr = flag_ptr;
  
//   return (sequence_item_t*)index_find_value (m_si_index,
// 			   begin,
// 			   end,
// 			   si_flag_ptr_equal,
// 			   &key,
// 			   iterator);
// }

// static sequence_item_t*
// si_rfind_flag_ptr (automan_t* automan,
// 		   bool* flag_ptr,
// 		   riterator_t* riterator)
// {
//   sequence_item_t key;
//   key.flag_ptr = flag_ptr;
  
//   return (sequence_item_t*)index_rfind_value (m_si_index,
// 			    index_rbegin (m_si_index),
// 			    index_rend (m_si_index),
// 			    si_flag_ptr_equal,
// 			    &key,
// 			    riterator);
// }

// static sequence_item_t*
// si_find_aid_ptr (automan_t* automan,
// 		 aid_t* aid_ptr,
// 		 iterator_t begin,
// 		 iterator_t end,
// 		 iterator_t* iterator)
// {
//   sequence_item_t key;
//   key.aid_ptr = aid_ptr;
  
//   return (sequence_item_t*)index_find_value (m_si_index,
// 			   begin,
// 			   end,
// 			   si_aid_ptr_equal,
// 			   &key,
// 			   iterator);
// }

// static sequence_item_t*
// si_find_composition_param (automan_t* automan,
// 			   bool* flag_ptr,
// 			   void* param,
// 			   iterator_t begin,
// 			   iterator_t* pos)
// {
//   sequence_item_t key;
//   key.flag_ptr = flag_ptr;
//   key.declare.param = param;

//   return (sequence_item_t*)index_find_value (m_si_index,
// 			   begin,
// 			   index_end (m_si_index),
// 			   si_parameterized_composition,
// 			   &key,
// 			   pos);
// }

// static sequence_item_t*
// si_find_composition_aid (automan_t* automan,
// 			 aid_t* aid_ptr,
// 			 iterator_t begin,
// 			 iterator_t end,
// 			 iterator_t* pos)
// {
//   sequence_item_t key;
//   key.aid_ptr = aid_ptr;
  
//   return (sequence_item_t*)index_find_value (m_si_index,
// 			   begin,
// 			   end,
// 			   si_aid_composition,
// 			   &key,
// 			   pos);
// }

bool
automan::si_process ()
{
  for (si_list::iterator pos = m_si_index.begin ();
       pos != m_si_index.end ();
       ++pos) {
    switch (pos->order_type) {
    case CREATE:
      {
	if (*pos->aid_ptr == -1) {
	  /* Create an automaton. */
	  order_create_init (&m_last_order,
			     pos->create.descriptor,
			     pos->create.ctor_arg);
	  m_last_sequence = *pos;
	  return true;
	}
      }
      break;
    case DECLARE:
      {
	if (*pos->flag_ptr == false) {
	  /* We need to declare a parameter. */
	  order_declare_init (&m_last_order,
			      pos->declare.param);
	  m_last_sequence = *pos;
	  return true;
	}
      }
      break;
    case COMPOSE:
      {
      	if (*pos->flag_ptr == false &&
      	    *pos->compose.out_aid_ptr != -1 &&
      	    si_param_declared (pos->compose.out_param) &&
      	    *pos->compose.in_aid_ptr != -1 &&
      	    si_param_declared (pos->compose.in_param)) {
      	  /* We need to compose. */
      	  order_compose_init (&m_last_order,
      			      *pos->compose.out_aid_ptr,
      			      pos->compose.output,
      			      pos->compose.out_param,
      			      *pos->compose.in_aid_ptr,
      			      pos->compose.input,
      			      pos->compose.in_param);
      	  m_last_sequence = *pos;
      	  return true;
      	}
      }
      break;
    case DECOMPOSE:
      {
	if (*pos->flag_ptr == true) {
	  /* We need to decompose. */
	  order_decompose_init (&m_last_order,
				*pos->compose.out_aid_ptr,
				pos->compose.output,
				pos->compose.out_param,
				*pos->compose.in_aid_ptr,
				pos->compose.input,
				pos->compose.in_param);
	  m_last_sequence = *pos;
	  return true;
	}
      }
      break;
    case RESCIND:
      {
	if (*pos->flag_ptr == true) {
	  /* We need to rescind a parameter. */
	  order_rescind_init (&m_last_order,
			      pos->declare.param);
	  m_last_sequence = *pos;
	  return true;
	}
      }
      break;
    case DESTROY:
      {
	if (*pos->aid_ptr != -1) {
	  /* Create an automaton. */
	  order_destroy_init (&m_last_order,
			      *pos->aid_ptr);
	  m_last_sequence = *pos;
	  return true;
	}
      }
      break;
    }
  }

  return false;
}

void
automan::si_balance_compose (bool* flag_ptr)
{
  /* Find the compose. */
  si_list::iterator compose_pos = std::find_if (m_si_index.begin (),
						m_si_index.end (),
						si_flag_ptr_equal (flag_ptr));
  assert (compose_pos != m_si_index.end ());
  assert (compose_pos->order_type == COMPOSE);
  
  ++compose_pos;
  
  /* Find the decompose. */
  si_list::iterator decompose_pos = std::find_if (compose_pos,
						  m_si_index.end (),
						  si_flag_ptr_equal (flag_ptr));
  if (decompose_pos == m_si_index.end ()) {
    /* Decompose doesn't exist so add one. */
    assert (decompose (flag_ptr) == 0);
  }
}

automan::si_list::iterator
automan::si_decompose_flag_ptr (bool* flag_ptr,
				receipt_type_t receipt)
{
  /* Remove the compose. */
  si_list::iterator compose_pos = std::find_if (m_si_index.begin (),
						m_si_index.end (),
						si_flag_ptr_equal (flag_ptr));
  assert (compose_pos != m_si_index.end ());
  assert (compose_pos->order_type == COMPOSE);
  sequence_item_t compose_item = *compose_pos;
  compose_pos = m_si_index.erase (compose_pos);

  /* Remove the decompose. */
  si_list::iterator decompose_pos = std::find_if (compose_pos,
						  m_si_index.end (),
						  si_flag_ptr_equal (flag_ptr));
  assert (decompose_pos != m_si_index.end ());
  assert (decompose_pos->order_type == DECOMPOSE);
  decompose_pos = m_si_index.erase (decompose_pos);

  /* Set the flag. */
  *compose_item.flag_ptr = false;

  if (compose_item.handler != NULL) {
    compose_item.handler (m_state, compose_item.hparam, receipt);
  }

  return compose_pos;
}

void
automan::si_balance_create (aid_t* aid_ptr)
{
  /* Find the create. */
  si_list::iterator create_pos = std::find_if (m_si_index.begin (),
					       m_si_index.end (),
					       si_aid_ptr_equal (aid_ptr));
  assert (create_pos != m_si_index.end ());
  assert (create_pos->order_type == CREATE);
  
  ++create_pos;
  
  /* Find the destroy. */
  si_list::iterator destroy_pos = std::find_if (create_pos,
						m_si_index.end (),
						si_aid_ptr_equal (aid_ptr));
  if (destroy_pos == m_si_index.end ()) {
    /* Destroy doesn't exist so add one. */
    assert (destroy (aid_ptr) == 0);
  }
}

automan::si_list::iterator
automan::si_rescind_flag_ptr (bool* flag_ptr,
			      receipt_type_t receipt)
{
  /* Remove the declare. */
  si_list::iterator declare_pos = std::find_if (m_si_index.begin (),
						m_si_index.end (),
						si_flag_ptr_equal (flag_ptr));
  assert (declare_pos != m_si_index.end ());
  assert (declare_pos->order_type == DECLARE);
  sequence_item_t declare_item = *declare_pos;
  declare_pos = m_si_index.erase (declare_pos);

  /* Remove the rescind. */
  si_list::iterator rescind_pos = std::find_if (declare_pos,
						m_si_index.end (),
						si_flag_ptr_equal (flag_ptr));
  assert (rescind_pos != m_si_index.end ());
  assert (rescind_pos->order_type == RESCIND);
  rescind_pos = m_si_index.erase (rescind_pos);

  /* Set the flag. */
  *declare_item.flag_ptr = false;

  if (declare_item.handler != NULL) {
    declare_item.handler (m_state, declare_item.hparam, receipt);
  }

  return declare_pos;
}

void
automan::si_destroy_aid_ptr (aid_t* aid_ptr)
{
  /* Remove the create. */
  si_list::iterator create_pos = std::find_if (m_si_index.begin (),
					       m_si_index.end (),
					       si_aid_ptr_equal (aid_ptr));
  assert (create_pos != m_si_index.end ());
  assert (create_pos->order_type == CREATE);
  sequence_item_t create_item = *create_pos;
  create_pos = m_si_index.erase (create_pos);
  
  /* Remove the destroy. */
  si_list::iterator destroy_pos = std::find_if (create_pos,
						m_si_index.end (),
						si_aid_ptr_equal (aid_ptr));
  assert (destroy_pos != m_si_index.end ());
  assert (destroy_pos->order_type == DESTROY);
  if (create_pos == destroy_pos) {
    /* Nothing between create and destroy. Skip over. */
    ++create_pos;
    /* The next line will make them equal again. */
  }
  destroy_pos = m_si_index.erase (destroy_pos);
  
  /* Remove all compositions between create_pos and destroy_pos that use this aid. */
  si_list::iterator compose_pos = create_pos;
  for (;;) {
    /* Find a composition that uses this aid. */
    compose_pos = std::find_if (compose_pos,
				destroy_pos,
				si_composition_aid_ptr_equal (create_item.aid_ptr));
    if (compose_pos != destroy_pos) {
      /* Found one.  Decompose. */
      compose_pos = si_decompose_flag_ptr (compose_pos->flag_ptr,
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
    create_item.handler (m_state, create_item.hparam, CHILD_DESTROYED);
  }
}

int
automan::create (aid_t* aid_ptr,
		 const descriptor_t* descriptor,
		 const void* ctor_arg,
		 automan_handler_t handler,
		 void* hparam)
{
  assert (aid_ptr != NULL);
  assert (descriptor != NULL);
  assert (hparam == NULL || handler != NULL);

  if (!si_closed_aid_ptr (aid_ptr)) {
    /* Duplicate create. */
    return -1;
  }

  if (std::find_if (m_si_index.begin (),
		    m_si_index.end (),
		    si_aid_ptr_equal (aid_ptr)) == m_si_index.end ()) {
    /* Clear the aid. */
    *aid_ptr = -1;
  }
  
  /* Add an action item. */
  si_append_create (aid_ptr,
		    descriptor,
		    ctor_arg,
		    handler,
		    hparam);
  
  if (*m_self_ptr != -1) {
    assert (schedule_system_output () == 0);
  }
  
  return 0;
}

// int
// automan_declare (automan_t* automan,
// 		 bool* flag_ptr,
// 		 void* param,
// 		 automan_handler_t handler,
// 		 void* hparam)
// {
//   assert (automan != NULL);
//   assert (flag_ptr != NULL);
//   assert (param != NULL);
//   assert (hparam == NULL || handler != NULL);

//   if (!si_closed_flag_ptr (automan, flag_ptr) ||
//       !si_closed_param (automan, param)) {
//     /* Duplicate declare. */
//     return -1;
//   }

//   if (si_find_flag_ptr (automan,
// 			flag_ptr,
// 			index_begin (m_si_index),
// 			index_end (m_si_index),
// 			NULL) == NULL) {
//     /* Clear the flag. */
//     *flag_ptr = false;
//   }
  
//   /* Add an action item. */
//   si_append_declare (automan,
// 		  flag_ptr,
// 		  param,
// 		  handler,
// 		  hparam);

//   if (*m_self_ptr != -1) {
//     assert (schedule_system_output () == 0);
//   }
  
//   return 0;
// }

// int
// automan_compose (automan_t* automan,
// 		 bool* flag_ptr,
// 		 aid_t* out_aid_ptr,
// 		 output_t output,
// 		 void* out_param,
// 		 aid_t* in_aid_ptr,
// 		 input_t input,
// 		 void* in_param,
// 		 automan_handler_t handler,
// 		 void* hparam)
// {
//   assert (automan != NULL);
//   assert (flag_ptr != NULL);
//   assert (out_aid_ptr != NULL);
//   assert (output != NULL);
//   assert (in_aid_ptr != NULL);
//   assert (input != NULL);
//   assert (out_param == NULL || in_param == NULL);
//   assert (out_param == NULL || m_self_ptr == out_aid_ptr);
//   assert (in_param == NULL || m_self_ptr == in_aid_ptr);

//   assert (hparam == NULL || handler != NULL);
  
//   /* Check for parameter management. */
//   if (out_param != NULL &&
//       !si_open_param (automan, out_param)) {
//     return -1;
//   }
//   else if (in_param != NULL &&
// 	   !si_open_param (automan, in_param)) {
//     return -1;
//   }

//   if (!si_closed_flag_ptr (automan, flag_ptr) ||
//       !si_closed_input (automan, in_aid_ptr, input, in_param)) {
//     /* Duplicate composition. */
//     return -1;
//   }

//   if (si_find_flag_ptr (automan,
// 			flag_ptr,
// 			index_begin (m_si_index),
// 			index_end (m_si_index),
// 			NULL) == NULL) {
//     /* Clear the flag. */
//     *flag_ptr = false;
//   }
  
//   si_append_compose (automan,
// 		  flag_ptr,
// 		  out_aid_ptr,
// 		  output,
// 		  out_param,
// 		  in_aid_ptr,
// 		  input,
// 		  in_param,
// 		  handler,
// 		  hparam);

//   if (*m_self_ptr != -1) {
//     assert (schedule_system_output () == 0);
//   }
  
//   return 0;
// }

int
automan::decompose (bool* flag_ptr)
{
  assert (flag_ptr != NULL);
  
  if (!si_open_flag_ptr_compose (flag_ptr)) {
    /* Duplicate decompose. */
    return -1;
  }
  
  /* Find the compose. */
  si_list::reverse_iterator pos = std::find_if (m_si_index.rbegin (),
						m_si_index.rend (),
						si_flag_ptr_equal (flag_ptr));
  si_append_decompose (pos->flag_ptr,
		       pos->compose.out_aid_ptr,
		       pos->compose.output,
		       pos->compose.out_param,
		       pos->compose.in_aid_ptr,
		       pos->compose.input,
		       pos->compose.in_param,
		       pos->handler,
		       pos->hparam);
  
  if (*m_self_ptr != -1) {
    assert (schedule_system_output () == 0);
  }
  
  return 0;
}

// int
// automan_rescind (automan_t* automan,
// 		 bool* flag_ptr)
// {
//   assert (automan != NULL);
//   assert (flag_ptr != NULL);

//   if (!si_open_flag_ptr_declare (automan, flag_ptr)) {
//     /* Duplicate rescind. */
//     return -1;
//   }
  
//   /* Find the declare. */
//   riterator_t declare_pos;
//   sequence_item_t* sequence_item = si_rfind_flag_ptr (automan,
// 						      flag_ptr,
// 						      &declare_pos);

//   /* Every composition that uses the parameter must be paired with a decomposition. */
//   iterator_t compose_pos = riterator_reverse (m_si_index, declare_pos);
//   compose_pos = index_advance (m_si_index, compose_pos);
//   for (;;) {
//     /* Find a composition using this parameter. */
//     sequence_item_t* composition = si_find_composition_param (automan,
// 							   flag_ptr,
// 							   sequence_item->declare.param,
// 							   compose_pos,
// 							   &compose_pos);

//     if (composition != NULL) {
//       /* Found a composition using this parameter. Look for the decomposition. */
//       sequence_item_t* decomposition = si_find_flag_ptr (automan,
// 							 composition->flag_ptr,
// 							 index_advance (m_si_index, compose_pos),
// 							 index_end (m_si_index),
// 							 NULL);
//       if (decomposition == NULL) {
//       	/* No corresponding decomposition so add one. */
// 	si_append_decompose (automan,
// 			  composition->flag_ptr,
// 			  composition->compose.out_aid_ptr,
// 			  composition->compose.output,
// 			  composition->compose.out_param,
// 			  composition->compose.in_aid_ptr,
// 			  composition->compose.input,
// 			  composition->compose.in_param,
// 			  composition->handler,
// 			  composition->hparam);
// 	/* Technically, we don't need to add the decompositions.
// 	   We could just rescind the parameter and let the system tell us about all of the decompositions.
// 	   However, it would violate the invariant that the user could extend the sequence by leaving unpaired compositions.
// 	*/
//       }
      
//       /* Advance. */
//       compose_pos = index_advance (m_si_index, compose_pos);
//     }
//     else {
//       /* No subsequent compositions use this paramter.  Done. */
//       break;
//     }
//   }
  
//   /* Add an action item. */
//   si_append_rescind (automan,
// 		  sequence_item->flag_ptr,
// 		  sequence_item->declare.param,
// 		  sequence_item->handler,
// 		  sequence_item->hparam);

//   if (*m_self_ptr != -1) {
//     assert (schedule_system_output () == 0);
//   }
  
//   return 0;
// }

int
automan::destroy (aid_t* aid_ptr)
{
  assert (aid_ptr != NULL);

  if (!si_open_aid_ptr_create (aid_ptr)) {
    /* Duplicate destroy. */
    return -1;
  }

  /* Find the create. */
  si_list::reverse_iterator create_pos = std::find_if (m_si_index.rbegin (),
						       m_si_index.rend (),
						       si_aid_ptr_equal (aid_ptr));

  /* Every composition that uses this aid must be paired with a decomposition. */
  si_list::iterator compose_pos = create_pos.base ()++;
  ++compose_pos;

  for (;;) {
    /* Find a composition using this aid. */
    compose_pos = std::find_if (compose_pos,
				m_si_index.end (),
				si_composition_aid_ptr_equal (aid_ptr));
    if (compose_pos != m_si_index.end ()) {
      /* Found a composition using this aid. Look for the decomposition. */
      si_list::iterator decomposition_pos = compose_pos;
      ++decomposition_pos;
      decomposition_pos = std::find_if (decomposition_pos,
					m_si_index.end (),
					si_flag_ptr_equal (compose_pos->flag_ptr));
      if (decomposition_pos == m_si_index.end ()) {
      	/* No corresponding decomposition so add one. */
  	si_append_decompose (compose_pos->flag_ptr,
			     compose_pos->compose.out_aid_ptr,
			     compose_pos->compose.output,
			     compose_pos->compose.out_param,
			     compose_pos->compose.in_aid_ptr,
			     compose_pos->compose.input,
			     compose_pos->compose.in_param,
			     compose_pos->handler,
			     compose_pos->hparam);
  	/* Technically, we don't need to add the decompositions.
  	   We could just destroy the automaton and let the system tell us about all of the decompositions.
  	   However, it would violate the invariant that the user could extend the sequence by leaving unpaired compositions.
  	*/
      }
      
      /* Advance. */
      compose_pos = ++compose_pos;
    }
    else {
      /* No subsequent compositions use this paramter.  Done. */
      break;
    }
  }
  
  /* Add an action item. */
  si_append_destroy (create_pos->aid_ptr,
		     create_pos->handler,
		     create_pos->hparam);

  if (*m_self_ptr != -1) {
    assert (schedule_system_output () == 0);
  }

  return 0;
}

// void
// automan_self_destruct (automan_t* automan)
// {
//   assert (automan != NULL);

//   /* Add an action item. */
//   si_append_destroy (automan,
// 		     m_self_ptr,
// 		     NULL,
// 		     NULL);
  
//   if (*m_self_ptr != -1) {
//     assert (schedule_system_output () == 0);
//   }
// }

bid_t
automan::action ()
{
  switch (m_sequence_status) {
  case NORMAL:
    {
      if (si_process ()) {
	bid_t bid = buffer_alloc (sizeof (order_t));
	order_t* order = (order_t*)buffer_write_ptr (bid);
	*order = m_last_order;
	m_sequence_status = OUTSTANDING;
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
