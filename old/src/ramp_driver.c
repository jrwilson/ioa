#include <ccm.h>
#include <automan.h>
#include <mftp.h>

#include "ramp.h"
#include "ramp_proxy.h"

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#define PORT 64470

static void composer_sender_receiver_created (void*, void*, receipt_type_t);

typedef struct {
  automan_t* automan;

  aid_t self;

  msg_sender_create_arg_t msg_sender_arg;
  aid_t msg_sender;
  msg_receiver_create_arg_t msg_receiver_arg;
  aid_t msg_receiver;

  uuid_t id;
  component_create_arg_t component_arg;
  aid_t component;
} composer_t;

static void
composer_sender_receiver_created (void* state, void* param, receipt_type_t receipt)
{
  composer_t* composer = state;
  assert (composer != NULL);
  assert (receipt == CHILD_CREATED);

  if (composer->msg_sender != -1 &&
      composer->msg_receiver != -1) {
  uuid_generate (composer->id);
  component_create_arg_init (&composer->component_arg,
			     &ramp_descriptor,
			     NULL,
			     ramp_request_proxy,
			     composer->id,
			     ramp_port_descriptors,
			     composer->msg_sender,
			     composer->msg_receiver);
  assert (automan_create (composer->automan,
  			  &composer->component,
  			  &component_descriptor,
  			  &composer->component_arg,
  			  NULL,
  			  NULL) == 0);
  }
}

static void*
composer_create (const void* arg)
{
  composer_t* composer = malloc (sizeof (composer_t));

  composer->automan = automan_creat (composer,
				     &composer->self);

  composer->msg_sender_arg.port = PORT;
  assert (automan_create (composer->automan,
			  &composer->msg_sender,
			  &msg_sender_descriptor,
			  &composer->msg_sender_arg,
			  composer_sender_receiver_created,
			  NULL) == 0);

  composer->msg_receiver_arg.port = PORT;
  assert (automan_create (composer->automan,
			  &composer->msg_receiver,
			  &msg_receiver_descriptor,
			  &composer->msg_receiver_arg,
			  composer_sender_receiver_created,
			  NULL) == 0);
  return composer;
}

static void
composer_system_input (void* state, void* param, bid_t bid)
{
  composer_t* composer = state;
  assert (composer != NULL);

  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));
  const receipt_t* receipt = buffer_read_ptr (bid);

  automan_apply (composer->automan, receipt);
}

static bid_t
composer_system_output (void* state, void* param)
{
  composer_t* composer = state;
  assert (composer != NULL);

  return automan_action (composer->automan);
}

descriptor_t composer_descriptor = {
  .constructor = composer_create,
  .system_input = composer_system_input,
  .system_output = composer_system_output,
};

int
main (int argc, char** argv)
{
  ueioa_run (&composer_descriptor, NULL, 1);
  exit (EXIT_SUCCESS);
}
