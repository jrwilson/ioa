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

static bid_t composer_new_comm_out (void*, void*);

typedef struct {
  manager_t* manager;
  aid_t self;

  aid_t msg_sender;
  aid_t msg_receiver;

  file_server_create_arg_t file_arg;
  aid_t file;

  file_server_create_arg_t meta_arg;
  aid_t meta;

  matcher_create_arg_t matcher_arg;
  aid_t matcher;
} composer_t;

typedef struct {
  mftp_File_t* file;
  mftp_File_t* meta;
} composer_create_arg_t;

static void*
composer_create (void* a)
{
  composer_create_arg_t* arg = a;
  assert (arg != NULL);
  assert (arg->file != NULL);
  assert (arg->file != NULL);

  composer_t* composer = malloc (sizeof (composer_t));

  composer->manager = manager_create ();

  manager_self_set (composer->manager, &composer->self);

  manager_child_add (composer->manager, &composer->msg_sender, &msg_sender_descriptor, NULL);
  manager_child_add (composer->manager, &composer->msg_receiver, &msg_receiver_descriptor, NULL);

  composer->file_arg.file = arg->file;
  composer->file_arg.announce = true;
  composer->file_arg.download = false;
  composer->file_arg.msg_sender = &composer->msg_sender;
  composer->file_arg.msg_receiver = &composer->msg_receiver;
  manager_child_add (composer->manager, &composer->file, &file_server_descriptor, &composer->file_arg);
  manager_composition_add (composer->manager, &composer->self, composer_new_comm_out, NULL, &composer->file, file_server_new_comm_in, NULL);

  composer->meta_arg.file = arg->meta;
  composer->meta_arg.announce = true;
  composer->meta_arg.download = false;
  composer->meta_arg.msg_sender = &composer->msg_sender;
  composer->meta_arg.msg_receiver = &composer->msg_receiver;
  manager_child_add (composer->manager, &composer->meta, &file_server_descriptor, &composer->meta_arg);
  manager_composition_add (composer->manager, &composer->self, composer_new_comm_out, NULL, &composer->meta, file_server_new_comm_in, NULL);

  composer->matcher_arg.msg_sender = &composer->msg_sender;
  composer->matcher_arg.msg_receiver = &composer->msg_receiver;
  manager_child_add (composer->manager, &composer->matcher, &matcher_descriptor, &composer->matcher_arg);
  manager_composition_add (composer->manager, &composer->self, composer_new_comm_out, NULL, &composer->matcher, matcher_new_comm_in, NULL);

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
  char* filename;
  char* nicename;
  struct stat stat_buf;
  size_t length;
  char* content;
  mftp_FileID_t fileid;
  composer_create_arg_t arg;

  if (!(argc == 2 || argc == 3)) {
    fprintf (stderr, "usage: %s FILE [NAME]\n", argv[0]);
    exit (EXIT_FAILURE);
  }

  filename = argv[1];
  nicename = filename;
  if (argc == 3) {
    nicename = argv[2];
  }

  int fd = open (filename, O_RDONLY);
  if (fd == -1) {
    perror ("open");
    exit (EXIT_FAILURE);
  }

  if (fstat (fd, &stat_buf) == -1) {
    perror ("fstat");
    exit (EXIT_FAILURE);
  }

  length = stat_buf.st_size;
  content = malloc (length);
  if (content == NULL) {
    perror ("malloc");
    exit (EXIT_FAILURE);
  }

  if (read (fd, content, length) != length) {
    perror ("read");
    exit (EXIT_FAILURE);
  }

  arg.file = mftp_File_create_buffer (content, length, FILEC);

  content = realloc (content, sizeof (mftp_FileID_t) + strlen (nicename));
  if (content == NULL) {
    perror ("realloc");
    exit (EXIT_FAILURE);
  }

  char* ptr = content;
  mftp_FileID_hostToNet (&fileid, &arg.file->fileid);
  memcpy (ptr, &fileid, sizeof (mftp_FileID_t));
  ptr += sizeof (mftp_FileID_t);
  memcpy (ptr, nicename, strlen (nicename));

  arg.meta = mftp_File_create_buffer (content, sizeof (mftp_FileID_t) + strlen (nicename), META);

  free (content);

  ueioa_run (&composer_descriptor, &arg, 1);
  exit (EXIT_SUCCESS);
}
