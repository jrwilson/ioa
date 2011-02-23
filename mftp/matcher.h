#ifndef __matcher_h__
#define __matcher_h__

#include <mftp.h>

extern descriptor_t matcher_descriptor;
typedef struct {
  aid_t* msg_sender;
  aid_t* msg_receiver;
} matcher_create_arg_t;

void matcher_new_comm_in (void*, void*, bid_t);

#endif
