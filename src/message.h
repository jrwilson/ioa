#ifndef __message_h__
#define __message_h__

typedef struct message_struct {
  size_t src_component;
  size_t src_port_type;
  size_t src_port;
  size_t src_message;

  size_t dst_component;
  size_t dst_port_type;
  size_t dst_port;
  size_t dst_message;

  bid_t bid;
} message_t;

#endif
