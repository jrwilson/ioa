#include "component_manager.h"

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

void
component_manager_create_arg_init (component_manager_create_arg_t* arg,
				   descriptor_t* component_descriptor,
				   void* create_arg,
				   input_t request_proxy,
				   uuid_t component,
				   port_type_descriptor_t* port_type_descriptors,
				   aid_t msg_sender,
				   aid_t msg_receiver)
{
  assert (arg != NULL);
  assert (component_descriptor != NULL);
  assert (request_proxy != NULL);
  assert (port_type_descriptors != NULL);
  assert (msg_sender != -1);
  assert (msg_receiver != -1);

  arg->component_descriptor = component_descriptor;
  arg->create_arg = create_arg;
  arg->request_proxy = request_proxy;
  uuid_copy (arg->component, component);
  arg->port_type_descriptors = port_type_descriptors;
  arg->msg_sender = msg_sender;
  arg->msg_receiver = msg_receiver;
}

typedef struct port_proxy_struct {
  aid_t aid;
} port_proxy_t;

typedef struct port_struct port_t;
struct port_struct {
  port_create_arg_t port_create_arg;
  aid_t port_aid;
  port_t* next;
};

typedef struct {
  port_type_descriptor_t* port_type_descriptors;
  aid_t component_aid;
  input_t request_proxy;
  uuid_t component;
  port_allocator_t* port_allocator;

  file_server_create_arg_t description_arg;
  aid_t description;

  file_server_create_arg_t channel_summary_arg;
  aid_t channel_summary;
  
  port_t* ports;
  automan_t* automan;
  aid_t self;
} component_manager_t;

static json_object*
encode_input_messages (component_manager_t* component_manager, uint32_t port_type)
{
  json_object* array = json_object_new_array ();

  uint32_t input_message;
  for (input_message = 0;
       input_message < port_allocator_input_message_count (component_manager->port_allocator, port_type);
       ++input_message) {
    json_object* object = json_object_new_object ();
    json_object_object_add (object, "name", json_object_new_string (component_manager->port_type_descriptors[port_type].input_messages[input_message].name));
    json_object_object_add (object, "type", json_object_new_string (component_manager->port_type_descriptors[port_type].input_messages[input_message].type));
    json_object_array_add (array, object);
  }

  return array;
}

static json_object*
encode_output_messages (component_manager_t* component_manager, uint32_t port_type)
{
  json_object* array = json_object_new_array ();

  uint32_t output_message;
  for (output_message = 0;
       output_message < port_allocator_output_message_count (component_manager->port_allocator, port_type);
       ++output_message) {
    json_object* object = json_object_new_object ();
    json_object_object_add (object, "name", json_object_new_string (component_manager->port_type_descriptors[port_type].output_messages[output_message].name));
    json_object_object_add (object, "type", json_object_new_string (component_manager->port_type_descriptors[port_type].output_messages[output_message].type));
    json_object_array_add (array, object);
  }

  return array;
}

static json_object*
encode_port_type (component_manager_t* component_manager, uint32_t port_type)
{
  json_object* object = json_object_new_object ();

  json_object_object_add (object, "cardinality", json_object_new_int (port_allocator_cardinality (component_manager->port_allocator, port_type)));
  json_object_object_add (object, "input messages", encode_input_messages (component_manager, port_type));
  json_object_object_add (object, "output messages", encode_output_messages (component_manager, port_type));

  return object;
}

static json_object*
encode_port_types (component_manager_t* component_manager)
{
  json_object* array = json_object_new_array ();

  uint32_t port_type;
  for (port_type = 0;
       port_type < port_allocator_port_type_count (component_manager->port_allocator);
       ++port_type) {
    json_object_array_add (array, encode_port_type (component_manager, port_type));
  }

  return array;
}

static json_object* 
encode_descriptor (component_manager_t* component_manager)
{
  char u[37];
  uuid_unparse (component_manager->component, u);

  json_object* object = json_object_new_object ();
  json_object_object_add (object, "component", json_object_new_string (u));
  json_object_object_add (object, "port types", encode_port_types (component_manager));

  return object;
}

static json_object*
encode_port_type_summary (component_manager_t* component_manager, uint32_t port_type)
{
  json_object* object = json_object_new_object ();

  json_object_object_add (object, "cardinality", json_object_new_int (port_allocator_cardinality (component_manager->port_allocator, port_type)));
  json_object_object_add (object, "count", json_object_new_int (port_allocator_free_count (component_manager->port_allocator, port_type)));

  return object;
}

static json_object*
encode_port_type_summaries (component_manager_t* component_manager)
{
  json_object* array = json_object_new_array ();

  uint32_t port_type;
  for (port_type = 0;
       port_type < port_allocator_port_type_count (component_manager->port_allocator);
       ++port_type) {
    json_object_array_add (array, encode_port_type_summary (component_manager, port_type));
  }

  return array;
}

static json_object* 
encode_channel_summary (component_manager_t* component_manager)
{
  char u[37];
  uuid_unparse (component_manager->component, u);

  json_object* object = json_object_new_object ();
  json_object_object_add (object, "component", json_object_new_string (u));
  json_object_object_add (object, "port types", encode_port_type_summaries (component_manager));

  return object;
}

static void
build_channel_summary_server (component_manager_t* component_manager)
{
  json_object* channel_summary = encode_channel_summary (component_manager);
  component_manager->channel_summary_arg.file = mftp_File_create_buffer (json_object_to_json_string (channel_summary),
								     strlen (json_object_to_json_string (channel_summary)) + 1,
								     COMPONENT_CHANNEL_SUMMARY);
  json_object_put (channel_summary);
  //  printf ("%s\n", component_manager->channel_summary_arg.file->data);
  assert (automan_create (component_manager->automan,
			  &component_manager->channel_summary,
			  &file_server_descriptor,
			  &component_manager->channel_summary_arg,
			  NULL,
			  NULL) == 0);
}

static void
destroy_channel_summary_server (component_manager_t* component_manager)
{
  assert (automan_destroy (component_manager->automan,
			   &component_manager->channel_summary) == 0);
  /* assert (0); */
  /* json_object* channel_summary = encode_channel_summary (component_manager); */
  /* component_manager->channel_summary_arg.file = mftp_File_create_buffer (json_object_to_json_string (channel_summary), */
  /* 								     strlen (json_object_to_json_string (channel_summary)) + 1, */
  /* 								     COMPONENT_CHANNEL_SUMMARY); */
  /* json_object_put (channel_summary); */
  /* printf ("%s\n", component_manager->channel_summary_arg.file->data); */
}

static void
rebuild_channel_summary_server (component_manager_t* component_manager)
{
  destroy_channel_summary_server (component_manager);
  build_channel_summary_server (component_manager);
}

static void*
component_manager_create (const void* a)
{
  const component_manager_create_arg_t* arg = a;
  assert (arg != NULL);
  assert (arg->component_descriptor != NULL);
  assert (arg->port_type_descriptors != NULL);

  component_manager_t* component_manager = malloc (sizeof (component_manager_t));

  component_manager->port_type_descriptors = arg->port_type_descriptors;
  component_manager->request_proxy = arg->request_proxy;
  uuid_copy (component_manager->component, arg->component);
  component_manager->port_allocator = port_allocator_create (arg->port_type_descriptors);
  component_manager->ports = NULL;


  component_manager->automan = automan_creat (component_manager, &component_manager->self);

  assert (automan_create (component_manager->automan,
			  &component_manager->component_aid,
			  arg->component_descriptor,
			  arg->create_arg,
			  NULL,
			  NULL) == 0);

  json_object* description = encode_descriptor (component_manager);
  component_manager->description_arg.file = mftp_File_create_buffer (json_object_to_json_string (description),
								     strlen (json_object_to_json_string (description)) + 1,
								     COMPONENT_DESCRIPTION);
  json_object_put (description);
  //  printf ("%s\n", component_manager->description_arg.file->data);
  component_manager->description_arg.announce = true;
  component_manager->description_arg.download = false;
  component_manager->description_arg.msg_sender = arg->msg_sender;
  component_manager->description_arg.msg_receiver = arg->msg_receiver;
  assert (automan_create (component_manager->automan,
			  &component_manager->description,
			  &file_server_descriptor,
			  &component_manager->description_arg,
			  NULL,
			  NULL) == 0);
  
  component_manager->channel_summary_arg.announce = true;
  component_manager->channel_summary_arg.download = false;
  component_manager->channel_summary_arg.msg_sender = arg->msg_sender;
  component_manager->channel_summary_arg.msg_receiver = arg->msg_receiver;

  build_channel_summary_server (component_manager);

  return component_manager;
}

static bid_t component_manager_system_output (void* state, void* param);

static void
component_manager_system_input (void* state, void* param, bid_t bid)
{
  component_manager_t* component_manager = state;
  assert (component_manager != NULL);

  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));
  const receipt_t* receipt = buffer_read_ptr (bid);

  automan_apply (component_manager->automan, receipt);
}

static bid_t
component_manager_system_output (void* state, void* param)
{
  assert (state != NULL);
  component_manager_t* component_manager = state;

  return automan_action (component_manager->automan);
}

void
component_manager_request_port (void* state, void* param, bid_t bid)
{
  printf ("%s\n", __func__);
  assert (state != NULL);
  component_manager_t* component_manager = state;

  /* Increment the reference count so we can read the child. */
  buffer_incref (bid);

  assert (buffer_size (bid) == sizeof (proxy_request_t));
  const proxy_request_t* proxy_request = buffer_read_ptr (bid);

  assert (buffer_size (proxy_request->bid) == sizeof (port_request_t));
  const port_request_t* request = buffer_read_ptr (proxy_request->bid);

  buffer_decref (bid);

  if (request->port_type >= port_allocator_port_type_count (component_manager->port_allocator)) {
    /* TODO: Out of bounds. */
    assert (0);
  }

  if (!port_allocator_contains_free_port (component_manager->port_allocator, request->port_type)) {
    /* TODO: Out of ports. */
    assert (0);
  }

  port_t* port = malloc (sizeof (port_t));
  /* Set the port type. */
  uint32_t port_type = request->port_type;
  /* Set the port index. */
  uint32_t port_idx = port_allocator_get_free_port (component_manager->port_allocator, request->port_type);
  /* Add a child for the port. */
  port_create_arg_init (&port->port_create_arg,
			component_manager->component_aid,
			component_manager->request_proxy,
			component_manager->port_type_descriptors,
			port_allocator_input_message_count (component_manager->port_allocator, request->port_type),
			port_allocator_output_message_count (component_manager->port_allocator, request->port_type),
			component_manager->component,
			port_type,
			port_idx,
			proxy_request->callback_aid,
			proxy_request->callback_free_input);
  assert (automan_create (component_manager->automan,
			  &port->port_aid,
			  &port_descriptor,
			  &port->port_create_arg,
			  NULL,
			  NULL) == 0);

  port->next = component_manager->ports;
  component_manager->ports = port;

  rebuild_channel_summary_server (component_manager);
  
  assert (schedule_system_output () == 0);


  /* if (receipt->type == SELF_CREATED) { */
  /*   /\* /\\* Callback. *\\/ *\/ */
  /*   /\* bid_t port_bid = buffer_alloc (sizeof (port_receipt_t)); *\/ */
  /*   /\* port_receipt_t* port_receipt = buffer_write_ptr (port_bid); *\/ */
  /*   /\* port_receipt_init (port_receipt, port->port); *\/ */
  /*   /\* bid_t proxy_bid = proxy_receipt_create (receipt->self_created.self, port_bid); *\/ */
  /*   /\* assert (schedule_free_input (port->aid, port->free_input, proxy_bid) == 0); *\/ */
  /* } */
}

void
component_manager_strobe_in (void* state, void* param, bid_t bid)
{
  assert (schedule_system_output () == 0);
}

static input_t component_manager_free_inputs[] = {
  component_manager_request_port,
  component_manager_strobe_in,
  NULL
};

descriptor_t component_manager_descriptor = {
  .constructor = component_manager_create,
  .system_input = component_manager_system_input,
  .system_output = component_manager_system_output,
  .free_inputs = component_manager_free_inputs,
};

/* TODO: Factor these out. */
void
port_request_init (port_request_t* pr, uint32_t port_type)
{
  assert (pr != NULL);

  pr->port_type = port_type;
}

void
port_receipt_init (port_receipt_t* pr, uint32_t port)
{
  assert (pr != NULL);

  pr->port = port;
}
