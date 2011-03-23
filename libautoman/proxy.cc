#include "automan.hh"

#include <cassert>
#include <algorithm>

bool
automan::pi_closed_aid_ptr (aid_t* aid_ptr) const
{
  return std::find_if (m_pi_index.rbegin (),
		       m_pi_index.rend (),
		       pi_aid_ptr_equal (aid_ptr)) != m_pi_index.rend ();
}

bid_t
automan::proxy_request_create (bid_t bid,
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
automan::pi_process ()
{
  if (*m_self_ptr != -1 &&
      m_proxy_status == NORMAL) {

    pi_list::iterator pos = m_pi_index.begin ();
    while (pos != m_pi_index.end ()) {
      if (*pos->aid_ptr == -1) {
	/* Request a proxy. */
	bid_t bid = proxy_request_create (pos->bid,
					  *m_self_ptr,
					  pos->callback);
	if (schedule_free_input (pos->source_aid, pos->source_free_input, bid) == 0) {
	  m_last_proxy = *pos;
	  m_proxy_status = OUTSTANDING;
	  break;
	}
	else {
	  /* The free input was not so free. */
	  if (pos->handler != NULL) {
	    pos->handler (m_state, pos->pparam, PROXY_REQUEST_INPUT_DNE, -1);
	  }
	  pos = m_pi_index.erase (pos);
	}
      }
      else {
	++pos;
      }
    }
  }
}

int
automan::proxy_add (aid_t* aid_ptr,
		    aid_t source_aid,
		    input_t source_free_input,
		    bid_t bid,
		    input_t callback,
		    proxy_handler_t handler,
		    void* pparam)
{
  assert (aid_ptr != NULL);
  assert (source_aid != -1);
  assert (source_free_input != NULL);
  assert (callback != NULL);

  if (!pi_closed_aid_ptr (aid_ptr)) {
    /* Duplicate proxy. */
    return -1;
  }

  if (std::find_if (m_pi_index.begin (),
		    m_pi_index.end (),
		    pi_aid_ptr_equal (aid_ptr)) == m_pi_index.end ()) {
    /* Clear the aid. */
    *aid_ptr = -1;
  }

  m_pi_index.push_back (proxy_item_t (aid_ptr, source_aid, source_free_input, bid, callback, handler, pparam));

  pi_process ();

  return 0;
}

int
automan::proxy_send_created (aid_t proxy_aid,
			     bid_t bid,
			     const proxy_request_t& proxy_request)
{
  assert (proxy_aid != -1);

  bid_t b = buffer_alloc (sizeof (proxy_receipt_t));
  proxy_receipt_t* proxy_receipt = (proxy_receipt_t*)buffer_write_ptr (b);
  proxy_receipt->type = PROXY_CREATED;
  proxy_receipt->proxy_aid = proxy_aid;
  proxy_receipt->bid = bid;
  if (bid != -1) {
    buffer_add_child (b, bid);
  }

  return schedule_free_input (proxy_request.callback_aid, proxy_request.callback_free_input, b);
}

int
automan::proxy_send_not_created (bid_t bid,
				 const proxy_request_t& proxy_request)
{
  bid_t b = buffer_alloc (sizeof (proxy_receipt_t));
  proxy_receipt_t* proxy_receipt = (proxy_receipt_t*)buffer_write_ptr (b);
  proxy_receipt->type = PROXY_NOT_CREATED;
  proxy_receipt->proxy_aid = -1;
  proxy_receipt->bid = bid;
  if (bid != -1) {
    buffer_add_child (b, bid);
  }

  return schedule_free_input (proxy_request.callback_aid, proxy_request.callback_free_input, b);
}

int
automan::proxy_send_destroyed (aid_t aid,
			       const proxy_request_t& proxy_request)
{
  assert (aid != -1);
  
  bid_t b = buffer_alloc (sizeof (proxy_receipt_t));
  proxy_receipt_t* proxy_receipt = (proxy_receipt_t*)buffer_write_ptr (b);
  proxy_receipt->type = PROXY_DESTROYED;
  proxy_receipt->proxy_aid = aid;
  proxy_receipt->bid = -1;

  return schedule_free_input (proxy_request.callback_aid, proxy_request.callback_free_input, b);
}

void
automan::proxy_receive (bid_t bid)
{
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
      assert (m_proxy_status == OUTSTANDING);
      /* Set the aid. */
      *m_last_proxy.aid_ptr = proxy_receipt->proxy_aid;
      /* Invoke the handler. */
      if (m_last_proxy.handler != NULL) {
	m_last_proxy.handler (m_state,
				     m_last_proxy.pparam,
				     proxy_receipt->type,
				     proxy_receipt->bid);
      }
      m_proxy_status = NORMAL;
    }
    break;
  case PROXY_NOT_CREATED:
    {
      assert (m_proxy_status == OUTSTANDING);
      /* Remove the proxy. */
      pi_list::iterator pos = std::find_if (m_pi_index.begin (),
					    m_pi_index.end (),
					    pi_aid_ptr_equal (m_last_proxy.aid_ptr));
      m_pi_index.erase (pos);
      /* Invoke the handler. */
      if (m_last_proxy.handler != NULL) {
	m_last_proxy.handler (m_state,
				     m_last_proxy.pparam,
				     proxy_receipt->type,
				     proxy_receipt->bid);
      }
      m_proxy_status = NORMAL;
    }
    break;
  case PROXY_DESTROYED:
    {
      /* Find the proxy. */
      pi_list::iterator pos = std::find_if (m_pi_index.begin (),
					    m_pi_index.end (),
					    pi_receipt_aid_equal (*proxy_receipt));
      assert (pos != m_pi_index.end ());
      proxy_item_t proxy_item (*pos);
      /* Erase the item. */
      m_pi_index.erase (pos);
      /* Set the pointer. */
      *proxy_item.aid_ptr = -1;
      /* Invoke the handler. */
      if (proxy_item.handler != NULL) {
      	proxy_item.handler (m_state,
			    proxy_item.pparam,
			    proxy_receipt->type,
			    -1);
      }
    }
    break;
  }
  
  buffer_decref (bid);
  
  pi_process ();
  
  assert (schedule_system_output () == 0);
}
