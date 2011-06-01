#ifndef __message_h__
#define __message_h__

#include <stdint.h>

typedef struct message_struct {
  uuid_t src_component;
  uint32_t src_port_type;
  uint32_t src_port;
  uint32_t src_message;

  uuid_t dst_component;
  uint32_t dst_port_type;
  uint32_t dst_port;
  uint32_t dst_message;

  bid_t bid;
} message_t;

#endif
