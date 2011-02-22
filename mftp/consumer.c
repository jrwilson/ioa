#include "consumer.h"

#include <stdio.h>
#include <assert.h>
#include "mftp.h"

void
consumer_announcement_in (void* state, void* param, bid_t bid)
{
  const mftp_Message_t* message = buffer_read_ptr (bid);
  assert (message->header.type == ANNOUNCEMENT);

  printf ("ANNOUNCEMENT\t");
  int idx;
  for (idx = 0; idx < HASH_SIZE; ++idx) {
    printf ("%02x", message->announcement.fileid.hash[idx]);
  }
  printf (" size=%lu type=%lu\n", message->announcement.fileid.size, message->announcement.fileid.type);
}

void
consumer_match_in (void* state, void* param, bid_t bid)
{
  const mftp_Message_t* message = buffer_read_ptr (bid);
  assert (message->header.type == MATCH);

  printf ("MATCH\n");
}

void
consumer_request_in (void* state, void* param, bid_t bid)
{
  const mftp_Message_t* message = buffer_read_ptr (bid);
  assert (message->header.type == REQUEST);

  printf ("REQUEST\t\t");
  int idx;
  for (idx = 0; idx < HASH_SIZE; ++idx) {
    printf ("%02x", message->request.fileid.hash[idx]);
  }
  printf (" offset=%lu size=%lu\n", message->request.offset, message->request.size);
}

void
consumer_fragment_in (void* state, void* param, bid_t bid)
{
  const mftp_Message_t* message = buffer_read_ptr (bid);
  assert (message->header.type == FRAGMENT);

  printf ("FRAGMENT\t");
  int idx;
  for (idx = 0; idx < HASH_SIZE; ++idx) {
    printf ("%02x", message->fragment.fileid.hash[idx]);
  }
  printf (" offset=%lu size=%lu (%s)\n", message->fragment.offset, message->fragment.size, message->fragment.data);

}

static input_t consumer_inputs[] = { consumer_announcement_in, consumer_match_in, consumer_request_in, consumer_fragment_in, NULL };

descriptor_t consumer_descriptor = {
  .inputs = consumer_inputs,
};
