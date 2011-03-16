#include "component.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>

typedef struct {
  uint32_t port;
} instance_request_t;

bid_t
instance_request_create (uint32_t port)
{
  bid_t bid = buffer_alloc (sizeof (instance_request_t));
  instance_request_t* request = buffer_write_ptr (bid);
  request->port = port;
  return bid;
}

bid_t
instance_receipt_create (instance_receipt_status_t status,
			      uint32_t instance)
{
  bid_t bid = buffer_alloc (sizeof (instance_receipt_t));
  instance_receipt_t* receipt = buffer_write_ptr (bid);
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

static void
component_process_instance_requests (void*,
				     void*);

static void
component_automaton_created (void* state,
			     void* param,
			     receipt_type_t receipt)
{
  component_t* component = state;
  assert (component != NULL);
  assert (receipt == CHILD_CREATED);

  /* Might have requests waiting. */
  assert (schedule_internal (component_process_instance_requests, NULL) == 0);
}

static void
component_instance_created (void* state,
			    void* param,
			    receipt_type_t receipt)
{
  component_t* component = state;
  assert (component != NULL);
  instance_t* instance = param;
  assert (instance != NULL);

  if (receipt == CHILD_CREATED) {
    instance->aid2 = instance->aid;
    assert (automan_proxy_send_created (instance->aid,
					instance_receipt_create (INSTANCE_REQUEST_OKAY, instance->instance),
					&instance->request) == 0);
  }
  else if (receipt == CHILD_DESTROYED) {
    /* Instance died. Rescind the parameter. */
    assert (automan_rescind (component->automan,
			     &instance->declared) == 0);
  }
  else {
    assert (0);
  }
}

static void
component_instance_declared (void* state,
			     void* param,
			     receipt_type_t receipt)
{
  component_t* component = state;
  assert (component != NULL);
  instance_t* instance = param;
  assert (instance != NULL);

  if (receipt == DECLARED) {
    /* Okay. */
  }
  else if (receipt == RESCINDED) {
    /* Return the instance to the allocator. */
    instance_allocator_return_instance (component->instance_allocator,
					instance->port,
					instance->instance);
    /* Send the destoyed. */
    assert (automan_proxy_send_destroyed (instance->aid2,
					  &instance->request) == 0);
    /* Find the instance and remove it. */
    instance_t** ptr;
    for (ptr = &component->instances;
	 *ptr != NULL &&
	   *ptr != instance;
	 ptr = &(*ptr)->next)
      ;;
    assert (*ptr == instance);
    *ptr = instance->next;
    free (instance);
  }
  else {
    assert (0);
  }
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

  component->instance_allocator = instance_allocator_create (arg->port_descriptors);

  assert (automan_create (component->automan,
  			  &component->automaton,
  			  arg->descriptor,
  			  arg->create_arg,
  			  component_automaton_created,
  			  NULL) == 0);

  component->request_proxy = arg->request_proxy;

  component->port_descriptors = arg->port_descriptors;

  uuid_copy (component->component_id, arg->component_id);

  component->instances = NULL;

  component->instance_requestq = bidq_create ();

  json_object* description = encode_description (component);
  component->description_arg.file = mftp_File_create_buffer (json_object_to_json_string (description),
  								     strlen (json_object_to_json_string (description)) + 1,
  								     COMPONENT_DESCRIPTION);
  json_object_put (description);
  /* printf ("%s\n", component->description_arg.file->data); */
  component->description_arg.announce = true;
  component->description_arg.download = false;
  component->description_arg.msg_sender = arg->msg_sender;
  component->description_arg.msg_receiver = arg->msg_receiver;
  assert (automan_create (component->automan,
  			  &component->description,
  			  &file_server_descriptor,
  			  &component->description_arg,
  			  NULL,
  			  NULL) == 0);
  
  component->instance_summary_arg.announce = true;
  component->instance_summary_arg.download = false;
  component->instance_summary_arg.msg_sender = arg->msg_sender;
  component->instance_summary_arg.msg_receiver = arg->msg_receiver;

  build_instance_summary_server (component);

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
  component_t* component = state;
  assert (component != NULL);

  return automan_action (component->automan);
}

void
component_request_instance (void* state, void* param, bid_t bid)
{
  assert (state != NULL);
  component_t* component = state;

  buffer_incref (bid);
  bidq_push_back (component->instance_requestq, bid);

  assert (schedule_internal (component_process_instance_requests, NULL) == 0);
}

static void
component_process_instance_requests (void* state,
				     void* param)
{
  assert (state != NULL);
  component_t* component = state;

  if (!bidq_empty (component->instance_requestq) &&
      component->automaton != -1) {
    bid_t bid = bidq_front (component->instance_requestq);
    bidq_pop_front (component->instance_requestq);

    assert (buffer_size (bid) == sizeof (proxy_request_t));
    const proxy_request_t* proxy_request = buffer_read_ptr (bid);

    assert (buffer_size (proxy_request->bid) == sizeof (instance_request_t));
    const instance_request_t* request = buffer_read_ptr (proxy_request->bid);

    if (request->port >= instance_allocator_port_count (component->instance_allocator)) {
      /* Requested port was out of bounds. */
      assert (automan_proxy_send_not_created (instance_receipt_create (INSTANCE_REQUEST_DNE, -1),
					      proxy_request) == 0);
      return;
    }
    
    if (!instance_allocator_contains_free_instance (component->instance_allocator, request->port)) {
      /* Out of instances. */
      assert (automan_proxy_send_not_created (instance_receipt_create (INSTANCE_REQUEST_UNAVAILABLE, -1),
					      proxy_request) == 0);
      return;
    }

    instance_t* instance = malloc (sizeof (instance_t));
    instance->request = *proxy_request;
    /* Set the port. */
    instance->port = request->port;
    /* Set the instance. */
    instance->instance = instance_allocator_get_instance (component->instance_allocator, request->port);
    /* Add a child for the instance. */
    assert (automan_declare (component->automan,
			     &instance->declared,
			     instance,
			     component_instance_declared,
			     instance) == 0);
    instance_create_arg_init (&instance->create_arg,
			      component->automaton,
			      component->request_proxy,
			      component->port_descriptors,
			      instance_allocator_input_message_count (component->instance_allocator, request->port),
			      instance_allocator_output_message_count (component->instance_allocator, request->port),
			      component->component_id,
			      instance->port,
			      instance->instance);
    assert (automan_create (component->automan,
			    &instance->aid,
			    &instance_descriptor,
			    &instance->create_arg,
			    component_instance_created,
			    instance) == 0);
    
    instance->next = component->instances;
    component->instances = instance;
    
    rebuild_instance_summary_server (component);
        
    buffer_decref (bid);

    assert (schedule_internal (component_process_instance_requests, NULL) == 0);
  }
}

static input_t component_free_inputs[] = {
  component_request_instance,
  NULL
};

static internal_t component_internals[] = {
  component_process_instance_requests,
  NULL,
};

descriptor_t component_descriptor = {
  .constructor = component_create,
  .system_input = component_system_input,
  .system_output = component_system_output,
  .free_inputs = component_free_inputs,
  .internals = component_internals,
};
