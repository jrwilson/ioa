#include "msg_sender_proxy.h"

#include <mftp.h>

#include <automan.h>

#include <stdlib.h>
#include <assert.h>

static void msg_sender_proxy_composed (void* state, void* param, receipt_type_t receipt);

typedef struct {
  aid_t self;
  automan_t* automan;
  bool composed;

  bidq_t* bidq;
} msg_sender_proxy_t;

static void*
msg_sender_proxy_create (const void* a)
{
  msg_sender_proxy_t* msg_sender_proxy = (msg_sender_proxy_t*)malloc (sizeof (msg_sender_proxy_t));

  msg_sender_proxy->automan = automan_creat (msg_sender_proxy,
					     &msg_sender_proxy->self);
  assert (automan_input_add (msg_sender_proxy->automan,
			     &msg_sender_proxy->composed,
			     msg_sender_proxy_message_in,
			     NULL,
			     msg_sender_proxy_composed,
			     NULL) == 0);

  /* Initialize the queue. */
  msg_sender_proxy->bidq = bidq_create ();
  return msg_sender_proxy;
}

static void
msg_sender_proxy_system_input (void* state, void* param, bid_t bid)
{
  msg_sender_proxy_t* msg_sender_proxy = (msg_sender_proxy_t*)state;
  assert (msg_sender_proxy != NULL);

  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));
  const receipt_t* receipt = (const receipt_t*)buffer_read_ptr (bid);

  automan_apply (msg_sender_proxy->automan, receipt);
}

static bid_t
msg_sender_proxy_system_output (void* state, void* param)
{
  msg_sender_proxy_t* msg_sender_proxy = (msg_sender_proxy_t*)state;
  assert (msg_sender_proxy != NULL);

  return automan_action (msg_sender_proxy->automan);
}

static void
msg_sender_proxy_composed (void* state, void* param, receipt_type_t receipt)
{
  msg_sender_proxy_t* msg_sender_proxy = (msg_sender_proxy_t*)state;
  assert (msg_sender_proxy != NULL);

  if (receipt == INPUT_COMPOSED) {
    /* Yay. */
  }
  else if (receipt == INPUT_DECOMPOSED) {
    automan_self_destruct (msg_sender_proxy->automan);
  }
  else {
    assert (0);
  }
}

void
msg_sender_proxy_message_in (void* state, void* param, bid_t bid)
{
  msg_sender_proxy_t* msg_sender_proxy = (msg_sender_proxy_t*)state;
  assert (msg_sender_proxy != NULL);

  /* Enqueue the item. */
  buffer_incref (bid);
  bidq_push_back (msg_sender_proxy->bidq, bid);
  
  assert (schedule_output (msg_sender_proxy_message_out, NULL) == 0);
}

bid_t
msg_sender_proxy_message_out (void* state, void* param)
{
  msg_sender_proxy_t* msg_sender_proxy = (msg_sender_proxy_t*)state;
  assert (msg_sender_proxy != NULL);

  if (!bidq_empty (msg_sender_proxy->bidq)) {
    bid_t bid = bidq_front (msg_sender_proxy->bidq);
    bidq_pop_front (msg_sender_proxy->bidq);

    /* Go again. */
    assert (schedule_output (msg_sender_proxy_message_out, NULL) == 0);

    return bid;
  }
  else {
    return -1;
  }
}

static input_t msg_sender_proxy_inputs[] = { msg_sender_proxy_message_in, NULL };
static output_t msg_sender_proxy_outputs[] = { msg_sender_proxy_message_out, NULL };

const descriptor_t msg_sender_proxy_descriptor = {
  msg_sender_proxy_create,
  msg_sender_proxy_system_input,
  msg_sender_proxy_system_output,
  NULL,
  NULL,
  NULL,
  NULL,
  msg_sender_proxy_inputs,
  msg_sender_proxy_outputs,
  NULL,
};
