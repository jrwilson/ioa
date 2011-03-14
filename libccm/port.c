#include "port.h"

#include <assert.h>
#include <stdlib.h>

#include <automan.h>

typedef struct port_to_component_struct port_to_component_t;
struct port_to_component_struct {
  bool declared;
  bool composed;
  uint32_t message;
  port_to_component_t* next;
};

typedef struct {
  bool declared;
  bool composed;
  uint32_t message;
} component_to_port_t;

void
port_create_arg_init (port_create_arg_t* arg,
		      aid_t component_aid,
		      input_t request_proxy,
		      const port_type_descriptor_t* port_type_descriptors,
		      uint32_t input_count,
		      uint32_t output_count,
		      uuid_t component,
		      uint32_t port_type,
		      uint32_t port)
{
  assert (arg != NULL);
  assert (component_aid != -1);
  assert (request_proxy != NULL);
  assert (port_type_descriptors != NULL);

  arg->component_aid = component_aid;
  arg->request_proxy = request_proxy;
  arg->port_type_descriptors = port_type_descriptors;
  arg->input_count = input_count;
  arg->output_count = output_count;
  uuid_copy (arg->component, component);
  arg->port_type = port_type;
  arg->port = port;
}

static void port_callback (void* state, void* param, bid_t bid);
static void port_component_in (void* state, void* param, bid_t bid);
static bid_t port_component_out (void* state, void* param);
static void port_demux (void* state, void* param);

typedef struct port_struct {
  aid_t component_aid;
  input_t request_proxy;
  const port_type_descriptor_t* port_type_descriptors;
  uuid_t component;
  uint32_t port_type;
  uint32_t port;
  uint32_t input_count;
  uint32_t output_count;
  bidq_t* outq;
  bidq_t* inq;
  port_to_component_t* port_to_components;
  automan_t* automan;
  aid_t self;
  aid_t component_proxy;
} port_t;

static void
port_proxy_created (void* state,
		    void* param,
		    proxy_receipt_type_t receipt)
{
  port_t* port = state;
  assert (port != NULL);

  /* TODO:  The proxy construction failed. */
  assert (receipt == PROXY_CREATED); 
}

static void
port_port_to_component_composed (void* state,
				 void* param,
				 receipt_type_t receipt)
{
  port_t* port = state;
  assert (port != NULL);
  port_to_component_t* port_to_component = param;
  assert (port_to_component != NULL);

  /* TODO: Composing with the proxy failed. */
  assert (receipt == COMPOSED);

  /* Might have messages to deliver. */
  assert (schedule_internal (port_demux, NULL) == 0);
}

static void
port_component_to_port_composed (void* state,
				 void* param,
				 receipt_type_t receipt)
{
  port_t* port = state;
  assert (port != NULL);
  component_to_port_t* component_to_port = param;
  assert (component_to_port != NULL);

  /* TODO: Composing with the proxy failed. */
  assert (receipt == COMPOSED);
}

static void*
port_create (const void* a)
{
  const port_create_arg_t* arg = a;
  assert (arg != NULL);

  port_t* port = malloc (sizeof (port_t));

  port->component_aid = arg->component_aid;
  port->request_proxy = arg->request_proxy;
  port->port_type_descriptors = arg->port_type_descriptors;
  uuid_copy (port->component, arg->component);
  port->port_type = arg->port_type;
  port->port = arg->port;
  port->input_count = arg->input_count;
  port->output_count = arg->output_count;
  port->outq = bidq_create ();
  port->inq = bidq_create ();
  port->port_to_components = NULL;
  port->automan = automan_creat (port, &port->self);

  /* Add a proxy for the component. */
  assert (automan_proxy_add (port->automan,
			     &port->component_proxy,
			     port->component_aid,
			     port->request_proxy,
			     -1,
			     port_callback,
			     port_proxy_created,
			     NULL) == 0);
  
  /* Hook up component to port actions. */
  uint32_t input_idx;
  for (input_idx = 0;
       input_idx < port->input_count;
       ++input_idx) {
    port_to_component_t* port_to_component = malloc (sizeof (port_to_component_t));
    port_to_component->message = input_idx;
    assert (automan_declare (port->automan, 
			     &port_to_component->declared,
			     port_to_component,
			     NULL,
			     NULL) == 0);
    assert (automan_compose (port->automan,
			     &port_to_component->composed,
			     &port->self,
			     port_component_out,
			     port_to_component,
			     &port->component_proxy,
			     port->port_type_descriptors[port->port_type].input_messages[input_idx].input,
			     NULL,
			     port_port_to_component_composed,
			     port_to_component) == 0);
    port_to_component->next = port->port_to_components;
    port->port_to_components = port_to_component;
  }
  
  /* Hook up port to component actions. */
  uint32_t output_idx;
  for (output_idx = 0;
       output_idx < port->output_count;
       ++output_idx) {
    component_to_port_t* component_to_port = malloc (sizeof (component_to_port_t));
    component_to_port->message = output_idx;
    assert (automan_declare (port->automan,
			     &component_to_port->declared,
			     component_to_port,
			     NULL,
			     NULL) == 0);
    assert (automan_compose (port->automan,
			     &component_to_port->composed,
			     &port->component_proxy,
			     port->port_type_descriptors[port->port_type].output_messages[output_idx].output,
			     NULL,
			     &port->self,
			     port_component_in,
			     component_to_port,
			     port_component_to_port_composed,
			     component_to_port) == 0);
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
  automan_proxy_receive (port->automan, receipt);
}

static void
port_system_input (void* state, void* param, bid_t bid)
{
  port_t* port = state;
  assert (port != NULL);
  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));
  const receipt_t* receipt = buffer_read_ptr (bid);

  automan_apply (port->automan, receipt);
}

static bid_t
port_system_output (void* state, void* param)
{
  port_t* port = state;
  assert (port != NULL);

  return automan_action (port->automan);
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
  uuid_copy (m->src_component, port->component);
  m->src_port_type = port->port_type;
  m->src_port = port->port;
  m->src_message = component_to_port->message;
  buffer_add_child (b, bid);
  m->bid = bid;

  bidq_push_back (port->outq, b);
  assert (schedule_output (port_out, NULL) == 0);
}

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
	if (port_to_component->composed) {
	  /* No messages will be lost going to the component because we are composed. */
	  assert (schedule_output (port_component_out, port_to_component) == 0);
	}
	break;
      }
    }
    assert (port_to_component != NULL);
  }
}

void
port_in (void* state, void* param, bid_t bid)
{
  port_t* port = state;
  assert (port != NULL);

  assert (buffer_size (bid) == sizeof (message_t));
  const message_t* m = buffer_read_ptr (bid);
  assert (uuid_compare (m->dst_component, port->component) == 0);
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
