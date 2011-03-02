#ifndef __channel_h__
#define __channel_h__

#include <ueioa.h>

typedef struct channel_create_arg_struct {
  aid_t* a_port_manager;
  size_t a_component;
  size_t a_port_type;

  aid_t* b_port_manager;
  size_t b_component;
  size_t b_port_type;

  size_t a_to_b_map_size;
  size_t* a_to_b_map;

  size_t b_to_a_map_size;
  size_t* b_to_a_map;
} channel_create_arg_t;

void channel_create_arg_init (channel_create_arg_t*,
			      aid_t*,
			      size_t,
			      size_t,
			      aid_t*,
			      size_t,
			      size_t,
			      size_t,
			      size_t*,
			      size_t,
			      size_t*);

void channel_strobe (void*, void*, bid_t);

void channel_a_in (void*, void*, bid_t);
bid_t channel_a_out (void*, void*);
void channel_b_in (void*, void*, bid_t);
bid_t channel_b_out (void*, void*);

extern descriptor_t channel_descriptor;

#endif
