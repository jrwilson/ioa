#include "decls.h"

#include <assert.h>

static bool
oi_flag_ptr_equal (const void* x0,
		   const void* y0)
{
  const output_item_t* x = x0;
  const output_item_t* y = y0;

  return x->flag_ptr == y->flag_ptr;
}

static output_item_t*
oi_find_flag_ptr (automan_t* automan,
		  bool* flag_ptr)
{
  output_item_t key;
  key.flag_ptr = flag_ptr;
  
  return index_find_value (automan->oi_index,
			   index_begin (automan->oi_index),
			   index_end (automan->oi_index),
			   oi_flag_ptr_equal,
			   &key,
			   NULL);
}

static bool
oi_output_equal (const void* x0,
		 const void* y0)
{
  const output_item_t* x = x0;
  const output_item_t* y = y0;
  
  return
    x->output == y->output &&
    x->out_param == y->out_param;
}

output_item_t*
oi_find_output (automan_t* automan,
		output_t output,
		void* out_param)
{
  output_item_t key;
  key.output = output;
  key.out_param = out_param;
  
  return index_find_value (automan->oi_index,
			   index_begin (automan->oi_index),
			   index_end (automan->oi_index),
			   oi_output_equal,
			   &key,
			   NULL);
}

static void
append_output (automan_t* automan,
	      bool* flag_ptr,
	      output_t output,
	      void* out_param,
	      automan_handler_t handler,
	      void* hparam)
{
  output_item_t output_item;
  output_item.handler = handler;
  output_item.hparam = hparam;
  output_item.flag_ptr = flag_ptr;
  output_item.output = output;
  output_item.out_param = out_param;

  index_push_back (automan->oi_index, &output_item);
}

int
automan_output_add (automan_t* automan,
		    bool* flag_ptr,
		    output_t output,
		    void* out_param,
		    automan_handler_t handler,
		    void* hparam)
{
  assert (automan != NULL);
  assert (flag_ptr != NULL);
  assert (output != NULL);
  assert (hparam == NULL || handler != NULL);
  
  if (oi_find_flag_ptr (automan,
			flag_ptr) != NULL ||
      oi_find_output (automan,
		      output,
		      out_param) != NULL) {
    /* Already tracking. */
    return -1;
  }

  /* Clear the flag. */
  *flag_ptr = false;
  
  append_output (automan,
		 flag_ptr,
		 output,
		 out_param,
		 handler,
		 hparam);

  return 0;
}
