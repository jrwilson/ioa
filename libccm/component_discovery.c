#include "component.h"

#include <string.h>
#include <assert.h>

static json_object*
encode_input_messages (component_t* component, uint32_t port)
{
  json_object* array = json_object_new_array ();

  uint32_t input_message;
  for (input_message = 0;
       input_message < instance_allocator_input_message_count (component->instance_allocator, port);
       ++input_message) {
    json_object* object = json_object_new_object ();
    json_object_object_add (object, "name", json_object_new_string (component->port_descriptors[port].input_message_descriptors[input_message].name));
    json_object_object_add (object, "type", json_object_new_string (component->port_descriptors[port].input_message_descriptors[input_message].type));
    json_object_array_add (array, object);
  }

  return array;
}

static json_object*
encode_output_messages (component_t* component, uint32_t port)
{
  json_object* array = json_object_new_array ();

  uint32_t output_message;
  for (output_message = 0;
       output_message < instance_allocator_output_message_count (component->instance_allocator, port);
       ++output_message) {
    json_object* object = json_object_new_object ();
    json_object_object_add (object, "name", json_object_new_string (component->port_descriptors[port].output_message_descriptors[output_message].name));
    json_object_object_add (object, "type", json_object_new_string (component->port_descriptors[port].output_message_descriptors[output_message].type));
    json_object_array_add (array, object);
  }

  return array;
}

static json_object*
encode_port (component_t* component, uint32_t port)
{
  json_object* object = json_object_new_object ();

  json_object_object_add (object, "cardinality", json_object_new_int (instance_allocator_cardinality (component->instance_allocator, port)));
  json_object_object_add (object, "inputs", encode_input_messages (component, port));
  json_object_object_add (object, "outputs", encode_output_messages (component, port));

  return object;
}

static json_object*
encode_ports (component_t* component)
{
  json_object* array = json_object_new_array ();

  uint32_t port;
  for (port = 0;
       port < instance_allocator_port_count (component->instance_allocator);
       ++port) {
    json_object_array_add (array, encode_port (component, port));
  }

  return array;
}

json_object* 
encode_description (component_t* component)
{
  char u[37];
  uuid_unparse (component->component_id, u);
  
  json_object* object = json_object_new_object ();
  json_object_object_add (object, "id", json_object_new_string (u));
  json_object_object_add (object, "ports", encode_ports (component));
  
  return object;
}

static json_object*
encode_port_summary (component_t* component, uint32_t port_type)
{
  json_object* object = json_object_new_object ();

  json_object_object_add (object, "cardinality", json_object_new_int (instance_allocator_cardinality (component->instance_allocator, port_type)));
  json_object_object_add (object, "count", json_object_new_int (instance_allocator_free_count (component->instance_allocator, port_type)));

  return object;
}

static json_object*
encode_port_summaries (component_t* component)
{
  json_object* array = json_object_new_array ();

  uint32_t port;
  for (port = 0;
       port < instance_allocator_port_count (component->instance_allocator);
       ++port) {
    json_object_array_add (array, encode_port_summary (component, port));
  }

  return array;
}

static json_object* 
encode_instance_summary (component_t* component)
{
  char u[37];
  uuid_unparse (component->component_id, u);

  json_object* object = json_object_new_object ();
  json_object_object_add (object, "id", json_object_new_string (u));
  json_object_object_add (object, "ports", encode_port_summaries (component));
  
  return object;
}

void
build_instance_summary_server (component_t* component)
{
  json_object* instance_summary = encode_instance_summary (component);
  component->instance_summary_arg.file = mftp_File_create_buffer (json_object_to_json_string (instance_summary),
								  strlen (json_object_to_json_string (instance_summary)) + 1,
								  COMPONENT_INSTANCE_SUMMARY);
  /* printf ("%s\n", component->instance_summary_arg.file->data); */
  json_object_put (instance_summary);
  assert (automan_create (component->automan,
  			  &component->instance_summary,
  			  &file_server_descriptor,
  			  &component->instance_summary_arg,
  			  NULL,
  			  NULL) == 0);
}

static void
destroy_channel_summary_server (component_t* component)
{
  assert (automan_destroy (component->automan,
  			   &component->instance_summary) == 0);
  mftp_File_destroy (component->instance_summary_arg.file);
}

void
rebuild_instance_summary_server (component_t* component)
{
  destroy_channel_summary_server (component);
  build_instance_summary_server (component);
}
