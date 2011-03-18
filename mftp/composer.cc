#include "composer.h"

#include <automan.h>

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "mftp.h"
#include "consumer.h"
#include "ft.h"

typedef struct {
  automan_t* automan;
  aid_t self;
  
  msg_receiver_create_arg_t msg_receiver_arg;
  aid_t msg_receiver;

  aid_t consumer;

  bool receiver_announcement_composed;
  bool receiver_match_composed;
  bool receiver_request_composed;
  bool receiver_fragment_composed;
} composer_t;

static void*
composer_create (const void* arg)
{
  composer_t* composer = (composer_t*)malloc (sizeof (composer_t));

  composer->automan = automan_creat (composer,
				     &composer->self);

  composer->msg_receiver_arg.port = PORT;
  assert (automan_create (composer->automan,
			  &composer->msg_receiver,
			  &msg_receiver_descriptor,
			  &composer->msg_receiver_arg,
			  NULL,
			  NULL) == 0);

  assert (automan_create (composer->automan,
			  &composer->consumer,
			  &consumer_descriptor,
			  NULL,
			  NULL,
			  NULL) == 0);
  assert (automan_compose (composer->automan,
			   &composer->receiver_announcement_composed,
			   &composer->msg_receiver,
			   msg_receiver_announcement_out,
			   NULL,
			   &composer->consumer,
			   consumer_announcement_in,
			   NULL,
			   NULL,
			   NULL) == 0);
  assert (automan_compose (composer->automan,
			   &composer->receiver_match_composed,
			   &composer->msg_receiver,
			   msg_receiver_match_out, 
			   NULL, 
			   &composer->consumer, 
			   consumer_match_in, 
			   NULL,
			   NULL,
			   NULL) == 0);
  assert (automan_compose (composer->automan, 
			   &composer->receiver_request_composed,
			   &composer->msg_receiver, 
			   msg_receiver_request_out, 
			   NULL, 
			   &composer->consumer, 
			   consumer_request_in, 
			   NULL,
			   NULL,
			   NULL) == 0);
  assert (automan_compose (composer->automan,
			   &composer->receiver_fragment_composed,
			   &composer->msg_receiver,
			   msg_receiver_fragment_out,
			   NULL,
			   &composer->consumer,
			   consumer_fragment_in,
			   NULL,
			   NULL,
			   NULL) == 0);

  return composer;
}

static void
composer_system_input (void* state, void* param, bid_t bid)
{
  assert (state != NULL);
  composer_t* composer = (composer_t*)state;

  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));
  const receipt_t* receipt = (const receipt_t*)buffer_read_ptr (bid);

  automan_apply (composer->automan, receipt);
}

static bid_t
composer_system_output (void* state, void* param)
{
  composer_t* composer = (composer_t*)state;
  assert (composer != NULL);

  return automan_action (composer->automan);
}

descriptor_t composer_descriptor = {
  composer_create,
  composer_system_input,
  composer_system_output,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL
};
