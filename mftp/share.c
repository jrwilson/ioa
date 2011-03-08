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
composer_create (const void* a)
{
  const composer_create_arg_t* arg = a;
  assert (arg != NULL);
  assert (arg->file != NULL);
  assert (arg->file != NULL);

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

  composer->file_arg.file = arg->file;
  composer->file_arg.announce = true;
  composer->file_arg.download = false;

  composer->meta_arg.file = arg->meta;
  composer->meta_arg.announce = true;
  composer->meta_arg.download = false;

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

    composer->file_arg.msg_sender = composer->msg_sender;
    composer->file_arg.msg_receiver = composer->msg_receiver;
    assert (automan_create (composer->automan,
			    &composer->file,
			    &file_server_descriptor,
			    &composer->file_arg,
			    NULL,
			    NULL) == 0);
    
    composer->meta_arg.msg_sender = composer->msg_sender;
    composer->meta_arg.msg_receiver = composer->msg_receiver;
    assert (automan_create (composer->automan,
			    &composer->meta,
			    &file_server_descriptor,
			    &composer->meta_arg,
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
