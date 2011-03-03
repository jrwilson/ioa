#include <mftp.h>

#include <stdlib.h>
#include <assert.h>

#include "udp_sender.h"

typedef struct {
  bidq_t* bidq;
} msg_sender_proxy_t;

static void*
msg_sender_proxy_create (const void* a)
{
  msg_sender_proxy_t* msg_sender_proxy = malloc (sizeof (msg_sender_proxy_t));

  /* Initialize the queue. */
  msg_sender_proxy->bidq = bidq_create ();
  return msg_sender_proxy;
}

static bid_t msg_sender_proxy_message_out (void* state, void* param);

void
msg_sender_proxy_message_in (void* state, void* param, bid_t bid)
{
  msg_sender_proxy_t* msg_sender_proxy = state;
  assert (msg_sender_proxy != NULL);

  /* Enqueue the item. */
  buffer_incref (bid);
  bidq_push_back (msg_sender_proxy->bidq, bid);
  
  assert (schedule_output (msg_sender_proxy_message_out, NULL) == 0);
}

static bid_t
msg_sender_proxy_message_out (void* state, void* param)
{
  msg_sender_proxy_t* msg_sender_proxy = state;
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

static descriptor_t msg_sender_proxy_descriptor = {
  .constructor = msg_sender_proxy_create,
  .inputs = msg_sender_proxy_inputs,
  .outputs = msg_sender_proxy_outputs,
};



typedef struct {
  aid_t aid;
  aid_t callback_aid;
  input_t callback_free_input;
} proxy_t;

typedef struct {
  manager_t* manager;
  aid_t self;
  udp_sender_create_arg_t udp_sender_create_arg;
  aid_t udp_sender;
  bidq_t* bidq;
} msg_sender_t;

static bid_t msg_sender_packet_out (void* state, void* param);
static void msg_sender_message_in (void* state, void* param, bid_t bid);
static void msg_sender_proxy_created (void*, void*);

static void*
msg_sender_create (const void* a)
{
  const msg_sender_create_arg_t* arg = a;
  assert (arg != NULL);

  msg_sender_t* msg_sender = malloc (sizeof (msg_sender_t));

  msg_sender->manager = manager_create ();
  manager_self_set (msg_sender->manager,
		    &msg_sender->self);
  msg_sender->udp_sender_create_arg.port = arg->port;
  manager_child_add (msg_sender->manager,
		     &msg_sender->udp_sender,
		     &udp_sender_descriptor,
		     &msg_sender->udp_sender_create_arg,
		     NULL,
		     NULL);
  manager_composition_add (msg_sender->manager,
			   &msg_sender->self,
			   msg_sender_packet_out,
			   NULL,
			   &msg_sender->udp_sender,
			   udp_sender_packet_in,
			   NULL);

  /* Initialize the queue. */
  msg_sender->bidq = bidq_create ();
  return msg_sender;
}

static void
msg_sender_system_input (void* state, void* param, bid_t bid)
{
  msg_sender_t* msg_sender = state;
  assert (msg_sender != NULL);
  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));
  const receipt_t* receipt = buffer_read_ptr (bid);

  manager_apply (msg_sender->manager, receipt);
}

static bid_t
msg_sender_system_output (void* state, void* param)
{
  msg_sender_t* msg_sender = state;
  assert (msg_sender != NULL);

  return manager_action (msg_sender->manager);
}

void
msg_sender_request_proxy (void* state, void* param, bid_t bid)
{
  msg_sender_t* msg_sender = state;
  assert (msg_sender != NULL);

  assert (buffer_size (bid) == sizeof (proxy_request_t));

  const proxy_request_t* request = buffer_read_ptr (bid);
  proxy_t* proxy = malloc (sizeof (proxy_t));
  proxy->callback_aid = request->callback_aid;
  proxy->callback_free_input = request->callback_free_input;
  manager_param_add (msg_sender->manager,
		     proxy);
  manager_child_add (msg_sender->manager,
		     &proxy->aid,
		     &msg_sender_proxy_descriptor,
		     NULL,
		     msg_sender_proxy_created,
		     proxy);
  manager_composition_add (msg_sender->manager,
			   &proxy->aid,
			   msg_sender_proxy_message_out,
			   NULL,
			   &msg_sender->self,
			   msg_sender_message_in,
			   proxy);
  assert (schedule_system_output () == 0);
}

static void
msg_sender_message_in (void* state, void* param, bid_t bid)
{
  msg_sender_t* msg_sender = state;
  assert (msg_sender != NULL);
  assert (buffer_size (bid) == sizeof (mftp_Message_t));

  const mftp_Message_t* message = buffer_read_ptr (bid);

  /* Calculate the size. */
  uint32_t size = sizeof (mftp_Header_t);
  switch (message->header.type) {
  case ANNOUNCEMENT:
    size += sizeof (mftp_Announcement_t);
    break;
  case MATCH:
    size += sizeof (mftp_Match_t);
    break;
  case REQUEST:
    size += sizeof (mftp_Request_t);
    break;
  case FRAGMENT:
    size += sizeof (mftp_Fragment_t) - FRAGMENT_SIZE + message->fragment.size;
    break;
  default:
    return;
    break;
  }

  bid_t new_bid = buffer_alloc (size);
  mftp_Message_t* new_message = buffer_write_ptr (new_bid);
  mftp_Message_hostToNet (new_message, message);

  /* Enqueue the item. */
  bidq_push_back (msg_sender->bidq, new_bid);
  
  assert (schedule_output (msg_sender_packet_out, NULL) == 0);
}

static bid_t
msg_sender_packet_out (void* state, void* param)
{
  msg_sender_t* msg_sender = state;
  assert (msg_sender != NULL);

  if (!bidq_empty (msg_sender->bidq)) {
    bid_t bid = bidq_front (msg_sender->bidq);
    bidq_pop_front (msg_sender->bidq);

    /* Go again. */
    assert (schedule_output (msg_sender_packet_out, NULL) == 0);

    return bid;
  }
  else {
    return -1;
  }
}

static void
msg_sender_proxy_created (void* state, void* param)
{
  msg_sender_t* msg_sender = state;
  assert (msg_sender != NULL);

  proxy_t* proxy = param;
  assert (proxy != NULL);

  assert (schedule_free_input (proxy->callback_aid, proxy->callback_free_input, proxy_receipt_create (proxy->aid, -1)) == 0);
}


static input_t msg_sender_free_inputs[] = { msg_sender_request_proxy, NULL };
static input_t msg_sender_inputs[] = { msg_sender_message_in, NULL };
static output_t msg_sender_outputs[] = { msg_sender_packet_out, NULL };
static internal_t msg_sender_internals[] = { msg_sender_proxy_created, NULL };

descriptor_t msg_sender_descriptor = {
  .constructor = msg_sender_create,
  .system_input = msg_sender_system_input,
  .system_output = msg_sender_system_output,
  .free_inputs = msg_sender_free_inputs,
  .inputs = msg_sender_inputs,
  .outputs = msg_sender_outputs,
  .internals = msg_sender_internals,
};
