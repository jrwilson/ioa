#ifndef __udp_sender_h__
#define __udp_sender_h__

#include <ueioa.h>
#include <stdint.h>

typedef struct {
  uint16_t port;
} udp_sender_create_arg_t;

void udp_sender_packet_in (void*, void*, bid_t);

extern descriptor_t udp_sender_descriptor;

#endif /* __udp_sender_h__ */
