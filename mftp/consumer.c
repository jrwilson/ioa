#include "consumer.h"

#include <stdio.h>
#include <assert.h>
#include "mftp.h"

static void
print_fileid (const mftp_FileID_t* fileid)
{
  int idx;
  printf ("(");
  for (idx = 0; idx < HASH_SIZE; ++idx) {
    printf ("%02x", fileid->hash[idx]);
  }
  printf (",%u,%u)", fileid->type, fileid->size);
}

void
consumer_announcement_in (void* state, void* param, bid_t bid)
{
  const mftp_Message_t* message = buffer_read_ptr (bid);
  assert (message->header.type == ANNOUNCEMENT);

  printf ("A ");
  print_fileid (&message->announcement.fileid);
  printf ("\n");
}

void
consumer_match_in (void* state, void* param, bid_t bid)
{
  const mftp_Message_t* message = buffer_read_ptr (bid);
  assert (message->header.type == MATCH);

  printf ("M ");
  print_fileid (&message->match.fileid1);
  printf (" ");
  print_fileid (&message->match.fileid2);
  printf ("\n");
}

void
consumer_request_in (void* state, void* param, bid_t bid)
{
  const mftp_Message_t* message = buffer_read_ptr (bid);
  assert (message->header.type == REQUEST);

  printf ("R ");
  print_fileid (&message->request.fileid);
  printf (" offset=%u size=%u\n", message->request.offset, message->request.size);
}

void
consumer_fragment_in (void* state, void* param, bid_t bid)
{
  const mftp_Message_t* message = buffer_read_ptr (bid);
  assert (message->header.type == FRAGMENT);

  printf ("F ");
  print_fileid (&message->request.fileid);
  printf (" offset=%u size=%u\n", message->request.offset, message->request.size);
}

static input_t consumer_inputs[] = { consumer_announcement_in, consumer_match_in, consumer_request_in, consumer_fragment_in, NULL };

descriptor_t consumer_descriptor = {
  .inputs = consumer_inputs,
};
