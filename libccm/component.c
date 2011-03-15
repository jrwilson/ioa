#include <ccm.h>

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <uuid/uuid.h>

#include "port_allocator.h"
#include "port.h"
#include <json/json.h>
#include <string.h>
#include "mftp.h"
#include <automan.h>

#define COMPONENT_DESCRIPTION 0
#define COMPONENT_CHANNEL_SUMMARY 1

typedef struct {
  uint32_t port;
} port_instance_request_t;

bid_t
port_instance_request_create (uint32_t port)
{
  bid_t bid = buffer_alloc (sizeof (port_instance_request_t));
  port_instance_request_t* request = buffer_write_ptr (bid);
  request->port = port;
  return bid;
}

bid_t
port_instance_receipt_create (port_instance_receipt_status_t status,
			      uint32_t instance)
{
  bid_t bid = buffer_alloc (sizeof (port_instance_receipt_t));
  port_instance_receipt_t* receipt = buffer_write_ptr (bid);
  receipt->status = status;
  receipt->instance = instance;
  return bid;
}

void
component_create_arg_init (component_create_arg_t* arg,
			   const descriptor_t* descriptor,
			   const void* create_arg,
			   input_t request_proxy,
			   const uuid_t component_id,
			   const port_descriptor_t* port_descriptors,
			   aid_t msg_sender,
			   aid_t msg_receiver)
{
  assert (arg != NULL);
  assert (descriptor != NULL);
  assert (request_proxy != NULL);
  assert (port_descriptors != NULL);
  assert (msg_sender != -1);
  assert (msg_receiver != -1);

  arg->descriptor = descriptor;
  arg->create_arg = create_arg;
  arg->request_proxy = request_proxy;
  uuid_copy (arg->component_id, component_id);
  arg->port_descriptors = port_descriptors;
  arg->msg_sender = msg_sender;
  arg->msg_receiver = msg_receiver;
}

typedef struct port_instance_struct port_instance_t;
struct port_instance_struct {
  proxy_request_t request;
  uint32_t port;
  uint32_t instance;
  bool declared;
  port_create_arg_t create_arg;
  aid_t aid;
  port_instance_t* next;
};

static void
component_process_port_requests (void*,
				 void*);

typedef struct {
  aid_t self;
  automan_t* automan;

  port_allocator_t* port_allocator;

  aid_t automaton;
  input_t request_proxy;
  const port_descriptor_t* port_descriptors;
  uuid_t component_id;
  port_instance_t* port_instances;
  bidq_t* port_requestq;

  /* file_server_create_arg_t description_arg; */
  /* aid_t description; */

  /* file_server_create_arg_t channel_summary_arg; */
  /* aid_t channel_summary; */
} component_t;

static void
component_automaton_created (void* state,
			     void* param,
			     receipt_type_t receipt)
{
  component_t* component = state;
  assert (component != NULL);
  assert (receipt == CHILD_CREATED);

  /* Might have requests waiting. */
  assert (schedule_internal (component_process_port_requests, NULL) == 0);
}

static void
component_port_created (void* state,
			void* param,
			receipt_type_t receipt)
{
  component_t* component = state;
  assert (component != NULL);
  port_instance_t* port_instance = param;
  assert (port_instance != NULL);
  assert (receipt == CHILD_CREATED);

  assert (automan_proxy_send (port_instance->aid,
			      port_instance_receipt_create (PORT_INSTANCE_OKAY, port_instance->instance),
			      &port_instance->request) == 0);
}

static void*
component_create (const void* a)
{
  const component_create_arg_t* arg = a;
  assert (arg != NULL);
  assert (arg->descriptor != NULL);
  assert (arg->port_descriptors != NULL);

  component_t* component = malloc (sizeof (component_t));

  component->automan = automan_creat (component,
				      &component->self);

  component->port_allocator = port_allocator_create (arg->port_descriptors);

  assert (automan_create (component->automan,
  			  &component->automaton,
  			  arg->descriptor,
  			  arg->create_arg,
  			  component_automaton_created,
  			  NULL) == 0);

  component->request_proxy = arg->request_proxy;

  component->port_descriptors = arg->port_descriptors;

  uuid_copy (component->component_id, arg->component_id);

  component->port_instances = NULL;

  component->port_requestq = bidq_create ();

  /* json_object* description = encode_descriptor (component); */
  /* component->description_arg.file = mftp_File_create_buffer (json_object_to_json_string (description), */
  /* 								     strlen (json_object_to_json_string (description)) + 1, */
  /* 								     COMPONENT_DESCRIPTION); */
  /* json_object_put (description); */
  /* //  printf ("%s\n", component->description_arg.file->data); */
  /* component->description_arg.announce = true; */
  /* component->description_arg.download = false; */
  /* component->description_arg.msg_sender = arg->msg_sender; */
  /* component->description_arg.msg_receiver = arg->msg_receiver; */
  /* assert (automan_create (component->automan, */
  /* 			  &component->description, */
  /* 			  &file_server_descriptor, */
  /* 			  &component->description_arg, */
  /* 			  NULL, */
  /* 			  NULL) == 0); */
  
  /* component->channel_summary_arg.announce = true; */
  /* component->channel_summary_arg.download = false; */
  /* component->channel_summary_arg.msg_sender = arg->msg_sender; */
  /* component->channel_summary_arg.msg_receiver = arg->msg_receiver; */

  /* build_channel_summary_server (component); */

  return component;
}

static bid_t component_system_output (void* state, void* param);

static void
component_system_input (void* state, void* param, bid_t bid)
{
  component_t* component = state;
  assert (component != NULL);

  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));
  const receipt_t* receipt = buffer_read_ptr (bid);

  automan_apply (component->automan, receipt);
}

static bid_t
component_system_output (void* state, void* param)
{
  assert (state != NULL);
  component_t* component = state;

  return automan_action (component->automan);
}

void
component_request_port_instance (void* state, void* param, bid_t bid)
{
  assert (state != NULL);
  component_t* component = state;

  buffer_incref (bid);
  bidq_push_back (component->port_requestq, bid);

  assert (schedule_internal (component_process_port_requests, NULL) == 0);
}

static void
component_process_port_requests (void* state,
				 void* param)
{
  assert (state != NULL);
  component_t* component = state;

  if (!bidq_empty (component->port_requestq) &&
      component->automaton != -1) {
    bid_t bid = bidq_front (component->port_requestq);
    bidq_pop_front (component->port_requestq);

    assert (buffer_size (bid) == sizeof (proxy_request_t));
    const proxy_request_t* proxy_request = buffer_read_ptr (bid);

    assert (buffer_size (proxy_request->bid) == sizeof (port_instance_request_t));
    const port_instance_request_t* request = buffer_read_ptr (proxy_request->bid);

    if (request->port >= port_allocator_port_count (component->port_allocator)) {
      /* Requested port was out of bounds. */
      assert (automan_proxy_send (-1,
      				  port_instance_receipt_create (PORT_INSTANCE_DNE, -1),
      				  proxy_request) == 0);
      return;
    }
    
    if (!port_allocator_contains_free_instance (component->port_allocator, request->port)) {
      /* TODO: Out of ports. */
      assert (0);
    }

    port_instance_t* port_instance = malloc (sizeof (port_instance_t));
    port_instance->request = *proxy_request;
    /* Set the port. */
    port_instance->port = request->port;
    /* Set the port instance. */
    port_instance->instance = port_allocator_get_free_instance (component->port_allocator, request->port);
    /* Add a child for the port. */
    assert (automan_declare (component->automan,
			     &port_instance->declared,
			     port_instance,
			     NULL,
			     NULL) == 0);
    port_create_arg_init (&port_instance->create_arg,
			  component->automaton,
			  component->request_proxy,
			  component->port_descriptors,
			  port_allocator_input_message_count (component->port_allocator, request->port),
			  port_allocator_output_message_count (component->port_allocator, request->port),
			  component->component_id,
			  port_instance->port,
			  port_instance->instance);
    assert (automan_create (component->automan,
			    &port_instance->aid,
			    &port_descriptor,
			    &port_instance->create_arg,
			    component_port_created,
			    port_instance) == 0);
    
    port_instance->next = component->port_instances;
    component->port_instances = port_instance;
    
    /* rebuild_channel_summary_server (component); */
        
    buffer_decref (bid);
  }
}

static input_t component_free_inputs[] = {
  component_request_port_instance,
  NULL
};

static internal_t component_internals[] = {
  component_process_port_requests,
  NULL,
};

descriptor_t component_descriptor = {
  .constructor = component_create,
  .system_input = component_system_input,
  .system_output = component_system_output,
  .free_inputs = component_free_inputs,
  .internals = component_internals,
};
