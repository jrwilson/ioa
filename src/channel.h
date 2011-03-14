#ifndef __channel_h__
#define __channel_h__

#include <ueioa.h>
#include <uuid/uuid.h>
#include <stdint.h>

typedef struct channel_create_arg_struct {
  aid_t a_component_manager;
  uuid_t a_component;
  uint32_t a_port_type;

  aid_t b_component_manager;
  uuid_t b_component;
  uint32_t b_port_type;

  uint32_t a_to_b_map_size;
  uint32_t* a_to_b_map;

  uint32_t b_to_a_map_size;
  uint32_t* b_to_a_map;
} channel_create_arg_t;

void channel_create_arg_init (channel_create_arg_t*,
			      aid_t a_component_manager,
			      uuid_t a_component,
			      uint32_t a_port_type,
			      aid_t b_component_manager,
			      uuid_t b_component,
			      uint32_t b_port_type,
			      uint32_t,
			      uint32_t*,
			      uint32_t,
			      uint32_t*);

void channel_a_in (void*, void*, bid_t);
bid_t channel_a_out (void*, void*);
void channel_b_in (void*, void*, bid_t);
bid_t channel_b_out (void*, void*);

extern const descriptor_t channel_descriptor;

#endif
