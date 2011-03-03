#ifndef __udp_receiver_h__
#define __udp_receiver_h__

#include <ueioa.h>
#include <stdint.h>

typedef struct {
  uint16_t port;
} udp_receiver_create_arg_t;

bid_t udp_receiver_packet_out (void*, void*);

extern descriptor_t udp_receiver_descriptor;

#endif /* __udp_receiver_h__ */
