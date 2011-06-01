#include "matcher.h"

#include <automan.h>

#include "ft.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>

static void matcher_callback (void* state, void* param, bid_t bid);

static void matcher_meta_download_complete (void* state, void* param, bid_t bid);
static void matcher_query_download_complete (void* state, void* param, bid_t bid);

static void matcher_announcement_in (void*, void*, bid_t);
static void matcher_match_in (void*, void*, bid_t);
static bid_t matcher_message_out (void*, void*);
static void matcher_message_out_composed (void*, void*, receipt_type_t);

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
  mftp_FileID_t meta_fileid;
  mftp_FileID_t query_fileid;
  bool recent_match;
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
  const matcher_create_arg_t* arg = (const matcher_create_arg_t*)a;
  assert (arg != NULL);
  assert (arg->msg_sender != -1);
  assert (arg->msg_receiver != -1);

  matcher_t* matcher = (matcher_t*)malloc (sizeof (matcher_t));
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
			   matcher_message_out_composed,
			   NULL) == 0);

  return matcher;
}

static void
matcher_system_input (void* state, void* param, bid_t bid)
{
  assert (state != NULL);
  matcher_t* matcher = (matcher_t*)state;

  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));
  const receipt_t* receipt = (const receipt_t*)buffer_read_ptr (bid);

  automan_apply (matcher->automan, receipt);
}

static bid_t
matcher_system_output (void* state, void* param)
{
  matcher_t* matcher = (matcher_t*)state;
  assert (matcher != NULL);

  return automan_action (matcher->automan);
}

static void
matcher_callback (void* state, void* param, bid_t bid)
{
  matcher_t* matcher = (matcher_t*)state;
  assert (matcher != NULL);

  automan_proxy_receive (matcher->automan, bid);
}

void
matcher_announcement_in (void* state, void* param, bid_t bid)
{
  matcher_t* matcher = (matcher_t*)state;
  assert (matcher != NULL);

  assert (buffer_size (bid) == sizeof (mftp_Message_t));
  const mftp_Message_t* message = (const mftp_Message_t*)buffer_read_ptr (bid);
  assert (message->header.type == ANNOUNCEMENT);

  if (message->announcement.fileid.type == META) {
    /* Check if the meta is in our list. */
    meta_item_t* meta;
    for (meta = matcher->metas; meta != NULL && mftp_FileID_cmp (&meta->meta_fileid, &message->announcement.fileid) != 0; meta = meta->next)
      ;;
    
    if (meta == NULL) {
      /* Add it to the list. */
      meta = (meta_item_t*)malloc (sizeof (meta_item_t));
      
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
    }

    match_item_t* match;
    for (match = matcher->matches;
	 match != NULL;
	 match = match->next) {
      if (mftp_FileID_cmp (&match->meta_fileid, &meta->meta_fileid) == 0) {
	if (match->recent_match) {
	  /* We have seen a match recently.  Don't send but reset. */
	  match->recent_match = false;
	}
	else {
	  /* Send match for meta. */
	  bid_t bid = buffer_alloc (sizeof (mftp_Message_t));
	  mftp_Message_t* message = (mftp_Message_t*)buffer_write_ptr (bid);
	  mftp_Match_init (message, &match->meta_fileid, &match->query_fileid);
	  bidq_push_back (matcher->bidq, bid);
	}
      }
    }
  }
  else if (message->announcement.fileid.type == QUERY) {
    /* Check if the query is in our list. */
    query_item_t* query;
    for (query = matcher->queries; query != NULL && mftp_FileID_cmp (&query->query_fileid, &message->announcement.fileid) != 0; query = query->next)
      ;;
    
    if (query == NULL) {
      /* Add it to the list. */
      query = (query_item_t*)malloc (sizeof (query_item_t));
      
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
    }

    match_item_t* match;
    for (match = matcher->matches;
	 match != NULL;
	 match = match->next) {
      if (mftp_FileID_cmp (&match->query_fileid, &query->query_fileid) == 0) {
	if (match->recent_match) {
	  /* We have seen a match recently.  Don't send but reset. */
	  match->recent_match = false;
	}
	else {
	  /* Send match for query. */
	  bid_t bid = buffer_alloc (sizeof (mftp_Message_t));
	  mftp_Message_t* message = (mftp_Message_t*)buffer_write_ptr (bid);
	  mftp_Match_init (message, &match->meta_fileid, &match->query_fileid);
	  bidq_push_back (matcher->bidq, bid);
	}
      }
    }
  }

  /* Might have matches to send. */
  assert (schedule_output (matcher_message_out, NULL) == 0);
}

static void
add_match (matcher_t* matcher, const mftp_FileID_t* meta_fileid, const mftp_FileID_t* query_fileid)
{
  assert (matcher != NULL);
  assert (meta_fileid != NULL);
  assert (query_fileid != NULL);

  /* Check that the match doesn't exist. */
  match_item_t* match;
  for (match = matcher->matches;
       match != NULL &&
	 !(mftp_FileID_cmp (meta_fileid, &match->meta_fileid) == 0 &&
	   mftp_FileID_cmp (query_fileid, &match->query_fileid) == 0);
       match = match->next)
    ;;

  if (match == NULL) {
    match = (match_item_t*)malloc (sizeof (match_item_t));
    
    memcpy (&match->meta_fileid, meta_fileid, sizeof (mftp_FileID_t));
    memcpy (&match->query_fileid, query_fileid, sizeof (mftp_FileID_t));
    match->recent_match = false;
    
    match->next = matcher->matches;
    matcher->matches = match;
  }
}

void
matcher_match_in (void* state, void* param, bid_t bid)
{
  matcher_t* matcher = (matcher_t*)state;
  assert (matcher != NULL);

  assert (buffer_size (bid) == sizeof (mftp_Message_t));
  const mftp_Message_t* message = (const mftp_Message_t*)buffer_read_ptr (bid);
  assert (message->header.type == MATCH);

  if (message->match.fileid1.type == META && message->match.fileid2.type == QUERY) {
    match_item_t* match;
    for (match = matcher->matches;
  	 match != NULL &&
  	   !(mftp_FileID_cmp (&message->match.fileid1, &match->meta_fileid) == 0 &&
  	     mftp_FileID_cmp (&message->match.fileid2, &match->query_fileid) == 0);
  	 match = match->next)
      ;;


    if (match != NULL) {
      /* We have seen a recent match. */
      match->recent_match = true;
    }
    else {
      /* A new match. */
      add_match (matcher, &message->match.fileid1, &message->match.fileid2);
    }
  }
}

static void
matcher_meta_download_complete (void* state, void* param, bid_t bid)
{
  matcher_t* matcher = (matcher_t*)state;
  assert (matcher != NULL);

  meta_item_t* meta = (meta_item_t*)param;
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
  matcher_t* matcher = (matcher_t*)state;
  assert (matcher != NULL);

  query_item_t* query = (query_item_t*)param;
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
  matcher_t* matcher = (matcher_t*)state;
  assert (matcher != NULL);

  if (matcher->message_out_composed && !bidq_empty (matcher->bidq)) {
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

static void
matcher_message_out_composed (void* state, void* param, receipt_type_t receipt)
{
  matcher_t* matcher = (matcher_t*)state;
  assert (matcher != NULL);

  if (receipt == COMPOSED) {
    assert (schedule_output (matcher_message_out, NULL) == 0);
  }
  else if (receipt == DECOMPOSED) {
    /* Okay. */
  }
  else {
    assert (0);
  }
}

static input_t matcher_free_inputs[] = {
  matcher_callback,
  NULL
};

static input_t matcher_inputs[] = {
  matcher_announcement_in,
  matcher_match_in,
  matcher_meta_download_complete,
  matcher_query_download_complete,
  NULL
};

static output_t matcher_outputs[] = {
  matcher_message_out,
  NULL
};

descriptor_t matcher_descriptor = {
  matcher_create,
  matcher_system_input,
  matcher_system_output,
  NULL,
  NULL,
  NULL,
  matcher_free_inputs,
  matcher_inputs,
  matcher_outputs,
  NULL
};
