#include "instance.h"

#include <assert.h>
#include <stdlib.h>

#include <automan.h>

typedef struct instance_to_component_struct instance_to_component_t;
struct instance_to_component_struct {
  bool declared;
  bool composed;
  uint32_t message;
  instance_to_component_t* next;
};

typedef struct component_to_instance_struct component_to_instance_t;
struct component_to_instance_struct {
  bool declared;
  bool composed;
  uint32_t message;
  component_to_instance_t* next;
};

void
instance_create_arg_init (instance_create_arg_t* arg,
			  aid_t component_aid,
			  input_t request_proxy,
			  const port_descriptor_t* port_descriptors,
			  uint32_t input_count,
			  uint32_t output_count,
			  uuid_t component,
			  uint32_t port,
			  uint32_t instance)
{
  assert (arg != NULL);
  assert (component_aid != -1);
  assert (request_proxy != NULL);
  assert (port_descriptors != NULL);

  arg->component_aid = component_aid;
  arg->request_proxy = request_proxy;
  arg->port_descriptors = port_descriptors;
  arg->input_count = input_count;
  arg->output_count = output_count;
  uuid_copy (arg->component, component);
  arg->port = port;
  arg->instance = instance;
}

static void instance_callback (void* state, void* param, bid_t bid);
static void instance_component_in (void* state, void* param, bid_t bid);
static bid_t instance_component_out (void* state, void* param);
static void instance_demux (void* state, void* param);

typedef struct instance_struct {
  aid_t component_aid;
  input_t request_proxy;
  const port_descriptor_t* port_descriptors;
  uuid_t component;
  uint32_t port;
  uint32_t instance;
  uint32_t input_count;
  uint32_t output_count;
  bidq_t* outq;
  bidq_t* inq;
  instance_to_component_t* instance_to_components;
  component_to_instance_t* component_to_instances;
  automan_t* automan;
  aid_t self;
  aid_t component_proxy;
  bool in_composed;
  bool out_composed;
} instance_t;

static void
instance_component_proxy_created (void* state,
				  void* param,
				  proxy_receipt_type_t receipt,
				  bid_t bid)
{
  instance_t* instance = state;
  assert (instance != NULL);

  if (receipt == PROXY_CREATED) {
    /* Okay. */
  }
  else if (receipt == PROXY_DESTROYED) {
    automan_self_destruct (instance->automan);
  }
  else {
    /* TODO:  The proxy construction failed. */
    assert (0);
  }
}

static void
instance_instance_to_component_composed (void* state,
				 void* param,
				 receipt_type_t receipt)
{
  instance_t* instance = state;
  assert (instance != NULL);
  instance_to_component_t* instance_to_component = param;
  assert (instance_to_component != NULL);

  if (receipt == COMPOSED) {
    /* Might have messages to deliver. */
    assert (schedule_internal (instance_demux, NULL) == 0);
  }
  else if (receipt == DECOMPOSED) {
    /* Okay. */
  }
  else {
    /* TODO: Composing with the proxy failed. */
    assert (0);
  }
}

static void
instance_component_to_instance_composed (void* state,
				 void* param,
				 receipt_type_t receipt)
{
  instance_t* instance = state;
  assert (instance != NULL);
  component_to_instance_t* component_to_instance = param;
  assert (component_to_instance != NULL);

  if (receipt == COMPOSED) {
    /* Okay. */
  }
  else if (receipt == DECOMPOSED) {
    /* Okay. */
  }
  else {
    /* TODO: Composing with the proxy failed. */
    assert (0);
  }
}

static void
instance_in_out_composed (void* state,
			  void* param,
			  receipt_type_t receipt)
{
  instance_t* instance = state;
  assert (instance != NULL);

  if (receipt == INPUT_DECOMPOSED ||
      receipt == OUTPUT_DECOMPOSED) {
    if (!instance->in_composed &&
	!instance->out_composed) {
      instance_to_component_t* instance_to_component;
      for (instance_to_component = instance->instance_to_components;
	   instance_to_component != NULL;
	   instance_to_component = instance_to_component->next) {
	assert (automan_rescind (instance->automan,
				 &instance_to_component->declared) == 0);
      }
      component_to_instance_t* component_to_instance;
      for (component_to_instance = instance->component_to_instances;
	   component_to_instance != NULL;
	   component_to_instance = component_to_instance->next) {
	assert (automan_rescind (instance->automan,
				 &component_to_instance->declared) == 0);
      }
    }
  }
}

static void*
instance_create (const void* a)
{
  const instance_create_arg_t* arg = a;
  assert (arg != NULL);

  instance_t* instance = malloc (sizeof (instance_t));

  instance->component_aid = arg->component_aid;
  instance->request_proxy = arg->request_proxy;
  instance->port_descriptors = arg->port_descriptors;
  uuid_copy (instance->component, arg->component);
  instance->port = arg->port;
  instance->instance = arg->instance;
  instance->input_count = arg->input_count;
  instance->output_count = arg->output_count;
  instance->outq = bidq_create ();
  instance->inq = bidq_create ();
  instance->instance_to_components = NULL;
  instance->component_to_instances = NULL;
  instance->automan = automan_creat (instance, &instance->self);

  /* Add a proxy for the component. */
  assert (automan_proxy_add (instance->automan,
			     &instance->component_proxy,
			     instance->component_aid,
			     instance->request_proxy,
			     -1,
			     instance_callback,
			     instance_component_proxy_created,
			     NULL) == 0);
  
  /* Hook up component to instance actions. */
  uint32_t input_idx;
  for (input_idx = 0;
       input_idx < instance->input_count;
       ++input_idx) {
    instance_to_component_t* instance_to_component = malloc (sizeof (instance_to_component_t));
    instance_to_component->message = input_idx;
    assert (automan_declare (instance->automan, 
			     &instance_to_component->declared,
			     instance_to_component,
			     NULL,
			     NULL) == 0);
    assert (automan_compose (instance->automan,
			     &instance_to_component->composed,
			     &instance->self,
			     instance_component_out,
			     instance_to_component,
			     &instance->component_proxy,
			     instance->port_descriptors[instance->port].input_message_descriptors[input_idx].input,
			     NULL,
			     instance_instance_to_component_composed,
			     instance_to_component) == 0);
    instance_to_component->next = instance->instance_to_components;
    instance->instance_to_components = instance_to_component;
  }
  
  /* Hook up instance to component actions. */
  uint32_t output_idx;
  for (output_idx = 0;
       output_idx < instance->output_count;
       ++output_idx) {
    component_to_instance_t* component_to_instance = malloc (sizeof (component_to_instance_t));
    component_to_instance->message = output_idx;
    assert (automan_declare (instance->automan,
			     &component_to_instance->declared,
			     component_to_instance,
			     NULL,
			     NULL) == 0);
    assert (automan_compose (instance->automan,
			     &component_to_instance->composed,
			     &instance->component_proxy,
			     instance->port_descriptors[instance->port].output_message_descriptors[output_idx].output,
			     NULL,
			     &instance->self,
			     instance_component_in,
			     component_to_instance,
			     instance_component_to_instance_composed,
			     component_to_instance) == 0);
    component_to_instance->next = instance->component_to_instances;
    instance->component_to_instances = component_to_instance;
  }

  /* Monitor our input and output. */
  assert (automan_input_add (instance->automan,
			     &instance->in_composed,
			     instance_in,
			     NULL,
			     instance_in_out_composed,
			     NULL) == 0);
  assert (automan_output_add (instance->automan,
			      &instance->out_composed,
			      instance_out,
			      NULL,
			      instance_in_out_composed,
			      NULL) == 0);
  return instance;
}

static void
instance_callback (void* state, void* param, bid_t bid)
{
  instance_t* instance = state;
  assert (instance != NULL);

  automan_proxy_receive (instance->automan, bid);
}

static void
instance_system_input (void* state, void* param, bid_t bid)
{
  instance_t* instance = state;
  assert (instance != NULL);
  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));
  const receipt_t* receipt = buffer_read_ptr (bid);

  automan_apply (instance->automan, receipt);
}

static bid_t
instance_system_output (void* state, void* param)
{
  instance_t* instance = state;
  assert (instance != NULL);

  return automan_action (instance->automan);
}

static void
instance_component_in (void* state, void* param, bid_t bid)
{
  instance_t* instance = state;
  assert (instance != NULL);
  component_to_instance_t* component_to_instance = param;
  assert (component_to_instance != NULL);

  bid_t b = buffer_alloc (sizeof (message_t));
  message_t* m = buffer_write_ptr (b);
  uuid_copy (m->src_component, instance->component);
  m->src_port = instance->port;
  m->src_instance = instance->instance;
  m->src_message = component_to_instance->message;
  buffer_add_child (b, bid);
  m->bid = bid;

  bidq_push_back (instance->outq, b);
  assert (schedule_output (instance_out, NULL) == 0);
}

static bid_t
instance_component_out (void* state, void* param)
{
  instance_t* instance = state;
  assert (instance != NULL);
  instance_to_component_t* instance_to_component = param;
  assert (instance_to_component != NULL);

  assert (!bidq_empty (instance->inq));

  bid_t bid = bidq_front (instance->inq);
  bidq_pop_front (instance->inq);
  const message_t* m = buffer_read_ptr (bid);
  assert (m->dst_message == instance_to_component->message);
  bid_t b = m->bid;
  assert (schedule_internal (instance_demux, NULL) == 0);
  return b;
}

static void
instance_demux (void* state, void* param)
{
  instance_t* instance = state;
  assert (instance != NULL);

  if (!bidq_empty (instance->inq)) {
    bid_t bid = bidq_front (instance->inq);
    const message_t* m = buffer_read_ptr (bid);
    /* TODO: O(1) with an array. */
    instance_to_component_t* instance_to_component;
    for (instance_to_component = instance->instance_to_components;
         instance_to_component != NULL;
         instance_to_component = instance_to_component->next) {
      if (instance_to_component->message == m->dst_message) {
	if (instance_to_component->composed) {
	  /* No messages will be lost going to the component because we are composed. */
	  assert (schedule_output (instance_component_out, instance_to_component) == 0);
	}
	break;
      }
    }
    assert (instance_to_component != NULL);
  }
}

void
instance_in (void* state, void* param, bid_t bid)
{
  instance_t* instance = state;
  assert (instance != NULL);

  assert (buffer_size (bid) == sizeof (message_t));
  const message_t* m = buffer_read_ptr (bid);
  assert (uuid_compare (m->dst_component, instance->component) == 0);
  assert (m->dst_port == instance->port);
  assert (m->dst_instance == instance->instance);
  assert (m->dst_message < instance->input_count);

  buffer_incref (bid);
  bidq_push_back (instance->inq, bid);
  assert (schedule_internal (instance_demux, NULL) == 0);
}

bid_t
instance_out (void* state, void* param)
{
  instance_t* instance = state;
  assert (instance != NULL);

  if (!bidq_empty (instance->outq)) {
    bid_t bid = bidq_front (instance->outq);
    bidq_pop_front (instance->outq);
    assert (schedule_output (instance_out, NULL) == 0);
    return bid;
  }
  else {
    return -1;
  }
}

static input_t instance_free_inputs[] = {
  instance_callback,
  NULL
};

static input_t instance_inputs[] = {
  instance_component_in,
  instance_in,
  NULL
};

static output_t instance_outputs[] = {
  instance_component_out,
  instance_out,
  NULL
};

static internal_t instance_internals[] = {
  instance_demux,
  NULL
};

descriptor_t instance_descriptor = {
  .constructor = instance_create,
  .system_input = instance_system_input,
  .system_output = instance_system_output,
  .free_inputs = instance_free_inputs,
  .inputs = instance_inputs,
  .outputs = instance_outputs,
  .internals = instance_internals,
};
