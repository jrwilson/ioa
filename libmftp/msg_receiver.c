#include <mftp.h>

#include <stdlib.h>
#include <assert.h>

#include "udp_receiver.h"

typedef struct {
  manager_t* manager;
  aid_t self;
  aid_t udp_receiver;
  bidq_t* announcements;
  bidq_t* matches;
  bidq_t* requests;
  bidq_t* fragments;
} msg_receiver_t;

static void msg_receiver_packet_in (void* state, void* param, bid_t bid);

static void*
msg_receiver_create (void* arg)
{
  msg_receiver_t* msg_receiver = malloc (sizeof (msg_receiver_t));

  msg_receiver->manager = manager_create ();

  manager_self_set (msg_receiver->manager,
		    &msg_receiver->self);
  manager_child_add (msg_receiver->manager,
		     &msg_receiver->udp_receiver,
		     &udp_receiver_descriptor,
		     NULL,
		     NULL,
		     NULL);
  manager_composition_add (msg_receiver->manager,
			   &msg_receiver->udp_receiver,
			   udp_receiver_packet_out,
			   NULL,
			   &msg_receiver->self,
			   msg_receiver_packet_in,
			   NULL);

  /* Initialize the queue. */
  msg_receiver->announcements = bidq_create ();
  msg_receiver->matches = bidq_create ();
  msg_receiver->requests = bidq_create ();
  msg_receiver->fragments = bidq_create ();
  return msg_receiver;
}

static void
msg_receiver_system_input (void* state, void* param, bid_t bid)
{
  msg_receiver_t* msg_receiver = state;
  assert (msg_receiver != NULL);
  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));
  const receipt_t* receipt = buffer_read_ptr (bid);

  manager_apply (msg_receiver->manager, receipt);
}

static bid_t
msg_receiver_system_output (void* state, void* param)
{
  msg_receiver_t* msg_receiver = state;
  assert (msg_receiver != NULL);

  return manager_action (msg_receiver->manager);
}

static void
msg_receiver_packet_in (void* state, void* param, bid_t bid)
{
  msg_receiver_t* msg_receiver = state;
  assert (msg_receiver != NULL);
  assert (bid != -1);

  const mftp_Message_t* message = buffer_read_ptr (bid);
  uint32_t bytesRemaining = buffer_size (bid);

  bid_t new_bid = buffer_alloc (sizeof (mftp_Message_t));
  mftp_Message_t* new_message = buffer_write_ptr (new_bid);

  if (mftp_Message_netToHost (new_message, message, bytesRemaining) == 0) {
    switch (new_message->header.type) {
    case ANNOUNCEMENT:
      bidq_push_back (msg_receiver->announcements, new_bid);
      assert (schedule_output (msg_receiver_announcement_out, NULL) == 0);
      break;
    case MATCH:
      bidq_push_back (msg_receiver->matches, new_bid);
      assert (schedule_output (msg_receiver_match_out, NULL) == 0);
      break;
    case REQUEST:
      bidq_push_back (msg_receiver->requests, new_bid);
      assert (schedule_output (msg_receiver_request_out, NULL) == 0);
      break;
    case FRAGMENT:
      bidq_push_back (msg_receiver->fragments, new_bid);
      assert (schedule_output (msg_receiver_fragment_out, NULL) == 0);
      break;
    }
  }
  else {
    /* Axe the buffer. */
    buffer_decref (new_bid);
  }
}

bid_t
msg_receiver_announcement_out (void* state, void* param)
{
  msg_receiver_t* msg_receiver = state;
  assert (msg_receiver != NULL);

  if (!bidq_empty (msg_receiver->announcements)) {
    bid_t bid = bidq_front (msg_receiver->announcements);
    bidq_pop_front (msg_receiver->announcements);
    assert (schedule_output (msg_receiver_announcement_out, NULL) == 0);
    return bid;
  }
  else {
    return -1;
  }
}

bid_t
msg_receiver_match_out (void* state, void* param)
{
  msg_receiver_t* msg_receiver = state;
  assert (msg_receiver != NULL);

  if (!bidq_empty (msg_receiver->matches)) {
    bid_t bid = bidq_front (msg_receiver->matches);
    bidq_pop_front (msg_receiver->matches);
    assert (schedule_output (msg_receiver_match_out, NULL) == 0);
    return bid;
  }
  else {
    return -1;
  }
}

bid_t
msg_receiver_request_out (void* state, void* param)
{
  msg_receiver_t* msg_receiver = state;
  assert (msg_receiver != NULL);

  if (!bidq_empty (msg_receiver->requests)) {
    bid_t bid = bidq_front (msg_receiver->requests);
    bidq_pop_front (msg_receiver->requests);
    assert (schedule_output (msg_receiver_request_out, NULL) == 0);
    return bid;
  }
  else {
    return -1;
  }
}

bid_t
msg_receiver_fragment_out (void* state, void* param)
{
  msg_receiver_t* msg_receiver = state;
  assert (msg_receiver != NULL);

  if (!bidq_empty (msg_receiver->fragments)) {
    bid_t bid = bidq_front (msg_receiver->fragments);
    bidq_pop_front (msg_receiver->fragments);
    assert (schedule_output (msg_receiver_fragment_out, NULL) == 0);
    return bid;
  }
  else {
    return -1;
  }
}

static input_t msg_receiver_inputs[] = { msg_receiver_packet_in, NULL };
static output_t msg_receiver_outputs[] = { msg_receiver_announcement_out, msg_receiver_match_out, msg_receiver_request_out, msg_receiver_fragment_out, NULL };

descriptor_t msg_receiver_descriptor = {
  .constructor = msg_receiver_create,
  .system_input = msg_receiver_system_input,
  .system_output = msg_receiver_system_output,
  .inputs = msg_receiver_inputs,
  .outputs = msg_receiver_outputs,
};
