#include <mftp.h>

#include <automan.h>

#include <stdlib.h>
#include <assert.h>

#include "udp_sender.h"
#include "msg_sender_proxy.h"

typedef struct {
  proxy_request_t proxy_request;
  aid_t aid;
  bool declared;
  bool composed;
} proxy_t;

typedef struct {
  aid_t self;
  automan_t* automan;

  udp_sender_create_arg_t udp_sender_create_arg;
  aid_t udp_sender;
  bool packet_out_composed;

  bidq_t* bidq;
} msg_sender_t;

static bid_t msg_sender_packet_out (void* state, void* param);
static void msg_sender_message_in (void* state, void* param, bid_t bid);
static void msg_sender_proxy_declared (void* state, void* param, receipt_type_t receipt);
static void msg_sender_proxy_created (void* state, void* param, receipt_type_t receipt);

static void*
msg_sender_create (const void* a)
{
  const msg_sender_create_arg_t* arg = a;
  assert (arg != NULL);

  msg_sender_t* msg_sender = malloc (sizeof (msg_sender_t));

  msg_sender->automan = automan_creat (msg_sender,
				       &msg_sender->self);
  msg_sender->udp_sender_create_arg.port = arg->port;
  assert (automan_create (msg_sender->automan,
			  &msg_sender->udp_sender,
			  &udp_sender_descriptor,
			  &msg_sender->udp_sender_create_arg,
			  NULL,
			  NULL) == 0);
  assert (automan_compose (msg_sender->automan,
			   &msg_sender->packet_out_composed,
			   &msg_sender->self,
			   msg_sender_packet_out,
			   NULL,
			   &msg_sender->udp_sender,
			   udp_sender_packet_in,
			   NULL,
			   NULL,
			   NULL) == 0);

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

  automan_apply (msg_sender->automan, receipt);
}

static bid_t
msg_sender_system_output (void* state, void* param)
{
  msg_sender_t* msg_sender = state;
  assert (msg_sender != NULL);

  return automan_action (msg_sender->automan);
}

void
msg_sender_request_proxy (void* state, void* param, bid_t bid)
{
  msg_sender_t* msg_sender = state;
  assert (msg_sender != NULL);

  assert (buffer_size (bid) == sizeof (proxy_request_t));
  const proxy_request_t* proxy_request = buffer_read_ptr (bid);

  proxy_t* proxy = malloc (sizeof (proxy_t));

  proxy->proxy_request = *proxy_request;

  assert (automan_declare (msg_sender->automan,
			   &proxy->declared,
			   proxy,
			   msg_sender_proxy_declared,
			   proxy) == 0);
  assert (automan_create (msg_sender->automan,
			  &proxy->aid,
			  &msg_sender_proxy_descriptor,
			  NULL,
			  msg_sender_proxy_created,
			  proxy) == 0);
  assert (automan_compose (msg_sender->automan,
			   &proxy->composed,
			   &proxy->aid,
			   msg_sender_proxy_message_out,
			   NULL,
			   &msg_sender->self,
			   msg_sender_message_in,
			   proxy,
			   NULL,
			   NULL) == 0);
}

static void
msg_sender_proxy_declared (void* state, void* param, receipt_type_t receipt)
{
  msg_sender_t* msg_sender = state;
  assert (msg_sender != NULL);
  
  proxy_t* proxy = param;
  assert (proxy != NULL);
  
  if (receipt == DECLARED) {
    /* Yay! */
  }
  else if (receipt == RESCINDED) {
    free (proxy);
  }
  else {
    assert (0);
  }
}

static void
msg_sender_proxy_created (void* state, void* param, receipt_type_t receipt)
{
  msg_sender_t* msg_sender = state;
  assert (msg_sender != NULL);

  proxy_t* proxy = param;
  assert (proxy != NULL);

  if (receipt == CHILD_CREATED) {
    assert (automan_proxy_send_created (proxy->aid, -1, &proxy->proxy_request) == 0);
  }
  else if (receipt == CHILD_DESTROYED) {
    assert (automan_rescind (msg_sender->automan,
			     &proxy->declared) == 0);
  }
  else {
    assert (0);
  }
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

static input_t msg_sender_free_inputs[] = { msg_sender_request_proxy, NULL };
static input_t msg_sender_inputs[] = { msg_sender_message_in, NULL };
static output_t msg_sender_outputs[] = { msg_sender_packet_out, NULL };

descriptor_t msg_sender_descriptor = {
  .constructor = msg_sender_create,
  .system_input = msg_sender_system_input,
  .system_output = msg_sender_system_output,
  .free_inputs = msg_sender_free_inputs,
  .inputs = msg_sender_inputs,
  .outputs = msg_sender_outputs,
};
