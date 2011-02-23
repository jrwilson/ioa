#ifndef __match_getter_h__
#define __match_getter_h__

#include <mftp.h>

extern descriptor_t match_getter_descriptor;
typedef struct {
  mftp_File_t* query;
  aid_t* msg_sender;
  aid_t* msg_receiver;
} match_getter_create_arg_t;

void match_getter_new_comm_in (void*, void*, bid_t);

#endif
