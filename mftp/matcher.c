#include "matcher.h"

#include <automan.h>

#include "ft.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>

static void matcher_callback (void* state, void* param, bid_t bid);

static void matcher_match_alarm_in (void*, void*, bid_t);
static bid_t matcher_match_alarm_out (void*, void*);
static void matcher_match_alarm_composed (void*, void*, receipt_type_t);

static void matcher_meta_download_complete (void* state, void* param, bid_t bid);
static void matcher_query_download_complete (void* state, void* param, bid_t bid);

static void matcher_announcement_in (void*, void*, bid_t);
static void matcher_match_in (void*, void*, bid_t);
static bid_t matcher_message_out (void*, void*);

/* In seconds. */
#define INIT_MATCH_INTERVAL 1
#define MAX_MATCH_INTERVAL 16 /* 4096 */

typedef struct meta_item_struct meta_item_t;
struct meta_item_struct {
  bool declared;
  bool download_complete_composed;
  mftp_FileID_t meta_fileid;
  file_server_create_arg_t meta_arg;
  aid_t meta_aid;
  meta_item_t* next;
};

typedef struct query_item_struct query_item_t;
struct query_item_struct {
  bool declared;
  bool download_complete_composed;
  mftp_FileID_t query_fileid;
  file_server_create_arg_t query_arg;
  aid_t query_aid;
  query_item_t* next;
};

typedef struct match_item_struct match_item_t;
struct match_item_struct {
  bool declared;
  bool alarm_in_composed;
  bool alarm_out_composed;
  mftp_FileID_t meta_fileid;
  mftp_FileID_t query_fileid;
  time_t next_match;
  time_t match_interval;
  aid_t match_alarm;
  match_item_t* next;
};

typedef struct {
  bidq_t* bidq;			/* Queue of outgoing messages. */
  meta_item_t* metas;
  query_item_t* queries;
  match_item_t* matches;

  automan_t* automan;
  aid_t self;
  aid_t msg_sender;
  aid_t msg_sender_proxy;
  aid_t msg_receiver;
  bool announcement_in_composed;
  bool match_in_composed;
  bool message_out_composed;
} matcher_t;

static void*
matcher_create (const void* a)
{
  const matcher_create_arg_t* arg = a;
  assert (arg != NULL);
  assert (arg->msg_sender != -1);
  assert (arg->msg_receiver != -1);

  matcher_t* matcher = malloc (sizeof (matcher_t));
  matcher->bidq = bidq_create ();
  matcher->metas = NULL;
  matcher->queries = NULL;
  matcher->matches = NULL;

  matcher->automan = automan_creat (matcher,
				    &matcher->self);
  matcher->msg_sender = arg->msg_sender;
  matcher->msg_receiver = arg->msg_receiver;

  assert (automan_compose (matcher->automan,
			   &matcher->announcement_in_composed,
			   &matcher->msg_receiver,
			   msg_receiver_announcement_out,
			   NULL,
			   &matcher->self,
			   matcher_announcement_in,
			   NULL,
			   NULL,
			   NULL) == 0);
  assert (automan_compose (matcher->automan,
			   &matcher->match_in_composed,
			   &matcher->msg_receiver,
			   msg_receiver_match_out,
			   NULL,
			   &matcher->self,
			   matcher_match_in,
			   NULL,
			   NULL,
			   NULL) == 0);

  assert (automan_proxy_add (matcher->automan,
			     &matcher->msg_sender_proxy,
			     matcher->msg_sender,
			     msg_sender_request_proxy,
			     -1,
			     matcher_callback,
			     NULL,
			     NULL) == 0);
  assert (automan_compose (matcher->automan,
			   &matcher->message_out_composed,
			   &matcher->self,
			   matcher_message_out,
			   NULL,
			   &matcher->msg_sender_proxy,
			   msg_sender_proxy_message_in,
			   NULL,
			   NULL,
			   NULL) == 0);

  return matcher;
}

static void
matcher_system_input (void* state, void* param, bid_t bid)
{
  assert (state != NULL);
  matcher_t* matcher = state;

  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));
  const receipt_t* receipt = buffer_read_ptr (bid);

  automan_apply (matcher->automan, receipt);
}

static bid_t
matcher_system_output (void* state, void* param)
{
  matcher_t* matcher = state;
  assert (matcher != NULL);

  return automan_action (matcher->automan);
}

static void
matcher_callback (void* state, void* param, bid_t bid)
{
  matcher_t* matcher = state;
  assert (matcher != NULL);

  assert (buffer_size (bid) == sizeof (proxy_receipt_t));
  const proxy_receipt_t* receipt = buffer_read_ptr (bid);
  automan_proxy_receive (matcher->automan, receipt);
}

static void
matcher_match_alarm_composed (void* state, void* param, receipt_type_t receipt)
{
  matcher_t* matcher = state;
  assert (matcher != NULL);
  assert (receipt == COMPOSED);

  match_item_t* match = param;
  assert (match != NULL);

  if (match->alarm_in_composed &&
      match->alarm_out_composed) {
    assert (schedule_output (matcher_match_alarm_out, match) == 0);
  }
}

static void
matcher_match_alarm_in (void* state, void* param, bid_t bid)
{
  matcher_t* matcher = state;
  assert (matcher != NULL);

  match_item_t* match = param;
  assert (match != NULL);

  if (match->next_match < time (NULL)) {
    /* Time to announce and we have all of the fragments. */
    bid_t bid = buffer_alloc (sizeof (mftp_Message_t));
    mftp_Message_t* message = buffer_write_ptr (bid);
    mftp_Match_init (message, &match->meta_fileid, &match->query_fileid);
    bidq_push_back (matcher->bidq, bid);
    assert (schedule_output (matcher_message_out, NULL) == 0);
  }

  /* Set the alarm again. */
  assert (schedule_output (matcher_match_alarm_out, match) == 0);
}

static bid_t
matcher_match_alarm_out (void* state, void* param)
{
  matcher_t* matcher = state;
  assert (matcher != NULL);

  match_item_t* match = param;
  assert (match != NULL);

  /* Set the alarm. */
  bid_t bid = buffer_alloc (sizeof (alarm_set_in_t));
  alarm_set_in_t* in = buffer_write_ptr (bid);
  in->secs = match->match_interval;
  in->usecs = 0;

  return bid;
}

void
matcher_announcement_in (void* state, void* param, bid_t bid)
{
  matcher_t* matcher = state;
  assert (matcher != NULL);

  assert (buffer_size (bid) == sizeof (mftp_Message_t));
  const mftp_Message_t* message = buffer_read_ptr (bid);
  assert (message->header.type == ANNOUNCEMENT);

  if (message->announcement.fileid.type == META) {
    /* Check if the meta is in our list. */
    meta_item_t* meta;
    for (meta = matcher->metas; meta != NULL && mftp_FileID_cmp (&meta->meta_fileid, &message->announcement.fileid) != 0; meta = meta->next)
      ;;
    
    if (meta == NULL) {
      /* Add it to the list. */
      meta = malloc (sizeof (meta_item_t));
      
      assert (automan_declare (matcher->automan,
			       &meta->declared,
			       meta,
			       NULL,
			       NULL) == 0);
      
      memcpy (&meta->meta_fileid, &message->announcement.fileid, sizeof (mftp_FileID_t));
      meta->meta_arg.file = mftp_File_create_empty (&meta->meta_fileid);
      meta->meta_arg.announce = false;
      meta->meta_arg.download = true;
      meta->meta_arg.msg_sender = matcher->msg_sender;
      meta->meta_arg.msg_receiver = matcher->msg_receiver;
      assert (automan_create (matcher->automan,
			      &meta->meta_aid,
			      &file_server_descriptor,
			      &meta->meta_arg,
			      NULL,
			      NULL) == 0);
      assert (automan_compose (matcher->automan,
			       &meta->download_complete_composed,
			       &meta->meta_aid,
			       file_server_download_complete_out,
			       NULL,
			       &matcher->self,
			       matcher_meta_download_complete,
			       meta,
			       NULL,
			       NULL) == 0);
      
      meta->next = matcher->metas;
      matcher->metas = meta;
      
      assert (schedule_system_output () == 0);
    }
  }
  else if (message->announcement.fileid.type == QUERY) {
    /* Check if the query is in our list. */
    query_item_t* query;
    for (query = matcher->queries; query != NULL && mftp_FileID_cmp (&query->query_fileid, &message->announcement.fileid) != 0; query = query->next)
      ;;
    
    if (query == NULL) {
      /* Add it to the list. */
      query = malloc (sizeof (query_item_t));
      
      assert (automan_declare (matcher->automan,
			       &query->declared,
			       query,
			       NULL,
			       NULL) == 0);
      
      memcpy (&query->query_fileid, &message->announcement.fileid, sizeof (mftp_FileID_t));
      query->query_arg.file = mftp_File_create_empty (&query->query_fileid);
      query->query_arg.announce = false;
      query->query_arg.download = true;
      query->query_arg.msg_sender = matcher->msg_sender;
      query->query_arg.msg_receiver = matcher->msg_receiver;
      assert (automan_create (matcher->automan,
			      &query->query_aid,
			      &file_server_descriptor,
			      &query->query_arg,
			      NULL,
			      NULL) == 0);
      assert (automan_compose (matcher->automan,
			       &query->download_complete_composed,
			       &query->query_aid,
			       file_server_download_complete_out,
			       NULL,
			       &matcher->self,
			       matcher_query_download_complete,
			       query,
			       NULL,
			       NULL) == 0);
      
      query->next = matcher->queries;
      matcher->queries = query;
      
      assert (schedule_system_output () == 0);
    }
  }

}

void
matcher_match_in (void* state, void* param, bid_t bid)
{
  matcher_t* matcher = state;
  assert (matcher != NULL);

  assert (buffer_size (bid) == sizeof (mftp_Message_t));
  const mftp_Message_t* message = buffer_read_ptr (bid);
  assert (message->header.type == MATCH);

  if (message->match.fileid1.type == META && message->match.fileid2.type == QUERY) {
    match_item_t* ptr;
    for (ptr = matcher->matches;
	 ptr != NULL &&
	   !(mftp_FileID_cmp (&message->match.fileid1, &ptr->meta_fileid) == 0 &&
	     mftp_FileID_cmp (&message->match.fileid2, &ptr->query_fileid) == 0);
	 ptr = ptr->next)
      ;;

    if (ptr != NULL) {
      /* We already know about this match. */
      ptr->next_match += ptr->match_interval;
      ptr->match_interval *= 2;
      if (ptr->match_interval > MAX_MATCH_INTERVAL) {
      	ptr->match_interval = MAX_MATCH_INTERVAL;
      }
    }
    else {
      /* TODO: Do something with a match we don't have. */	 
    }
  }
}

static void
add_match (matcher_t* matcher, mftp_FileID_t* meta_fileid, mftp_FileID_t* query_fileid)
{
  assert (matcher != NULL);
  assert (meta_fileid != NULL);
  assert (query_fileid != NULL);

  match_item_t* match = malloc (sizeof (match_item_t));

  assert (automan_declare (matcher->automan,
			   &match->declared,
			   match,
			   NULL,
			   NULL) == 0);

  memcpy (&match->meta_fileid, meta_fileid, sizeof (mftp_FileID_t));
  memcpy (&match->query_fileid, query_fileid, sizeof (mftp_FileID_t));
  match->next_match = time (NULL);
  match->match_interval = INIT_MATCH_INTERVAL;
  assert (automan_create (matcher->automan,
			  &match->match_alarm,
			  &alarm_descriptor,
			  NULL,
			  NULL,
			  NULL) == 0);
  assert (automan_compose (matcher->automan,
			   &match->alarm_in_composed,
			   &match->match_alarm,
			   alarm_alarm_out,
			   NULL,
			   &matcher->self,
			   matcher_match_alarm_in,
			   match,
			   matcher_match_alarm_composed,
			   match) == 0);
  assert (automan_compose (matcher->automan,
			   &match->alarm_out_composed,
			   &matcher->self,
			   matcher_match_alarm_out,
			   match,
			   &match->match_alarm,
			   alarm_set_in,
			   NULL,
			   matcher_match_alarm_composed,
			   match) == 0);

  match->next = matcher->matches;
  matcher->matches = match;
      
  assert (schedule_system_output () == 0);
}

static void
matcher_meta_download_complete (void* state, void* param, bid_t bid)
{
  matcher_t* matcher = state;
  assert (matcher != NULL);

  meta_item_t* meta = param;
  assert (meta != NULL);

  query_item_t* query;
  for (query = matcher->queries; query != NULL; query = query->next) {
    if (query->query_fileid.size == meta->meta_fileid.size - sizeof (mftp_FileID_t) &&
        memcmp (query->query_arg.file->data, meta->meta_arg.file->data + sizeof (mftp_FileID_t), query->query_fileid.size) == 0) {
      /* Create a new match. */
      add_match (matcher, &meta->meta_fileid, &query->query_fileid);
    }
  }
}

static void
matcher_query_download_complete (void* state, void* param, bid_t bid)
{
  matcher_t* matcher = state;
  assert (matcher != NULL);

  query_item_t* query = param;
  assert (query != NULL);

  meta_item_t* meta;
  for (meta = matcher->metas; meta != NULL; meta = meta->next) {
    if (query->query_fileid.size == meta->meta_fileid.size - sizeof (mftp_FileID_t) &&
        memcmp (query->query_arg.file->data, meta->meta_arg.file->data + sizeof (mftp_FileID_t), query->query_fileid.size) == 0) {
      /* Create a new match. */
      add_match (matcher, &meta->meta_fileid, &query->query_fileid);
    }
  }
}

bid_t
matcher_message_out (void* state, void* param)
{
  matcher_t* matcher = state;
  assert (matcher != NULL);

  if (!bidq_empty (matcher->bidq)) {
    /* We have a message to send. */
    bid_t bid = bidq_front (matcher->bidq);
    bidq_pop_front (matcher->bidq);
    assert (schedule_output (matcher_message_out, NULL) == 0);
    return bid;
  }
  else {
    /* No messages. */
    return -1;
  }
}

static input_t matcher_free_inputs[] = {
  matcher_callback,
  NULL
};

static input_t matcher_inputs[] = {
  matcher_match_alarm_in,
  matcher_announcement_in,
  matcher_match_in,
  matcher_meta_download_complete,
  matcher_query_download_complete,
  NULL
};

static output_t matcher_outputs[] = {
  matcher_match_alarm_out,
  matcher_message_out,
  NULL
};

descriptor_t matcher_descriptor = {
  .constructor = matcher_create,
  .system_input = matcher_system_input,
  .system_output = matcher_system_output,
  .free_inputs = matcher_free_inputs,
  .inputs = matcher_inputs,
  .outputs = matcher_outputs,
};