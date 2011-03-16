#include <mftp.h>

#include <automan.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "bitset.h"

static void file_server_callback (void* state, void* param, bid_t bid);

static void file_server_announcement_alarm_in (void*, void*, bid_t);
static bid_t file_server_announcement_alarm_out (void*, void*);
static void file_server_announcement_alarm_composed (void*, void*, receipt_type_t);

static void file_server_request_alarm_in (void*, void*, bid_t);
static bid_t file_server_request_alarm_out (void*, void*);
static void file_server_request_alarm_composed (void*, void*, receipt_type_t);

static void file_server_fragment_alarm_in (void*, void*, bid_t);
static bid_t file_server_fragment_alarm_out (void*, void*);
static void file_server_fragment_alarm_composed (void*, void*, receipt_type_t);

static void file_server_announcement_in (void*, void*, bid_t);
static void file_server_request_in (void*, void*, bid_t);
static void file_server_fragment_in (void*, void*, bid_t);
static bid_t file_server_message_out (void*, void*);

static void file_server_download_complete_composed (void*, void*, receipt_type_t);

#define OFFSET_TO_FRAGMENT(offset) ((offset) >> FRAGMENT_SIZE_LOG2)
#define FRAGMENT_TO_OFFSET(fragment) ((fragment) << FRAGMENT_SIZE_LOG2)

/* In seconds. */
#define INIT_ANNOUNCE_INTERVAL 1
#define MAX_ANNOUNCE_INTERVAL 8 /*4096*/
#define REQUEST_INTERVAL 1

/* In microseconds. */
#define FRAGMENT_INTERVAL 1000

typedef struct {
  mftp_File_t* file;		/* The file we are working with. */
  bool announce;		/* Are we announcing the file? */
  bool download;		/* Are we downloading the file? */
  bool downloaded_sent;		/* Have we performed the downloaded_out action? */
  time_t announcement_interval;	/* Announcement interval (seconds). */
  time_t next_announce;		/* Announcement must be after this time. */
  bidq_t* bidq;			/* Queue of outgoing messages. */
  bitset_t* requests;		/* Bitset of requested fragments. */
  size_t request_idx;		/* Index into the requests. */
  bitset_t* fragments;		/* Bitset of acquired fragments. */
  size_t fragment_idx;		/* Index into the fragments. */

  automan_t* automan;
  aid_t self;

  aid_t announcement_alarm;
  bool announcement_alarm_in_composed;
  bool announcement_alarm_out_composed;

  aid_t request_alarm;
  bool request_alarm_in_composed;
  bool request_alarm_out_composed;

  aid_t fragment_alarm;
  bool fragment_alarm_in_composed;
  bool fragment_alarm_out_composed;

  bool download_complete_composed;

  aid_t msg_sender_proxy;
  bool message_out_composed;

  aid_t msg_receiver;
  bool announcement_in_composed;
  bool request_in_composed;
  bool fragment_in_composed;
} file_server_t;

static void*
file_server_create (const void* a)
{
  const file_server_create_arg_t* arg = a;
  assert (arg != NULL);
  assert (arg->file != NULL);
  assert (arg->msg_sender != -1);
  assert (arg->msg_receiver != -1);

  file_server_t* file_server = malloc (sizeof (file_server_t));
  file_server->file = arg->file;
  file_server->announce = arg->announce;
  file_server->download = arg->download;
  file_server->downloaded_sent = false;
  file_server->announcement_interval = INIT_ANNOUNCE_INTERVAL;
  file_server->next_announce = time (NULL);
  file_server->bidq = bidq_create ();
  file_server->requests = bitset_create (OFFSET_TO_FRAGMENT (file_server->file->fileid.size - 1) + 1);
  file_server->request_idx = 0;
  file_server->fragments = bitset_create (OFFSET_TO_FRAGMENT (file_server->file->fileid.size - 1) + 1);
  if (!file_server->download) {
    /* If we are not downloading, we have all the fragments. */
    bitset_set_all (file_server->fragments);
  }
  file_server->fragment_idx = 0;

  /*



					  +-------------------------------------------------------------------------------------------------------+
					  |  +-------------------------------------------------------------------------------------------------+  |
					  |  |  +-------------------------------------------------------------------------------------------+  |  |
					  |  |  |                                                                                           |  |  |
					  |  |  |   +--------------------------------------------------+         +----------------------+   |  |  |
					  |  |  |   |                   file_server                    |         |       alarm          |   |  |  |
					  |  |  |   +------------------------+-------------------------+         +---------+------------+   |  |  |
					  |  |  +-->| _announcement_alarm_in | _announcement_alarm_out |-------->| _set_in | _alarm_out |---+  |  |
					  |  |      |                        |                         |         +---------+------------+      |  |
					  |  |      |                        |                         |                                       |  |
					  |  |      |                        |                         |         +----------------------+      |  |
					  |  |      |                        |                         |         |       alarm          |      |  |
					  |  |      |                        |                         |         +---------+------------+      |  |
					  |  +----->| _request_alarm_in      | _request_alarm_out      |-------->| _set_in | _alarm_out |------+  |
					  |         |                        |                         |         +---------+------------+         |
					  |         |                        |                         |                                          |
					  |         |                        |                         |         +----------------------+         |
					  |         |                        |                         |         |       alarm          |         |
					  |         |                        |                         |         +---------+------------+         |
					  +-------->| _fragment_alarm_in     | _fragment_alarm_out     |-------->| _set_in | _alarm_out |---------+
					            |                        |                         |         +---------+------------+
					            |                        |                         |
  +---------------------------------------+         |                        |                         |         +----------------------------+
  |              msg_receiver             |         |                        |                         |         |      msg_sender_proxy      |
  +------------------+--------------------+         |                        |                         |         +-------------+--------------+
  |                  | _announcement_out  |-------->| _announcement_in       | _message_out            |-------->| _message_in |              |
  |                  |                    |         |                        |                         |         +-------------+--------------+
  |                  | _request_out       |-------->| _request_in            |                         |
  |                  |                    |         |                        |                         |
  |                  | _fragment_out      |-------->| _fragment_in           | _download_complete_out  |
  +------------------+--------------------+         +------------------------+-------------------------+

   */

  file_server->automan = automan_creat (file_server,
					&file_server->self);

  assert (automan_create (file_server->automan,
			  &file_server->announcement_alarm,
			  &alarm_descriptor,
			  NULL,
			  NULL,
			  NULL) == 0);
  assert (automan_compose (file_server->automan,
			   &file_server->announcement_alarm_in_composed,
			   &file_server->announcement_alarm,
			   alarm_alarm_out,
			   NULL,
			   &file_server->self,
			   file_server_announcement_alarm_in,
			   NULL,
			   file_server_announcement_alarm_composed,
			   NULL) == 0);
  assert (automan_compose (file_server->automan,
			   &file_server->announcement_alarm_out_composed,
			   &file_server->self,
			   file_server_announcement_alarm_out,
			   NULL,
			   &file_server->announcement_alarm,
			   alarm_set_in,
			   NULL,
			   file_server_announcement_alarm_composed,
			   NULL) == 0);

  assert (automan_create (file_server->automan,
			  &file_server->request_alarm,
			  &alarm_descriptor,
			  NULL,
			  NULL,
			  NULL) == 0);
  assert (automan_compose (file_server->automan,
			   &file_server->request_alarm_in_composed,
			   &file_server->request_alarm,
			   alarm_alarm_out,
			   NULL,
			   &file_server->self,
			   file_server_request_alarm_in,
			   NULL,
			   file_server_request_alarm_composed,
			   NULL) == 0);
  assert (automan_compose (file_server->automan,
			   &file_server->request_alarm_out_composed,
			   &file_server->self,
			   file_server_request_alarm_out,
			   NULL,
			   &file_server->request_alarm,
			   alarm_set_in,
			   NULL,
			   file_server_request_alarm_composed,
			   NULL) == 0);

  assert (automan_create (file_server->automan,
			  &file_server->fragment_alarm,
			  &alarm_descriptor,
			  NULL,
			  NULL,
			  NULL) == 0);
  assert (automan_compose (file_server->automan,
			   &file_server->fragment_alarm_in_composed,
			   &file_server->fragment_alarm,
			   alarm_alarm_out,
			   NULL,
			   &file_server->self,
			   file_server_fragment_alarm_in,
			   NULL,
			   file_server_fragment_alarm_composed,
			   NULL) == 0);
  assert (automan_compose (file_server->automan,
			   &file_server->fragment_alarm_out_composed,
			   &file_server->self,
			   file_server_fragment_alarm_out,
			   NULL,
			   &file_server->fragment_alarm,
			   alarm_set_in,
			   NULL,
			   file_server_fragment_alarm_composed,
			   NULL) == 0);

  assert (automan_output_add (file_server->automan,
			      &file_server->download_complete_composed,
			      file_server_download_complete_out,
			      NULL,
			      file_server_download_complete_composed,
			      NULL) == 0);

  assert (automan_proxy_add (file_server->automan,
			     &file_server->msg_sender_proxy,
			     arg->msg_sender,
			     msg_sender_request_proxy,
			     -1,
			     file_server_callback,
			     NULL,
			     NULL) == 0);
  assert (automan_compose (file_server->automan,
			   &file_server->message_out_composed,
			   &file_server->self,
			   file_server_message_out,
			   NULL,
			   &file_server->msg_sender_proxy,
			   msg_sender_proxy_message_in,
			   NULL,
			   NULL,
			   NULL) == 0);

  file_server->msg_receiver = arg->msg_receiver;
  assert (automan_compose (file_server->automan,
			   &file_server->announcement_in_composed,
			   &file_server->msg_receiver,
			   msg_receiver_announcement_out,
			   NULL,
			   &file_server->self,
			   file_server_announcement_in,
			   NULL,
			   NULL,
			   NULL) == 0);
  assert (automan_compose (file_server->automan,
			   &file_server->request_in_composed,
			   &file_server->msg_receiver,
			   msg_receiver_request_out,
			   NULL,
			   &file_server->self,
			   file_server_request_in,
			   NULL,
			   NULL,
			   NULL) == 0);
  assert (automan_compose (file_server->automan,
			   &file_server->fragment_in_composed,
			   &file_server->msg_receiver,
			   msg_receiver_fragment_out,
			   NULL,
			   &file_server->self,
			   file_server_fragment_in,
			   NULL,
			   NULL,
			   NULL) == 0);
  return file_server;
}

static void
file_server_system_input (void* state, void* param, bid_t bid)
{
  assert (state != NULL);
  file_server_t* file_server = state;

  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));
  const receipt_t* receipt = buffer_read_ptr (bid);

  automan_apply (file_server->automan, receipt);
}

static bid_t
file_server_system_output (void* state, void* param)
{
  file_server_t* file_server = state;
  assert (file_server != NULL);

  return automan_action (file_server->automan);
}

static void
file_server_announcement_alarm_composed (void* state, void* param, receipt_type_t receipt)
{
  file_server_t* file_server = state;
  assert (file_server != NULL);
  assert (receipt == COMPOSED);

  if (file_server->announcement_alarm_in_composed &&
      file_server->announcement_alarm_out_composed) {
    assert (schedule_output (file_server_announcement_alarm_out, NULL) == 0);
  }
}

static void
file_server_request_alarm_composed (void* state, void* param, receipt_type_t receipt)
{
  file_server_t* file_server = state;
  assert (file_server != NULL);
  assert (receipt == COMPOSED);

  if (file_server->request_alarm_in_composed &&
      file_server->request_alarm_out_composed) {
    assert (schedule_output (file_server_request_alarm_out, NULL) == 0);
  }
}

static void
file_server_fragment_alarm_composed (void* state, void* param, receipt_type_t receipt)
{
  file_server_t* file_server = state;
  assert (file_server != NULL);
  assert (receipt == COMPOSED);

  if (file_server->fragment_alarm_in_composed &&
      file_server->fragment_alarm_out_composed) {
    assert (schedule_output (file_server_fragment_alarm_out, NULL) == 0);
  }
}

static void
file_server_callback (void* state, void* param, bid_t bid)
{
  file_server_t* file_server = state;
  assert (file_server != NULL);

  automan_proxy_receive (file_server->automan, bid);
}

static void
file_server_announcement_alarm_in (void* state, void* param, bid_t bid)
{
  file_server_t* file_server = state;
  assert (file_server != NULL);

  if (file_server->announce) {
    if (file_server->next_announce < time (NULL)) {
      /* Time to announce and we have all of the fragments. */
      bid_t bid = buffer_alloc (sizeof (mftp_Message_t));
      mftp_Message_t* message = buffer_write_ptr (bid);
      mftp_Announcement_init (message, &file_server->file->fileid);
      bidq_push_back (file_server->bidq, bid);
      assert (schedule_output (file_server_message_out, NULL) == 0);
    }

    /* Set the alarm again. */
    assert (schedule_output (file_server_announcement_alarm_out, NULL) == 0);
  }
}

static bid_t
file_server_announcement_alarm_out (void* state, void* param)
{
  assert (state != NULL);
  file_server_t* file_server = state;

  /* Set the alarm. */
  bid_t bid = buffer_alloc (sizeof (alarm_set_in_t));
  alarm_set_in_t* in = buffer_write_ptr (bid);
  in->secs = file_server->announcement_interval;
  in->usecs = 0;

  return bid;
}

static void
file_server_request_alarm_in (void* state, void* param, bid_t bid)
{
  file_server_t* file_server = state;
  assert (file_server != NULL);

  if (!bitset_full (file_server->fragments)) {
    /* We don't have all the fragments. */

    /* Find a contiguous region to request. */
    uint32_t stop_fragment = bitset_capacity (file_server->fragments);
    uint32_t start_fragment = bitset_next_clear (file_server->fragments, file_server->fragment_idx);
    if (!bitset_empty (file_server->fragments)) {
      stop_fragment = bitset_next_set (file_server->fragments, start_fragment + 1);
      if (stop_fragment < start_fragment) {
  	stop_fragment = bitset_capacity (file_server->fragments);
      }
    }

    /* Form the request. */
    bid_t bid = buffer_alloc (sizeof (mftp_Message_t));
    mftp_Message_t* message = buffer_write_ptr (bid);
    uint32_t offset = FRAGMENT_TO_OFFSET (start_fragment);
    uint32_t size = (stop_fragment - start_fragment) * FRAGMENT_SIZE;
    if (offset + size > file_server->file->fileid.size) {
      size = file_server->file->fileid.size - offset;
    }
    mftp_Request_init (message, &file_server->file->fileid, offset, size);
    bidq_push_back (file_server->bidq, bid);
    assert (schedule_output (file_server_message_out, NULL) == 0);

    /* Set the alarm again. */
    assert (schedule_output (file_server_request_alarm_out, NULL) == 0);
  }

}

static bid_t
file_server_request_alarm_out (void* state, void* param)
{
  /* Set the alarm. */
  bid_t bid = buffer_alloc (sizeof (alarm_set_in_t));
  alarm_set_in_t* in = buffer_write_ptr (bid);
  in->secs = REQUEST_INTERVAL;
  in->usecs = 0;

  return bid;
}

static void
file_server_fragment_alarm_in (void* state, void* param, bid_t bid)
{
  file_server_t* file_server = state;
  assert (file_server != NULL);

  if (!bitset_empty (file_server->requests)) {
    /* Someone has requested a fragment. */

    /* Find a requested fragment. */
    file_server->request_idx = bitset_next_set (file_server->requests, file_server->request_idx);
    assert (bitset_test (file_server->fragments, file_server->request_idx));

    /* Form the fragment. */
    bid_t bid = buffer_alloc (sizeof (mftp_Message_t));
    mftp_Message_t* message = buffer_write_ptr (bid);
    uint32_t offset = FRAGMENT_TO_OFFSET (file_server->request_idx);
    uint32_t size = file_server->file->fileid.size - offset;
    if (size > FRAGMENT_SIZE) {
      size = FRAGMENT_SIZE;
    }
    mftp_Fragment_init (message, &file_server->file->fileid, offset, size, file_server->file->data + offset);
    bidq_push_back (file_server->bidq, bid);
    assert (schedule_output (file_server_message_out, NULL) == 0);

    /* Set the alarm again. */
    assert (schedule_output (file_server_fragment_alarm_out, NULL) == 0);

    /* Clear the request so we don't keep sending the same fragment. */
    bitset_clear (file_server->requests, file_server->request_idx);
  }
}

static bid_t
file_server_fragment_alarm_out (void* state, void* param)
{
  /* Set the alarm. */
  bid_t bid = buffer_alloc (sizeof (alarm_set_in_t));
  alarm_set_in_t* in = buffer_write_ptr (bid);
  in->secs = 0;
  in->usecs = FRAGMENT_INTERVAL;

  return bid;
}

void
file_server_announcement_in (void* state, void* param, bid_t bid)
{
  file_server_t* file_server = state;
  assert (file_server != NULL);

  assert (buffer_size (bid) == sizeof (mftp_Message_t));
  const mftp_Message_t* message = buffer_read_ptr (bid);
  assert (message->header.type == ANNOUNCEMENT);

  if (mftp_FileID_cmp (&file_server->file->fileid, &message->announcement.fileid) == 0) {
    /* The announcement is for our file.
       It could have come from us or someone else, it doesn't matter.
       Set the next announce time and double the interval to achieve exponential backoff. */
    file_server->next_announce += file_server->announcement_interval;
    file_server->announcement_interval *= 2;
    if (file_server->announcement_interval > MAX_ANNOUNCE_INTERVAL) {
      file_server->announcement_interval = MAX_ANNOUNCE_INTERVAL;
    }
  }
}

void
file_server_request_in (void* state, void* param, bid_t bid)
{
  file_server_t* file_server = state;
  assert (file_server != NULL);

  assert (buffer_size (bid) == sizeof (mftp_Message_t));
  const mftp_Message_t* message = buffer_read_ptr (bid);
  assert (message->header.type == REQUEST);

  if (mftp_FileID_cmp (&file_server->file->fileid, &message->request.fileid) == 0) {
    /* Someone has requested fragments from our file.
       Add the fragments to the requested set. */
    uint32_t s;
    for (s = 0;
	 s < message->request.size;
	 s += FRAGMENT_SIZE) {
      if (bitset_test (file_server->fragments, OFFSET_TO_FRAGMENT (message->request.offset + s))) {
	bitset_set (file_server->requests, OFFSET_TO_FRAGMENT (message->request.offset + s));
      }
    }

    /* We might have fragments to send. */
    assert (schedule_output (file_server_fragment_alarm_out, NULL) == 0);
  }
}

void
file_server_fragment_in (void* state, void* param, bid_t bid)
{
  file_server_t* file_server = state;
  assert (file_server != NULL);

  assert (buffer_size (bid) == sizeof (mftp_Message_t));
  const mftp_Message_t* message = buffer_read_ptr (bid);
  assert (message->header.type == FRAGMENT);

  if (mftp_FileID_cmp (&file_server->file->fileid, &message->fragment.fileid) == 0) {
    /* Someone sent a fragment from our file. */
    uint32_t fragment = OFFSET_TO_FRAGMENT (message->fragment.offset);
    /* Remove the sent fragment from the requests. */
    bitset_clear (file_server->requests, fragment);
    /* Save the fragment. */
    if (!bitset_test (file_server->fragments, fragment)) {
      memcpy (file_server->file->data + message->fragment.offset, message->fragment.data, message->fragment.size);
      bitset_set (file_server->fragments, fragment);
      /* The file might be complete. */
      assert (schedule_output (file_server_download_complete_out, NULL) == 0);
    }
  }
}

bid_t
file_server_message_out (void* state, void* param)
{
  file_server_t* file_server = state;
  assert (file_server != NULL);

  if (!bidq_empty (file_server->bidq)) {
    /* We have a message to send. */
    bid_t bid = bidq_front (file_server->bidq);
    bidq_pop_front (file_server->bidq);
    assert (schedule_output (file_server_message_out, NULL) == 0);
    return bid;
  }
  else {
    /* No messages. */
    return -1;
  }
}

static void
file_server_download_complete_composed (void* state, void* param, receipt_type_t receipt)
{
  file_server_t* file_server = state;
  assert (file_server != NULL);

  assert (schedule_output (file_server_download_complete_out, NULL) == 0);
}

bid_t
file_server_download_complete_out (void* state, void* param)
{
  file_server_t* file_server = state;
  assert (file_server != NULL);

  if (file_server->download &&
      bitset_full (file_server->fragments) &&
      file_server->download_complete_composed &&
      !file_server->downloaded_sent) {
    /* We were told to download a file AND
       we have all of the fragments AND
       someone will get the download complete message AND
       we have not sent the download complete message. */
    file_server->downloaded_sent = true;
    return buffer_alloc (0);
  }
  else {
    return -1;
  }
}

static input_t file_server_free_inputs[] = {
  file_server_callback,
  NULL
};

static input_t file_server_inputs[] = {
  file_server_announcement_alarm_in,
  file_server_request_alarm_in,
  file_server_fragment_alarm_in,
  file_server_announcement_in,
  file_server_request_in,
  file_server_fragment_in,
  NULL
};
static output_t file_server_outputs[] = {
  file_server_announcement_alarm_out,
  file_server_request_alarm_out,
  file_server_fragment_alarm_out,
  file_server_message_out,
  file_server_download_complete_out,
  NULL
};

descriptor_t file_server_descriptor = {
  .constructor = file_server_create,
  .system_input = file_server_system_input,
  .system_output = file_server_system_output,
  .free_inputs = file_server_free_inputs,
  .inputs = file_server_inputs,
  .outputs = file_server_outputs,
};
