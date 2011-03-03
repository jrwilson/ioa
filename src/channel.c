#include "channel.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "component_manager.h"
#include "port.h"
#include "message.h"

void
channel_create_arg_init (channel_create_arg_t* arg,
			 aid_t* a_component_manager,
			 uuid_t a_component,
			 uint32_t a_port_type,
			 aid_t* b_component_manager,
			 uuid_t b_component,
			 uint32_t b_port_type,
			 uint32_t a_to_b_map_size,
			 uint32_t* a_to_b_map,
			 uint32_t b_to_a_map_size,
			 uint32_t* b_to_a_map)

{
  assert (arg != NULL);
  assert (a_component_manager != NULL);
  assert (b_component_manager != NULL);
  assert (a_to_b_map != NULL);
  assert (b_to_a_map != NULL);
  assert (a_component != b_component);

  arg->a_component_manager = a_component_manager;
  uuid_copy (arg->a_component, a_component);
  arg->a_port_type = a_port_type;

  arg->b_component_manager = b_component_manager;
  uuid_copy (arg->b_component, b_component);
  arg->b_port_type = b_port_type;

  arg->a_to_b_map_size = a_to_b_map_size;
  arg->a_to_b_map = a_to_b_map;

  arg->b_to_a_map_size = b_to_a_map_size;
  arg->b_to_a_map = b_to_a_map;
}

static void channel_a_callback (void* state, void* param, bid_t bid);
static void channel_b_callback (void* state, void* param, bid_t bid);

typedef struct channel_struct {
  bidq_t* a_to_b;
  bidq_t* b_to_a;

  aid_t* a_component_manager;
  uuid_t a_component;
  uint32_t a_port_type;
  uint32_t a_port;
  aid_t a_port_aid;

  aid_t* b_component_manager;
  uuid_t b_component;
  uint32_t b_port_type;
  uint32_t b_port;
  aid_t b_port_aid;

  uint32_t a_to_b_map_size;
  uint32_t* a_to_b_map;

  uint32_t b_to_a_map_size;
  uint32_t* b_to_a_map;

  manager_t* manager;
  aid_t self;
} channel_t;

static void*
channel_create (const void* a)
{
  const channel_create_arg_t* arg = a;
  assert (arg != NULL);

  channel_t* channel = malloc (sizeof (channel_t));

  channel->a_to_b = bidq_create ();
  channel->b_to_a = bidq_create ();

  channel->a_component_manager = arg->a_component_manager;
  uuid_copy (channel->a_component, arg->a_component);
  channel->a_port_type = arg->a_port_type;

  channel->b_component_manager = arg->b_component_manager;
  uuid_copy (channel->b_component, arg->b_component);
  channel->b_port_type = arg->b_port_type;

  channel->a_to_b_map_size = arg->a_to_b_map_size;
  channel->a_to_b_map = malloc (sizeof (uint32_t) * channel->a_to_b_map_size);
  memcpy (channel->a_to_b_map, arg->a_to_b_map, sizeof (uint32_t) * channel->a_to_b_map_size);

  channel->b_to_a_map_size = arg->b_to_a_map_size;
  channel->b_to_a_map = malloc (sizeof (uint32_t) * channel->b_to_a_map_size);
  memcpy (channel->b_to_a_map, arg->b_to_a_map, sizeof (uint32_t) * channel->b_to_a_map_size);

  channel->manager = manager_create ();

  manager_self_set (channel->manager, &channel->self);

  bid_t b;
  port_request_t* port_request;
  
  b = buffer_alloc (sizeof (port_request_t));
  port_request = buffer_write_ptr (b);
  port_request_init (port_request,
		     channel->a_port_type);
  manager_proxy_add (channel->manager,
		     &channel->a_port_aid,
		     channel->a_component_manager,
		     component_manager_request_port,
		     channel_a_callback,
		     b);
  
  b = buffer_alloc (sizeof (port_request_t));
  port_request = buffer_write_ptr (b);
  port_request_init (port_request,
		     channel->b_port_type);
  manager_proxy_add (channel->manager,
		     &channel->b_port_aid,
		     channel->b_component_manager,
		     component_manager_request_port,
		     channel_b_callback,
		     b);
  
  manager_composition_add (channel->manager,
			   &channel->a_port_aid,
			   port_out,
			   NULL,
			   &channel->self,
			   channel_a_in,
			   NULL);
  
  manager_composition_add (channel->manager,
			   &channel->self,
			   channel_a_out,
			   NULL,
			   &channel->a_port_aid,
			   port_in,
			   NULL);
  
  manager_composition_add (channel->manager,
			   &channel->b_port_aid,
			   port_out,
			   NULL,
			   &channel->self,
			   channel_b_in,
			   NULL);
  
  manager_composition_add (channel->manager,
			   &channel->self,
			   channel_b_out,
			   NULL,
			   &channel->b_port_aid,
			   port_in,
			   NULL);
  
  return channel;
}

static void
channel_a_callback (void* state, void* param, bid_t bid)
{
  channel_t* channel = state;
  assert (channel != NULL);

  buffer_incref (bid);
  assert (buffer_size (bid) == sizeof (proxy_receipt_t));
  const proxy_receipt_t* proxy_receipt = buffer_read_ptr (bid);
  assert (buffer_size (proxy_receipt->bid) == sizeof (port_receipt_t));
  const port_receipt_t* port_receipt = buffer_read_ptr (proxy_receipt->bid);
  buffer_decref (bid);
  manager_proxy_receive (channel->manager, proxy_receipt);
  channel->a_port = port_receipt->port;
}

static void
channel_b_callback (void* state, void* param, bid_t bid)
{
  channel_t* channel = state;
  assert (channel != NULL);

  buffer_incref (bid);
  assert (buffer_size (bid) == sizeof (proxy_receipt_t));
  const proxy_receipt_t* proxy_receipt = buffer_read_ptr (bid);
  assert (buffer_size (proxy_receipt->bid) == sizeof (port_receipt_t));
  const port_receipt_t* port_receipt = buffer_read_ptr (proxy_receipt->bid);
  buffer_decref (bid);
  manager_proxy_receive (channel->manager, proxy_receipt);
  channel->b_port = port_receipt->port;
}

void
channel_strobe (void* state, void* param, bid_t bid)
{
  assert (schedule_system_output () == 0);
}

static void
channel_system_input (void* state, void* param, bid_t bid)
{
  channel_t* channel = state;
  assert (channel != NULL);

  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));
  const receipt_t* receipt = buffer_read_ptr (bid);

  manager_apply (channel->manager, receipt);
}

static bid_t
channel_system_output (void* state, void* param)
{
  channel_t* channel = state;
  assert (channel != NULL);

  return manager_action (channel->manager);
}

void
channel_a_in (void* state, void* param, bid_t bid)
{
  channel_t* channel = state;
  assert (channel != NULL);

  assert (buffer_size (bid) == sizeof (message_t));
  const message_t* m = buffer_read_ptr (bid);
  assert (uuid_compare (m->src_component, channel->a_component) == 0);
  assert (m->src_port_type == channel->a_port_type);
  assert (m->src_message < channel->a_to_b_map_size);
  assert (channel->a_to_b_map[m->src_message] != -1);

  buffer_incref (bid);
  bidq_push_back (channel->a_to_b, bid);
  assert (schedule_output (channel_b_out, NULL) == 0);
}

bid_t
channel_a_out (void* state, void* param)
{
  channel_t* channel = state;
  assert (channel != NULL);

  if (!bidq_empty (channel->b_to_a)) {
    bid_t bid = bidq_front (channel->b_to_a);
    bidq_pop_front (channel->b_to_a);

    bid_t b = buffer_dup (bid, buffer_size (bid));
    message_t* m = buffer_write_ptr (b);
    uuid_copy (m->dst_component, channel->a_component);
    m->dst_port_type = channel->a_port_type;
    m->dst_port = channel->a_port;
    m->dst_message = channel->b_to_a_map[m->src_message];

    assert (schedule_output (channel_a_out, NULL) == 0);
    return b;
  }
  else {
    return -1;
  }
}

void
channel_b_in (void* state, void* param, bid_t bid)
{
  channel_t* channel = state;
  assert (channel != NULL);

  const message_t* m = buffer_read_ptr (bid);
  assert (m->src_component == channel->b_component);
  assert (m->src_port_type == channel->b_port_type);
  assert (m->src_message < channel->b_to_a_map_size);
  assert (channel->b_to_a_map[m->src_message] != -1);

  buffer_incref (bid);
  bidq_push_back (channel->b_to_a, bid);
  assert (schedule_output (channel_a_out, NULL) == 0);
}

bid_t
channel_b_out (void* state, void* param)
{
  channel_t* channel = state;
  assert (channel != NULL);

  if (!bidq_empty (channel->a_to_b)) {
    bid_t bid = bidq_front (channel->a_to_b);
    bidq_pop_front (channel->a_to_b);

    bid_t b = buffer_dup (bid, buffer_size (bid));
    message_t* m = buffer_write_ptr (b);
    uuid_copy (m->dst_component, channel->b_component);
    m->dst_port_type = channel->b_port_type;
    m->dst_port = channel->b_port;
    m->dst_message = channel->a_to_b_map[m->src_message];

    assert (schedule_output (channel_b_out, NULL) == 0);
    return b;
  }
  else {
    return -1;
  }
}

static input_t channel_free_inputs[] = {
  channel_a_callback,
  channel_b_callback,
  channel_strobe,
  NULL
};

static input_t channel_inputs[] = {
  channel_a_in,
  channel_b_in,
  NULL
};

static output_t channel_outputs[] = {
  channel_a_out,
  channel_b_out,
  NULL
};

const descriptor_t channel_descriptor = {
  .constructor = channel_create,
  .system_input = channel_system_input,
  .system_output = channel_system_output,
  .free_inputs = channel_free_inputs,
  .inputs = channel_inputs,
  .outputs = channel_outputs,
};
