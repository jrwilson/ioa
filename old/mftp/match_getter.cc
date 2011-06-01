#include "match_getter.h"

#include <automan.h>

#include "ft.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

static void match_getter_meta_download_complete (void* state, void* param, bid_t bid);
static void match_getter_file_download_complete (void* state, void* param, bid_t bid);

static void match_getter_match_in (void*, void*, bid_t);

typedef struct meta_item_struct meta_item_t;
struct meta_item_struct {
  bool declared;
  mftp_FileID_t meta_fileid;
  file_server_create_arg_t meta_arg;
  aid_t meta_aid;
  bool meta_download_complete_composed;

  mftp_FileID_t file_fileid;
  file_server_create_arg_t file_arg;
  aid_t file_aid;
  bool file_download_complete_composed;

  meta_item_t* next;
};

typedef struct {
  mftp_File_t* query;
  meta_item_t* metas;

  automan_t* automan;
  aid_t self;
  aid_t msg_sender;
  aid_t msg_receiver;
  bool match_in_composed;
} match_getter_t;

static void*
match_getter_create (const void* a)
{
  const match_getter_create_arg_t* arg = (const match_getter_create_arg_t*)a;
  assert (arg != NULL);
  assert (arg->query != NULL);
  assert (arg->msg_sender != -1);
  assert (arg->msg_receiver != -1);

  match_getter_t* match_getter = (match_getter_t*)malloc (sizeof (match_getter_t));
  match_getter->query = arg->query;
  match_getter->metas = NULL;

  match_getter->automan = automan_creat (match_getter,
					 &match_getter->self);
  match_getter->msg_sender = arg->msg_sender;
  match_getter->msg_receiver = arg->msg_receiver;

  assert (automan_compose (match_getter->automan,
			   &match_getter->match_in_composed,
			   &match_getter->msg_receiver,
			   msg_receiver_match_out,
			   NULL,
			   &match_getter->self,
			   match_getter_match_in,
			   NULL,
			   NULL,
			   NULL) == 0);

  return match_getter;
}

static void
match_getter_system_input (void* state, void* param, bid_t bid)
{
  assert (state != NULL);
  match_getter_t* match_getter = (match_getter_t*)state;

  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));
  const receipt_t* receipt = (const receipt_t*)buffer_read_ptr (bid);

  automan_apply (match_getter->automan, receipt);
}

static bid_t
match_getter_system_output (void* state, void* param)
{
  match_getter_t* match_getter = (match_getter_t*)state;
  assert (match_getter != NULL);

  return automan_action (match_getter->automan);
}

void
match_getter_match_in (void* state, void* param, bid_t bid)
{
  match_getter_t* match_getter = (match_getter_t*)state;
  assert (match_getter != NULL);

  assert (buffer_size (bid) == sizeof (mftp_Message_t));
  const mftp_Message_t* message = (const mftp_Message_t*)buffer_read_ptr (bid);
  assert (message->header.type == MATCH);

  if (message->match.fileid1.type == META && message->match.fileid2.type == QUERY) {
    /* Is the query our query? */
    if (mftp_FileID_cmp (&message->match.fileid2, &match_getter->query->fileid) == 0) {
      /* Look for the meta file. */
      meta_item_t* meta;
      for (meta = match_getter->metas;
	   meta != NULL &&
	     mftp_FileID_cmp (&message->match.fileid1, &meta->meta_fileid) != 0;
	   meta = meta->next)
	;;

      if (meta == NULL) {
        /* New meta file. */
	/* Add it to the list. */
	meta = (meta_item_t*)malloc (sizeof (meta_item_t));
	
	meta->meta_aid = -1;
	meta->file_aid = -1;

	assert (automan_declare (match_getter->automan,
				 &meta->declared,
				 meta,
				 NULL,
				 NULL) == 0);
	
	memcpy (&meta->meta_fileid, &message->announcement.fileid, sizeof (mftp_FileID_t));
	meta->meta_arg.file = mftp_File_create_empty (&meta->meta_fileid);
	meta->meta_arg.announce = true;
	meta->meta_arg.download = true;
	meta->meta_arg.msg_sender = match_getter->msg_sender;
	meta->meta_arg.msg_receiver = match_getter->msg_receiver;
	assert (automan_create (match_getter->automan,
				&meta->meta_aid,
				&file_server_descriptor,
				&meta->meta_arg,
				NULL,
				NULL) == 0);
	assert (automan_compose (match_getter->automan,
				 &meta->meta_download_complete_composed,
				 &meta->meta_aid,
				 file_server_download_complete_out,
				 NULL,
				 &match_getter->self,
				 match_getter_meta_download_complete,
				 meta,
				 NULL,
				 NULL) == 0);
	
	meta->next = match_getter->metas;
	match_getter->metas = meta;
	
	assert (schedule_system_output () == 0);
      }
    }
  }
}

static void
match_getter_meta_download_complete (void* state, void* param, bid_t bid)
{
  match_getter_t* match_getter = (match_getter_t*)state;
  assert (match_getter != NULL);

  meta_item_t* meta = (meta_item_t*)param;
  assert (meta != NULL);

  /* Match again. */
  if (match_getter->query->fileid.size == meta->meta_fileid.size - sizeof (mftp_FileID_t) &&
      memcmp (match_getter->query->data, meta->meta_arg.file->data + sizeof (mftp_FileID_t), match_getter->query->fileid.size) == 0) {
    /* Download. */

    /* Convert the ID. */
    mftp_FileID_netToHost (&meta->file_fileid, (mftp_FileID_t*)meta->meta_arg.file->data);

    if (meta->file_fileid.type == FILEC) {
      meta->file_arg.file = mftp_File_create_empty (&meta->file_fileid);
      meta->file_arg.announce = true;
      meta->file_arg.download = true;
      meta->file_arg.msg_sender = match_getter->msg_sender;
      meta->file_arg.msg_receiver = match_getter->msg_receiver;
      assert (automan_create (match_getter->automan,
			      &meta->file_aid,
			      &file_server_descriptor,
			      &meta->file_arg,
			      NULL,
			      NULL) == 0);
      assert (automan_compose (match_getter->automan,
			       &meta->file_download_complete_composed,
			       &meta->file_aid,
			       file_server_download_complete_out,
			       NULL,
			       &match_getter->self,
			       match_getter_file_download_complete,
			       meta,
			       NULL,
			       NULL) == 0);
    
      assert (schedule_system_output () == 0);
    }
  }

}

static void
match_getter_file_download_complete (void* state, void* param, bid_t bid)
{
  match_getter_t* match_getter = (match_getter_t*)state;
  assert (match_getter != NULL);

  meta_item_t* meta = (meta_item_t*)param;
  assert (meta != NULL);

  /* Form the filename. */
  char* filename = (char*)malloc (match_getter->query->fileid.size + 1 + 2*HASH_SIZE + 1);
  char* ptr = filename;
  memcpy (ptr, match_getter->query->data, match_getter->query->fileid.size);
  ptr += match_getter->query->fileid.size;
  *ptr = '-';
  ++ptr;
  int i;
  for (i = 0; i < HASH_SIZE; ++i) {
    sprintf (ptr, "%02x", meta->file_fileid.hash[i]);
    ptr += 2;
  }
  *ptr = 0;

  printf ("%s %u\n", filename, meta->file_fileid.size);

  int fd = open (filename, O_WRONLY|O_TRUNC|O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
  if (fd == -1) {
    perror ("open");
    exit (EXIT_FAILURE);
  }

  if (write (fd, meta->file_arg.file->data, meta->file_fileid.size) != ssize_t(meta->file_fileid.size)) {
    perror ("write");
    exit (EXIT_FAILURE);
  }

  if (close (fd) == -1) {
    perror ("close");
    exit (EXIT_FAILURE);
  }

  free (filename);
}

static input_t match_getter_inputs[] = {
  match_getter_match_in,
  match_getter_meta_download_complete,
  match_getter_file_download_complete,
  NULL
};

descriptor_t match_getter_descriptor = {
  match_getter_create,
  match_getter_system_input,
  match_getter_system_output,
  NULL,
  NULL,
  NULL,
  NULL,
  match_getter_inputs,
  NULL,
  NULL
};
