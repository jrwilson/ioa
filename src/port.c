#include "port.h"

#include <assert.h>
#include <stdlib.h>

#include "port_manager.h"
#include "message.h"

typedef struct port_to_component_struct port_to_component_t;
struct port_to_component_struct {
  size_t message;
  port_to_component_t* next;
};

typedef struct {
  size_t message;
} component_to_port_t;

void
port_create_arg_init (port_create_arg_t* arg,
		      aid_t* component_aid,
		      input_t request_proxy,
		      port_descriptor_t* port_descriptors,
		      size_t input_count,
		      size_t output_count,
		      size_t component,
		      size_t port_type,
		      size_t port,
		      aid_t aid,
		      input_t free_input)
{
  assert (arg != NULL);
  assert (component_aid != NULL);
  assert (request_proxy != NULL);
  assert (port_descriptors != NULL);
  assert (aid != -1);
  assert (free_input != NULL);

  arg->component_aid = component_aid;
  arg->request_proxy = request_proxy;
  arg->port_descriptors = port_descriptors;
  arg->input_count = input_count;
  arg->output_count = output_count;
  arg->component = component;
  arg->port_type = port_type;
  arg->port = port;
  arg->aid = aid;
  arg->free_input = free_input;
}

static void port_callback (void* state, void* param, bid_t bid);
static void port_component_in (void* state, void* param, bid_t bid);
static bid_t port_component_out (void* state, void* param);

typedef struct port_struct {
  aid_t* component_aid;
  input_t request_proxy;
  port_descriptor_t* port_descriptors;
  size_t component;
  size_t port_type;
  size_t port;
  size_t input_count;
  size_t output_count;
  aid_t aid;
  input_t free_input;
  bidq_t* outq;
  bidq_t* inq;
  port_to_component_t* port_to_components;
  manager_t* manager;
  aid_t self;
  aid_t component_proxy;
} port_t;

static void*
port_create (void* a)
{
  port_create_arg_t* arg = a;
  assert (arg != NULL);
  assert (arg->aid != -1);
  assert (arg->free_input != NULL);

  port_t* port = malloc (sizeof (port_t));

  port->component_aid = arg->component_aid;
  port->request_proxy = arg->request_proxy;
  port->port_descriptors = arg->port_descriptors;
  port->component = arg->component;
  port->port_type = arg->port_type;
  port->port = arg->port;
  port->input_count = arg->input_count;
  port->output_count = arg->output_count;
  port->aid = arg->aid;
  port->free_input = arg->free_input;
  port->outq = bidq_create ();
  port->inq = bidq_create ();
  port->port_to_components = NULL;
  port->manager = manager_create ();

  manager_self_set (port->manager, &port->self);

  /* Add a proxy for the component. */
  manager_proxy_add (port->manager, &port->component_proxy, port->component_aid, port->request_proxy, port_callback, -1);
  
  /* Hook up component to port actions. */
  size_t input_idx;
  for (input_idx = 0;
       input_idx < port->input_count;
       ++input_idx) {
    port_to_component_t* port_to_component = malloc (sizeof (port_to_component_t));
    port_to_component->message = input_idx;
    manager_param_add (port->manager, port_to_component);
    manager_composition_add (port->manager,
			     &port->self,
			     port_component_out,
			     port_to_component,
			     &port->component_proxy,
			     port->port_descriptors[port->port_type].input_messages[input_idx],
			     NULL);
    port_to_component->next = port->port_to_components;
    port->port_to_components = port_to_component;
  }
  
  /* Hook up port to component actions. */
  size_t output_idx;
  for (output_idx = 0;
       output_idx < port->output_count;
       ++output_idx) {
    component_to_port_t* component_to_port = malloc (sizeof (component_to_port_t));
    component_to_port->message = output_idx;
    manager_param_add (port->manager, component_to_port);
    manager_composition_add (port->manager,
			     &port->component_proxy,
			     port->port_descriptors[port->port_type].output_messages[output_idx],
			     NULL,
			     &port->self,
			     port_component_in,
			     component_to_port);
  }
  
  return port;
}

static void
port_callback (void* state, void* param, bid_t bid)
{
  port_t* port = state;
  assert (port != NULL);

  assert (buffer_size (bid) == sizeof (proxy_receipt_t));
  const proxy_receipt_t* receipt = buffer_read_ptr (bid);
  manager_proxy_receive (port->manager, receipt);
}

static void
port_system_input (void* state, void* param, bid_t bid)
{
  port_t* port = state;
  assert (port != NULL);
  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));
  const receipt_t* receipt = buffer_read_ptr (bid);

  manager_apply (port->manager, receipt);

  if (receipt->type == SELF_CREATED) {
    /* Callback. */
    bid_t port_bid = buffer_alloc (sizeof (port_receipt_t));
    port_receipt_t* port_receipt = buffer_write_ptr (port_bid);
    port_receipt_init (port_receipt, port->port);
    bid_t proxy_bid = proxy_receipt_create (receipt->self_created.self, port_bid);
    assert (schedule_free_input (port->aid, port->free_input, proxy_bid) == 0);
  }
}

static bid_t
port_system_output (void* state, void* param)
{
  port_t* port = state;
  assert (port != NULL);

  return manager_action (port->manager);
}

void
port_strobe (void* state, void* param, bid_t bid)
{
  assert (schedule_system_output () == 0);
}

static void
port_component_in (void* state, void* param, bid_t bid)
{
  port_t* port = state;
  assert (port != NULL);
  component_to_port_t* component_to_port = param;
  assert (component_to_port != NULL);

  bid_t b = buffer_alloc (sizeof (message_t));
  message_t* m = buffer_write_ptr (b);
  m->src_component = port->component;
  m->src_port_type = port->port_type;
  m->src_port = port->port;
  m->src_message = component_to_port->message;
  buffer_add_child (b, bid);
  m->bid = bid;

  bidq_push_back (port->outq, b);
  assert (schedule_output (port_out, NULL) == 0);
}

static void port_demux (void* state, void* param);

static bid_t
port_component_out (void* state, void* param)
{
  port_t* port = state;
  assert (port != NULL);
  port_to_component_t* port_to_component = param;
  assert (port_to_component != NULL);

  assert (!bidq_empty (port->inq));

  bid_t bid = bidq_front (port->inq);
  bidq_pop_front (port->inq);
  const message_t* m = buffer_read_ptr (bid);
  assert (m->dst_message == port_to_component->message);
  bid_t b = m->bid;
  assert (schedule_internal (port_demux, NULL) == 0);
  return b;
}

static void
port_demux (void* state, void* param)
{
  port_t* port = state;
  assert (port != NULL);

  if (!bidq_empty (port->inq)) {
    bid_t bid = bidq_front (port->inq);
    const message_t* m = buffer_read_ptr (bid);
    /* TODO: O(1) with an array. */
    port_to_component_t* port_to_component;
    for (port_to_component = port->port_to_components;
         port_to_component != NULL;
         port_to_component = port_to_component->next) {
      if (port_to_component->message == m->dst_message) {
        break;
      }
    }
    assert (port_to_component != NULL);
    assert (schedule_output (port_component_out, port_to_component) == 0);
  }
}

void
port_in (void* state, void* param, bid_t bid)
{
  port_t* port = state;
  assert (port != NULL);

  assert (buffer_size (bid) == sizeof (message_t));
  const message_t* m = buffer_read_ptr (bid);
  assert (m->dst_component == port->component);
  assert (m->dst_port_type == port->port_type);
  assert (m->dst_port == port->port);
  assert (m->dst_message < port->input_count);

  buffer_incref (bid);
  bidq_push_back (port->inq, bid);
  assert (schedule_internal (port_demux, NULL) == 0);
}

bid_t
port_out (void* state, void* param)
{
  port_t* port = state;
  assert (port != NULL);

  if (!bidq_empty (port->outq)) {
    bid_t bid = bidq_front (port->outq);
    bidq_pop_front (port->outq);
    assert (schedule_output (port_out, NULL) == 0);
    return bid;
  }
  else {
    return -1;
  }
}

static input_t port_free_inputs[] = {
  port_strobe,
  port_callback,
  NULL
};

static input_t port_inputs[] = {
  port_component_in,
  port_in,
  NULL
};

static output_t port_outputs[] = {
  port_component_out,
  port_out,
  NULL
};

static internal_t port_internals[] = {
  port_demux,
  NULL
};

descriptor_t port_descriptor = {
  .constructor = port_create,
  .system_input = port_system_input,
  .system_output = port_system_output,
  .free_inputs = port_free_inputs,
  .inputs = port_inputs,
  .outputs = port_outputs,
  .internals = port_internals,
};
