#include <mftp.h>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "ft.h"

#include "matcher.h"

typedef struct {
  manager_t* manager;
  aid_t self;

  aid_t msg_sender;
  aid_t msg_receiver;

  matcher_create_arg_t matcher_arg;
  aid_t matcher;
} composer_t;

static void*
composer_create (const void* a)
{
  composer_t* composer = malloc (sizeof (composer_t));

  composer->manager = manager_create ();

  manager_self_set (composer->manager,
		    &composer->self);

  manager_child_add (composer->manager,
		     &composer->msg_sender,
		     &msg_sender_descriptor,
		     NULL,
		     NULL,
		     NULL);
  manager_child_add (composer->manager,
		     &composer->msg_receiver,
		     &msg_receiver_descriptor,
		     NULL,
		     NULL,
		     NULL);

  composer->matcher_arg.msg_sender = &composer->msg_sender;
  composer->matcher_arg.msg_receiver = &composer->msg_receiver;
  manager_child_add (composer->manager,
		     &composer->matcher,
		     &matcher_descriptor,
		     &composer->matcher_arg,
		     NULL,
		     NULL);
  manager_dependency_add (composer->manager,
			  &composer->msg_sender,
			  &composer->matcher,
			  matcher_strobe_in,
			  buffer_alloc (0));
  manager_dependency_add (composer->manager,
			  &composer->msg_receiver,
			  &composer->matcher,
			  matcher_strobe_in,
			  buffer_alloc (0));

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


int
main (int argc, char* argv[])
{
  ueioa_run (&composer_descriptor, NULL, 1);
  exit (EXIT_SUCCESS);
}
