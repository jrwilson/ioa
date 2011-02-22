#ifndef __udp_sender_h__
#define __udp_sender_h__

#include <ueioa.h>

extern descriptor_t udp_sender_descriptor;

void udp_sender_packet_in (void*, void*, bid_t);

#endif /* __udp_sender_h__ */
