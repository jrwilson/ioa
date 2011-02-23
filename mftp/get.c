#include <mftp.h>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "ft.h"

static bid_t composer_new_comm_out (void*, void*);

typedef struct {
  manager_t* manager;
  aid_t self;

  aid_t msg_sender;
  aid_t msg_receiver;

  file_server_create_arg_t query_arg;
  aid_t query;
} composer_t;

typedef struct {
  mftp_File_t* query;
} composer_create_arg_t;

static void*
composer_create (void* a)
{
  composer_create_arg_t* arg = a;
  assert (arg != NULL);
  assert (arg->query != NULL);

  composer_t* composer = malloc (sizeof (composer_t));

  composer->manager = manager_create ();

  manager_self_set (composer->manager, &composer->self);

  manager_child_add (composer->manager, &composer->msg_sender, &msg_sender_descriptor, NULL);
  manager_child_add (composer->manager, &composer->msg_receiver, &msg_receiver_descriptor, NULL);

  composer->query_arg.file = arg->query;
  composer->query_arg.announce = true;
  composer->query_arg.download = false;
  composer->query_arg.msg_sender = &composer->msg_sender;
  composer->query_arg.msg_receiver = &composer->msg_receiver;
  manager_child_add (composer->manager, &composer->query, &file_server_descriptor, &composer->query_arg);
  manager_composition_add (composer->manager, &composer->self, composer_new_comm_out, NULL, &composer->query, file_server_new_comm_in, NULL);

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

  if (composer->msg_sender != -1 &&
      composer->msg_receiver != -1) {
    assert (schedule_output (composer_new_comm_out, NULL) == 0);
  }
}

static bid_t
composer_system_output (void* state, void* param)
{
  composer_t* composer = state;
  assert (composer != NULL);

  return manager_action (composer->manager);
}

static bid_t
composer_new_comm_out (void* state, void* param)
{
  return buffer_alloc (0);
}

static output_t composer_outputs[] = {
  composer_new_comm_out,
  NULL
};

descriptor_t composer_descriptor = {
  .constructor = composer_create,
  .system_input = composer_system_input,
  .system_output = composer_system_output,
  .outputs = composer_outputs,
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
