#include "decls.h"

#include <assert.h>

static bool
ii_flag_ptr_equal (const void* x0,
		   const void* y0)
{
  const input_item_t* x = (const input_item_t*)x0;
  const input_item_t* y = (const input_item_t*)y0;

  return x->flag_ptr == y->flag_ptr;
}

static input_item_t*
ii_find_flag_ptr (automan_t* automan,
		  bool* flag_ptr)
{
  input_item_t key;
  key.flag_ptr = flag_ptr;
  
  return (input_item_t*)index_find_value (automan->ii_index,
					  index_begin (automan->ii_index),
					  index_end (automan->ii_index),
					  ii_flag_ptr_equal,
					  &key,
					  NULL);
}

static bool
ii_input_equal (const void* x0,
		const void* y0)
{
  const input_item_t* x = (const input_item_t*)x0;
  const input_item_t* y = (const input_item_t*)y0;
  
  return
    x->input == y->input &&
    x->in_param == y->in_param;
}

input_item_t*
ii_find_input (automan_t* automan,
	       input_t input,
	       void* in_param)
{
  input_item_t key;
  key.input = input;
  key.in_param = in_param;
  
  return (input_item_t*)index_find_value (automan->ii_index,
					  index_begin (automan->ii_index),
					  index_end (automan->ii_index),
					  ii_input_equal,
					  &key,
					  NULL);
}

static void
ii_append_input (automan_t* automan,
		 bool* flag_ptr,
		 input_t input,
		 void* in_param,
		 automan_handler_t handler,
		 void* hparam)
{
  input_item_t input_item;
  input_item.handler = handler;
  input_item.hparam = hparam;
  input_item.flag_ptr = flag_ptr;
  input_item.input = input;
  input_item.in_param = in_param;

  index_push_back (automan->ii_index, &input_item);
}

int
automan_input_add (automan_t* automan,
		   bool* flag_ptr,
		   input_t input,
		   void* in_param,
		   automan_handler_t handler,
		   void* hparam)
{
  assert (automan != NULL);
  assert (flag_ptr != NULL);
  assert (input != NULL);
  assert (hparam == NULL || handler != NULL);

  if (ii_find_flag_ptr (automan,
			flag_ptr) != NULL ||
      ii_find_input (automan,
		     input,
		     in_param) != NULL) {
    /* Already tracking. */
    return -1;
  }

  /* Clear the flag. */
  *flag_ptr = false;

  ii_append_input (automan,
		   flag_ptr,
		   input,
		   in_param,
		   handler,
		   hparam);

  return 0;
}
