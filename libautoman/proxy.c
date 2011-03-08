#include "decls.h"

#include <assert.h>

static bool
pi_aid_ptr_equal (const void* x0, const void* y0)
{
  const proxy_item_t* x = x0;
  const proxy_item_t* y = y0;

  return x->aid_ptr == y->aid_ptr;
}

static bool
pi_closed_aid_ptr (automan_t* automan,
		   aid_t* aid_ptr)
{
  proxy_item_t key;
  key.aid_ptr = aid_ptr;
  
  proxy_item_t* proxy_item = index_rfind_value (automan->pi_index,
						index_rbegin (automan->pi_index),
						index_rend (automan->pi_index),
						pi_aid_ptr_equal,
						&key,
						NULL);

  return proxy_item == NULL;
}


static void
pi_append_proxy_add (automan_t* automan,
		     aid_t* aid_ptr,
		     aid_t source_aid,
		     input_t source_free_input,
		     bid_t bid,
		     input_t callback,
		     proxy_handler_t handler,
		     void* pparam)
{
  proxy_item_t proxy_item;
  proxy_item.aid_ptr = aid_ptr;
  proxy_item.source_aid = source_aid;
  proxy_item.source_free_input = source_free_input;
  proxy_item.bid = bid;
  proxy_item.callback = callback;
  proxy_item.handler = handler;
  proxy_item.pparam = pparam;

  index_push_back (automan->pi_index, &proxy_item);
}

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

void
pi_process (automan_t* automan)
{
  if (*automan->self_ptr != -1 &&
      automan->proxy_status == NORMAL) {

    while (!index_empty (automan->pi_index)) {
      proxy_item_t* proxy_item = index_front (automan->pi_index);
      assert (*proxy_item->aid_ptr == -1);
      
      /* Request a proxy. */
      bid_t bid = proxy_request_create (proxy_item->bid,
					*automan->self_ptr,
					proxy_item->callback);
      if (schedule_free_input (proxy_item->source_aid, proxy_item->source_free_input, bid) != 0) {
	/* The free input was not so free. */
	if (proxy_item->handler != NULL) {
	  proxy_item->handler (automan->state, proxy_item->pparam, PROXY_REQUEST_INPUT_DNE);
	}
	index_pop_front (automan->pi_index);
	continue;
      }
      
      automan->proxy_status = OUTSTANDING;
      break;
    }
  }
}

int
automan_proxy_add (automan_t* automan,
		   aid_t* aid_ptr,
		   aid_t source_aid,
		   input_t source_free_input,
		   bid_t bid,
		   input_t callback,
		   proxy_handler_t handler,
		   void* pparam)
{
  assert (automan != NULL);
  assert (aid_ptr != NULL);
  assert (source_aid != -1);
  assert (source_free_input != NULL);
  assert (callback != NULL);

  if (!pi_closed_aid_ptr (automan, aid_ptr)) {
    /* Duplicate proxy. */
    return -1;
  }

  /* Clear the automaton. */
  *aid_ptr = -1;

  pi_append_proxy_add (automan,
		       aid_ptr,
		       source_aid,
		       source_free_input,
		       bid,
		       callback,
		       handler,
		       pparam);

  pi_process (automan);

  return 0;
}

int
automan_proxy_send (aid_t proxy_aid,
		    bid_t bid,
		    const proxy_request_t* proxy_request)
{
  assert (proxy_aid != -1);
  assert (proxy_request != NULL);

  bid_t b = buffer_alloc (sizeof (proxy_receipt_t));
  proxy_receipt_t* proxy_receipt = buffer_write_ptr (b);
  proxy_receipt->proxy_aid = proxy_aid;
  proxy_receipt->bid = bid;
  if (bid != -1) {
    buffer_add_child (b, bid);
  }

  return schedule_free_input (proxy_request->callback_aid, proxy_request->callback_free_input, b);
}

void
automan_proxy_receive (automan_t* automan,
		       const proxy_receipt_t* proxy_receipt)
{
  assert (automan != NULL);
  assert (proxy_receipt != NULL);
  
  assert (automan->proxy_status == OUTSTANDING);

  proxy_item_t* proxy_item = index_front (automan->pi_index);
  *proxy_item->aid_ptr = proxy_receipt->proxy_aid;

  if (proxy_item->handler != NULL) {
    proxy_item->handler (automan->state, proxy_item->pparam, PROXY_CREATED);
  }

  index_pop_front (automan->pi_index);
  automan->proxy_status = NORMAL;

  pi_process (automan);
  
  assert (schedule_system_output () == 0);
}
