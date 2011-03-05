#include "composer.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "mftp.h"
#include "consumer.h"
#include "ft.h"

typedef struct {
  manager_t* manager;
  aid_t self;
  
  msg_receiver_create_arg_t msg_receiver_arg;
  aid_t msg_receiver;

  aid_t consumer;
} composer_t;

static void*
composer_create (const void* arg)
{
  composer_t* composer = malloc (sizeof (composer_t));

  composer->manager = manager_create (&composer->self);

  composer->msg_receiver_arg.port = PORT;
  manager_child_add (composer->manager,
		     &composer->msg_receiver,
		     &msg_receiver_descriptor,
		     &composer->msg_receiver_arg,
		     NULL,
		     NULL);

  manager_child_add (composer->manager,
		     &composer->consumer,
		     &consumer_descriptor,
		     NULL,
		     NULL,
		     NULL);
  manager_composition_add (composer->manager,
			   &composer->msg_receiver,
			   msg_receiver_announcement_out,
			   NULL,
			   &composer->consumer,
			   consumer_announcement_in,
			   NULL);
  manager_composition_add (composer->manager,
			   &composer->msg_receiver,
			   msg_receiver_match_out, 
			   NULL, 
			   &composer->consumer, 
			   consumer_match_in, 
			   NULL);
  manager_composition_add (composer->manager, 
			   &composer->msg_receiver, 
			   msg_receiver_request_out, 
			   NULL, 
			   &composer->consumer, 
			   consumer_request_in, 
			   NULL);
  manager_composition_add (composer->manager,
			   &composer->msg_receiver,
			   msg_receiver_fragment_out,
			   NULL,
			   &composer->consumer,
			   consumer_fragment_in,
			   NULL);

  return composer;
}

static void
composer_system_input (void* state, void* param, bid_t bid)
{
  assert (state != NULL);
  composer_t* composer = state;

  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));
  const receipt_t* receipt = buffer_read_ptr (bid);

  manager_apply (composer->manager, receipt);
}

static bid_t
composer_system_output (void* state, void* param)
{
  composer_t* composer = state;
  assert (composer != NULL);

  return manager_action (composer->manager);
}

descriptor_t composer_descriptor = {
  .constructor = composer_create,
  .system_input = composer_system_input,
  .system_output = composer_system_output,
};
