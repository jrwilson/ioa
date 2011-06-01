#ifndef __camera_h__
#define __camera_h__

#include <ueioa.h>

extern descriptor_t camera_descriptor;

typedef struct {
  char* dev_name;
} camera_create_arg_t;

bid_t camera_frame_out (void* state, void* param);

#endif
