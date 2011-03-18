#include <mftp.h>

#include <automan.h>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "ft.h"

#include "matcher.h"

static void composer_sender_receiver_created (void* state, void* param, receipt_type_t recipt);

typedef struct {
  automan_t* automan;
  aid_t self;

  msg_sender_create_arg_t msg_sender_arg;
  aid_t msg_sender;
  msg_receiver_create_arg_t msg_receiver_arg;
  aid_t msg_receiver;

  matcher_create_arg_t matcher_arg;
  aid_t matcher;
} composer_t;

static void*
composer_create (const void* a)
{
  composer_t* composer = (composer_t*)malloc (sizeof (composer_t));

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

static void
composer_sender_receiver_created (void* state, void* param, receipt_type_t receipt)
{
  composer_t* composer = (composer_t*)state;
  assert (composer != NULL);
  assert (receipt == CHILD_CREATED);

  if (composer->msg_sender != -1 &&
      composer->msg_receiver != -1) {

    composer->matcher_arg.msg_sender = composer->msg_sender;
    composer->matcher_arg.msg_receiver = composer->msg_receiver;
    assert (automan_create (composer->automan,
			    &composer->matcher,
			    &matcher_descriptor,
			    &composer->matcher_arg,
			    NULL,
			    NULL) == 0);
  }
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


int
main (int argc, char* argv[])
{
  ueioa_run (&composer_descriptor, NULL, 1);
  exit (EXIT_SUCCESS);
}
