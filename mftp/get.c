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
#include "match_getter.h"

static void composer_sender_receiver_created (void* state, void* param, receipt_type_t recipt);

typedef struct {
  automan_t* automan;
  aid_t self;

  msg_sender_create_arg_t msg_sender_arg;
  aid_t msg_sender;
  msg_receiver_create_arg_t msg_receiver_arg;
  aid_t msg_receiver;

  file_server_create_arg_t query_arg;
  aid_t query;

  matcher_create_arg_t matcher_arg;
  aid_t matcher;

  match_getter_create_arg_t getter_arg;
  aid_t getter;
} composer_t;

typedef struct {
  mftp_File_t* query;
} composer_create_arg_t;

static void*
composer_create (const void* a)
{
  const composer_create_arg_t* arg = a;
  assert (arg != NULL);
  assert (arg->query != NULL);

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

  composer->query_arg.file = arg->query;
  composer->query_arg.announce = true;
  composer->query_arg.download = false;

  composer->getter_arg.query = arg->query;

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

  automan_apply (composer->automan, receipt);
}

static bid_t
composer_system_output (void* state, void* param)
{
  composer_t* composer = state;
  assert (composer != NULL);

  return automan_action (composer->automan);
}

static void
composer_sender_receiver_created (void* state, void* param, receipt_type_t receipt)
{
  composer_t* composer = state;
  assert (composer != NULL);
  assert (receipt == CHILD_CREATED);

  if (composer->msg_sender != -1 &&
      composer->msg_receiver != -1) {
    composer->query_arg.msg_sender = composer->msg_sender;
    composer->query_arg.msg_receiver = composer->msg_receiver;
    assert (automan_create (composer->automan,
			    &composer->query,
			    &file_server_descriptor,
			    &composer->query_arg,
			    NULL,
			    NULL) == 0);
    
    composer->matcher_arg.msg_sender = composer->msg_sender;
    composer->matcher_arg.msg_receiver = composer->msg_receiver;
    assert (automan_create (composer->automan,
			    &composer->matcher,
			    &matcher_descriptor,
			    &composer->matcher_arg,
			    NULL,
			    NULL) == 0);
    
    composer->getter_arg.msg_sender = composer->msg_sender;
    composer->getter_arg.msg_receiver = composer->msg_receiver;
    assert (automan_create (composer->automan,
			    &composer->getter,
			    &match_getter_descriptor,
			    &composer->getter_arg,
			    NULL,
			    NULL) == 0);
  }
}

descriptor_t composer_descriptor = {
  .constructor = composer_create,
  .system_input = composer_system_input,
  .system_output = composer_system_output,
};


int
main (int argc, char* argv[])
{
  char* nicename;
  char* content;
  composer_create_arg_t arg;

  if (argc != 2) {
    fprintf (stderr, "usage: %s NAME\n", argv[0]);
    exit (EXIT_FAILURE);
  }

  nicename = argv[1];

  content = malloc (strlen (nicename));
  if (content == NULL) {
    perror ("malloc");
    exit (EXIT_FAILURE);
  }

  char* ptr = content;
  memcpy (ptr, nicename, strlen (nicename));

  arg.query = mftp_File_create_buffer (content, strlen (nicename), QUERY);

  free (content);

  ueioa_run (&composer_descriptor, &arg, 1);
  exit (EXIT_SUCCESS);
}
