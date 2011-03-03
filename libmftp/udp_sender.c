#include "udp_sender.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>

typedef struct {
  int fd;
  struct sockaddr_in dest;
  socklen_t dest_len;
  bidq_t* bidq;
} udp_sender_t;

static void*
udp_sender_create (void* a)
{
  udp_sender_create_arg_t* arg = a;
  assert (arg != NULL);

  udp_sender_t* udp_sender = malloc (sizeof (udp_sender_t));
  /* Create the socket. */
  if ((udp_sender->fd = socket (AF_INET, SOCK_DGRAM, 0)) == -1) {
    perror ("socket");
    exit (EXIT_FAILURE);
  }
  /* Set the broadcast option. */
  int v = 1;
  if (setsockopt (udp_sender->fd, SOL_SOCKET, SO_BROADCAST, &v, sizeof (v)) == -1) {
    perror ("setsockopt");
    exit (EXIT_FAILURE);
  }
  /* Set non-blocking. */
  if (fcntl (udp_sender->fd, F_SETFL, O_NONBLOCK) == -1) {
    perror ("fcntl");
    exit (EXIT_FAILURE);
  }
  /* Bind to broadcast.
     TODO: Change this to multicast.
  */
  udp_sender->dest.sin_family = AF_INET;
  udp_sender->dest.sin_addr.s_addr = inet_addr ("255.255.255.255");
  udp_sender->dest.sin_port = htons (arg->port);
  udp_sender->dest_len = sizeof (udp_sender->dest);

  /* Initialize the queue. */
  udp_sender->bidq = bidq_create ();

  return udp_sender;
}

void
udp_sender_packet_in (void* state, void* param, bid_t bid)
{
  udp_sender_t* udp_sender = state;
  assert (udp_sender != NULL);
  assert (bid != -1);

  /* Enqueue the item. */
  buffer_incref (bid);
  bidq_push_back (udp_sender->bidq, bid);

  assert (schedule_write_input (udp_sender->fd) == 0);
}

static void
udp_sender_write_input (void* state, void* param, bid_t bid)
{
  udp_sender_t* udp_sender = state;
  assert (udp_sender != NULL);

  if (!bidq_empty (udp_sender->bidq)) {
    bid_t bid = bidq_front (udp_sender->bidq);
    bidq_pop_front (udp_sender->bidq);

    /* Send the item. */
    ssize_t bytes_sent = sendto (udp_sender->fd, buffer_read_ptr (bid), buffer_size (bid), 0, (struct sockaddr*)&udp_sender->dest, udp_sender->dest_len);
    if (bytes_sent != buffer_size (bid)) {
      perror ("sendto");
    }

    buffer_decref (bid);

    /* Go again. */
    assert (schedule_write_input (udp_sender->fd) == 0);
  }
}

static input_t udp_sender_inputs[] = { udp_sender_packet_in, NULL };

descriptor_t udp_sender_descriptor = {
  .constructor = udp_sender_create,
  .write_input = udp_sender_write_input,
  .inputs = udp_sender_inputs,
};
