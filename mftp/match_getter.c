#include "match_getter.h"

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
  mftp_FileID_t meta_fileid;
  file_server_create_arg_t meta_arg;
  aid_t meta_aid;

  mftp_FileID_t file_fileid;
  file_server_create_arg_t file_arg;
  aid_t file_aid;

  meta_item_t* next;
};

typedef struct {
  mftp_File_t* query;
  meta_item_t* metas;

  manager_t* manager;
  aid_t self;
  aid_t* msg_sender;
  aid_t* msg_receiver;
} match_getter_t;

static void*
match_getter_create (void* a)
{
  match_getter_create_arg_t* arg = a;
  assert (arg != NULL);
  assert (arg->query != NULL);
  assert (arg->msg_sender != NULL);
  assert (arg->msg_receiver != NULL);

  match_getter_t* match_getter = malloc (sizeof (match_getter_t));
  match_getter->query = arg->query;
  match_getter->metas = NULL;

  match_getter->manager = manager_create ();
  match_getter->msg_sender = arg->msg_sender;
  match_getter->msg_receiver = arg->msg_receiver;

  manager_self_set (match_getter->manager,
		    &match_getter->self);

  manager_composition_add (match_getter->manager,
			   match_getter->msg_receiver,
			   msg_receiver_match_out,
			   NULL,
			   &match_getter->self,
			   match_getter_match_in,
			   NULL);

  return match_getter;
}

static void
match_getter_system_input (void* state, void* param, bid_t bid)
{
  assert (state != NULL);
  match_getter_t* match_getter = state;

  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));
  const receipt_t* receipt = buffer_read_ptr (bid);

  manager_apply (match_getter->manager, receipt);
}

static bid_t
match_getter_system_output (void* state, void* param)
{
  match_getter_t* match_getter = state;
  assert (match_getter != NULL);

  return manager_action (match_getter->manager);
}

void
match_getter_match_in (void* state, void* param, bid_t bid)
{
  match_getter_t* match_getter = state;
  assert (match_getter != NULL);

  assert (buffer_size (bid) == sizeof (mftp_Message_t));
  const mftp_Message_t* message = buffer_read_ptr (bid);
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
	meta = malloc (sizeof (meta_item_t));
	
	meta->meta_aid = -1;
	meta->file_aid = -1;

	manager_param_add (match_getter->manager, meta);
	
	memcpy (&meta->meta_fileid, &message->announcement.fileid, sizeof (mftp_FileID_t));
	meta->meta_arg.file = mftp_File_create_empty (&meta->meta_fileid);
	meta->meta_arg.announce = true;
	meta->meta_arg.download = true;
	meta->meta_arg.msg_sender = match_getter->msg_sender;
	meta->meta_arg.msg_receiver = match_getter->msg_receiver;
	manager_child_add (match_getter->manager,
			   &meta->meta_aid,
			   &file_server_descriptor,
			   &meta->meta_arg,
			   NULL,
			   NULL);
	manager_dependency_add (match_getter->manager,
				match_getter->msg_sender,
				&meta->meta_aid,
				file_server_strobe_in,
				buffer_alloc (0));
	manager_dependency_add (match_getter->manager,
				match_getter->msg_receiver,
				&meta->meta_aid,
				file_server_strobe_in,
				buffer_alloc (0));
	manager_composition_add (match_getter->manager,
				 &meta->meta_aid,
				 file_server_download_complete_out,
				 NULL,
				 &match_getter->self,
				 match_getter_meta_download_complete,
				 meta);
	
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
  match_getter_t* match_getter = state;
  assert (match_getter != NULL);

  meta_item_t* meta = param;
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
      manager_child_add (match_getter->manager,
			 &meta->file_aid,
			 &file_server_descriptor,
			 &meta->file_arg,
			 NULL,
			 NULL);
      manager_dependency_add (match_getter->manager,
			      match_getter->msg_sender,
			      &meta->file_aid,
			      file_server_strobe_in,
			      buffer_alloc (0));
      manager_dependency_add (match_getter->manager,
			      match_getter->msg_receiver,
			      &meta->file_aid,
			      file_server_strobe_in,
			      buffer_alloc (0));
      manager_composition_add (match_getter->manager,
			       &meta->file_aid,
			       file_server_download_complete_out,
			       NULL,
			       &match_getter->self,
			       match_getter_file_download_complete,
			       meta);
    
      assert (schedule_system_output () == 0);
    }
  }

}

static void
match_getter_file_download_complete (void* state, void* param, bid_t bid)
{
  match_getter_t* match_getter = state;
  assert (match_getter != NULL);

  meta_item_t* meta = param;
  assert (meta != NULL);

  /* Form the filename. */
  char* filename = malloc (match_getter->query->fileid.size + 1 + 2*HASH_SIZE + 1);
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

  if (write (fd, meta->file_arg.file->data, meta->file_fileid.size) != meta->file_fileid.size) {
    perror ("write");
    exit (EXIT_FAILURE);
  }

  if (close (fd) == -1) {
    perror ("close");
    exit (EXIT_FAILURE);
  }

  free (filename);
}

void
match_getter_strobe_in (void* state, void* param, bid_t bid)
{
  match_getter_t* match_getter = state;
  assert (match_getter != NULL);

  assert (schedule_system_output () == 0);
}

static input_t match_getter_free_inputs[] = {
  match_getter_strobe_in,
  NULL
};

static input_t match_getter_inputs[] = {
  match_getter_match_in,
  match_getter_meta_download_complete,
  match_getter_file_download_complete,
  NULL
};

descriptor_t match_getter_descriptor = {
  .constructor = match_getter_create,
  .system_input = match_getter_system_input,
  .system_output = match_getter_system_output,
  .free_inputs = match_getter_free_inputs,
  .inputs = match_getter_inputs,
};
