#include "decls.h"

#include <assert.h>

static bool
pi_aid_ptr_equal (const void* x0, const void* y0)
{
  const proxy_item_t* x = (const proxy_item_t*)x0;
  const proxy_item_t* y = (const proxy_item_t*)y0;

  return x->aid_ptr == y->aid_ptr;
}

static bool
pi_aid_equal (const void* x0, const void* y0)
{
  const proxy_item_t* x = (const proxy_item_t*)x0;
  const proxy_receipt_t* y = (const proxy_receipt_t*)y0;

  return *x->aid_ptr == y->proxy_aid;
}

static bool
pi_closed_aid_ptr (automan_t* automan,
		   aid_t* aid_ptr)
{
  proxy_item_t key;
  key.aid_ptr = aid_ptr;
  
  proxy_item_t* proxy_item = (proxy_item_t*)index_rfind_value (automan->pi_index,
							       index_rbegin (automan->pi_index),
							       index_rend (automan->pi_index),
							       pi_aid_ptr_equal,
							       &key,
							       NULL);

  return proxy_item == NULL;
}

static proxy_item_t*
pi_find_aid_ptr (automan_t* automan,
		 aid_t* aid_ptr,
		 iterator_t begin,
		 iterator_t end,
		 iterator_t* iterator)
{
  proxy_item_t key;
  key.aid_ptr = aid_ptr;
  
  return (proxy_item_t*)index_find_value (automan->pi_index,
			   begin,
			   end,
			   pi_aid_ptr_equal,
			   &key,
			   iterator);
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
  proxy_request_t* proxy_request = (proxy_request_t*)buffer_write_ptr (b);
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

    iterator_t iterator = index_begin (automan->pi_index);
    while (iterator_ne (iterator, index_end (automan->pi_index))) {
      proxy_item_t* proxy_item = (proxy_item_t*)index_value (automan->pi_index, iterator);

      if (*proxy_item->aid_ptr == -1) {
	/* Request a proxy. */
	bid_t bid = proxy_request_create (proxy_item->bid,
					  *automan->self_ptr,
					  proxy_item->callback);
	if (schedule_free_input (proxy_item->source_aid, proxy_item->source_free_input, bid) == 0) {
	  automan->last_proxy = *proxy_item;
	  automan->proxy_status = OUTSTANDING;
	  break;
	}
	else {
	  /* The free input was not so free. */
	  if (proxy_item->handler != NULL) {
	    proxy_item->handler (automan->state, proxy_item->pparam, PROXY_REQUEST_INPUT_DNE, -1);
	  }
	  iterator = index_erase (automan->pi_index, iterator);
	}
      }
      else {
	iterator = index_advance (automan->pi_index, iterator);
      }
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

  if (pi_find_aid_ptr (automan,
		       aid_ptr,
		       index_begin (automan->pi_index),
		       index_end (automan->pi_index),
		       NULL) == NULL) {
    /* Clear the aid. */
    *aid_ptr = -1;
  }

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
automan_proxy_send_created (aid_t proxy_aid,
			    bid_t bid,
			    const proxy_request_t* proxy_request)
{
  assert (proxy_aid != -1);
  assert (proxy_request != NULL);

  bid_t b = buffer_alloc (sizeof (proxy_receipt_t));
  proxy_receipt_t* proxy_receipt = (proxy_receipt_t*)buffer_write_ptr (b);
  proxy_receipt->type = PROXY_CREATED;
  proxy_receipt->proxy_aid = proxy_aid;
  proxy_receipt->bid = bid;
  if (bid != -1) {
    buffer_add_child (b, bid);
  }

  return schedule_free_input (proxy_request->callback_aid, proxy_request->callback_free_input, b);
}

int
automan_proxy_send_not_created (bid_t bid,
				const proxy_request_t* proxy_request)
{
  assert (proxy_request != NULL);
  
  bid_t b = buffer_alloc (sizeof (proxy_receipt_t));
  proxy_receipt_t* proxy_receipt = (proxy_receipt_t*)buffer_write_ptr (b);
  proxy_receipt->type = PROXY_NOT_CREATED;
  proxy_receipt->proxy_aid = -1;
  proxy_receipt->bid = bid;
  if (bid != -1) {
    buffer_add_child (b, bid);
  }

  return schedule_free_input (proxy_request->callback_aid, proxy_request->callback_free_input, b);
}

int
automan_proxy_send_destroyed (aid_t aid,
			      const proxy_request_t* proxy_request)
{
  assert (aid != -1);
  assert (proxy_request != NULL);
  
  bid_t b = buffer_alloc (sizeof (proxy_receipt_t));
  proxy_receipt_t* proxy_receipt = (proxy_receipt_t*)buffer_write_ptr (b);
  proxy_receipt->type = PROXY_DESTROYED;
  proxy_receipt->proxy_aid = aid;
  proxy_receipt->bid = -1;

  return schedule_free_input (proxy_request->callback_aid, proxy_request->callback_free_input, b);
}

void
automan_proxy_receive (automan_t* automan,
		       bid_t bid)
{
  assert (automan != NULL);
  assert (buffer_size (bid) == sizeof (proxy_receipt_t));

  const proxy_receipt_t* proxy_receipt = (const proxy_receipt_t*)buffer_read_ptr (bid);

  /* So we can play with the argument buffer. */
  buffer_incref (bid);

  switch (proxy_receipt->type) {
  case PROXY_REQUEST_INPUT_DNE:
    /* This should not be encountered during a receive. */
    assert (0);
    break;
  case PROXY_CREATED:
    {
      assert (automan->proxy_status == OUTSTANDING);
      /* Set the aid. */
      *automan->last_proxy.aid_ptr = proxy_receipt->proxy_aid;
      /* Invoke the handler. */
      if (automan->last_proxy.handler != NULL) {
	automan->last_proxy.handler (automan->state,
				     automan->last_proxy.pparam,
				     proxy_receipt->type,
				     proxy_receipt->bid);
      }
      automan->proxy_status = NORMAL;
    }
    break;
  case PROXY_NOT_CREATED:
    {
      assert (automan->proxy_status == OUTSTANDING);
      /* Remove the proxy. */
      iterator_t iterator = index_find (automan->pi_index,
					index_begin (automan->pi_index),
					index_end (automan->pi_index),
					pi_aid_ptr_equal,
					&automan->last_proxy);
      index_erase (automan->pi_index, iterator);
      /* Invoke the handler. */
      if (automan->last_proxy.handler != NULL) {
	automan->last_proxy.handler (automan->state,
				     automan->last_proxy.pparam,
				     proxy_receipt->type,
				     proxy_receipt->bid);
      }
      automan->proxy_status = NORMAL;
    }
    break;
  case PROXY_DESTROYED:
    {
      /* Find the proxy. */
      iterator_t iterator = index_find (automan->pi_index,
					index_begin (automan->pi_index),
					index_end (automan->pi_index),
					pi_aid_equal,
					proxy_receipt);
      assert (iterator_ne (iterator, index_end (automan->pi_index)));
      proxy_item_t proxy_item = *(proxy_item_t*)index_value (automan->pi_index, iterator);
      /* Erase the item. */
      index_erase (automan->pi_index, iterator);
      /* Set the pointer. */
      *proxy_item.aid_ptr = -1;
      /* Invoke the handler. */
      if (proxy_item.handler != NULL) {
      	proxy_item.handler (automan->state,
			    proxy_item.pparam,
			    proxy_receipt->type,
			    -1);
      }
    }
    break;
  }

  buffer_decref (bid);

  pi_process (automan);
  
  assert (schedule_system_output () == 0);
}
