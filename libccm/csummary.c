static json_object*
encode_input_messages (component_t* component, uint32_t port_type)
{
  json_object* array = json_object_new_array ();

  /* uint32_t input_message; */
  /* for (input_message = 0; */
  /*      input_message < port_allocator_input_message_count (component->port_allocator, port_type); */
  /*      ++input_message) { */
  /*   json_object* object = json_object_new_object (); */
  /*   json_object_object_add (object, "name", json_object_new_string (component->port_type_descriptors[port_type].input_messages[input_message].name)); */
  /*   json_object_object_add (object, "type", json_object_new_string (component->port_type_descriptors[port_type].input_messages[input_message].type)); */
  /*   json_object_array_add (array, object); */
  /* } */

  return array;
}

static json_object*
encode_output_messages (component_t* component, uint32_t port_type)
{
  json_object* array = json_object_new_array ();

  /* uint32_t output_message; */
  /* for (output_message = 0; */
  /*      output_message < port_allocator_output_message_count (component->port_allocator, port_type); */
  /*      ++output_message) { */
  /*   json_object* object = json_object_new_object (); */
  /*   json_object_object_add (object, "name", json_object_new_string (component->port_type_descriptors[port_type].output_messages[output_message].name)); */
  /*   json_object_object_add (object, "type", json_object_new_string (component->port_type_descriptors[port_type].output_messages[output_message].type)); */
  /*   json_object_array_add (array, object); */
  /* } */

  return array;
}

static json_object*
encode_port_type (component_t* component, uint32_t port_type)
{
  json_object* object = json_object_new_object ();

  /* json_object_object_add (object, "cardinality", json_object_new_int (port_allocator_cardinality (component->port_allocator, port_type))); */
  /* json_object_object_add (object, "input messages", encode_input_messages (component, port_type)); */
  /* json_object_object_add (object, "output messages", encode_output_messages (component, port_type)); */

  return object;
}

static json_object*
encode_port_types (component_t* component)
{
  json_object* array = json_object_new_array ();

  /* uint32_t port_type; */
  /* for (port_type = 0; */
  /*      port_type < port_allocator_port_type_count (component->port_allocator); */
  /*      ++port_type) { */
  /*   json_object_array_add (array, encode_port_type (component, port_type)); */
  /* } */

  return array;
}

static json_object* 
encode_descriptor (component_t* component)
{
  /* char u[37]; */
  /* uuid_unparse (component->id, u); */

  json_object* object = json_object_new_object ();
  /* json_object_object_add (object, "component", json_object_new_string (u)); */
  /* json_object_object_add (object, "port types", encode_port_types (component)); */

  return object;
}

static json_object*
encode_port_type_summary (component_t* component, uint32_t port_type)
{
  json_object* object = json_object_new_object ();

  /* json_object_object_add (object, "cardinality", json_object_new_int (port_allocator_cardinality (component->port_allocator, port_type))); */
  /* json_object_object_add (object, "count", json_object_new_int (port_allocator_free_count (component->port_allocator, port_type))); */

  return object;
}

static json_object*
encode_port_type_summaries (component_t* component)
{
  json_object* array = json_object_new_array ();

  /* uint32_t port_type; */
  /* for (port_type = 0; */
  /*      port_type < port_allocator_port_type_count (component->port_allocator); */
  /*      ++port_type) { */
  /*   json_object_array_add (array, encode_port_type_summary (component, port_type)); */
  /* } */

  return array;
}

static json_object* 
encode_channel_summary (component_t* component)
{
  /* char u[37]; */
  /* uuid_unparse (component->id, u); */

  json_object* object = json_object_new_object ();
  /* json_object_object_add (object, "component", json_object_new_string (u)); */
  /* json_object_object_add (object, "port types", encode_port_type_summaries (component)); */

  return object;
}

static void
build_channel_summary_server (component_t* component)
{
  json_object* channel_summary = encode_channel_summary (component);
  /* component->channel_summary_arg.file = mftp_File_create_buffer (json_object_to_json_string (channel_summary), */
  /* 								     strlen (json_object_to_json_string (channel_summary)) + 1, */
  /* 								     COMPONENT_CHANNEL_SUMMARY); */
  json_object_put (channel_summary);
  //  printf ("%s\n", component->channel_summary_arg.file->data);
  /* assert (automan_create (component->automan, */
  /* 			  &component->channel_summary, */
  /* 			  &file_server_descriptor, */
  /* 			  &component->channel_summary_arg, */
  /* 			  NULL, */
  /* 			  NULL) == 0); */
}

static void
destroy_channel_summary_server (component_t* component)
{
  /* assert (automan_destroy (component->automan, */
  /* 			   &component->channel_summary) == 0); */
  /* assert (0); */
  /* json_object* channel_summary = encode_channel_summary (component); */
  /* component->channel_summary_arg.file = mftp_File_create_buffer (json_object_to_json_string (channel_summary), */
  /* 								     strlen (json_object_to_json_string (channel_summary)) + 1, */
  /* 								     COMPONENT_CHANNEL_SUMMARY); */
  /* json_object_put (channel_summary); */
  /* printf ("%s\n", component->channel_summary_arg.file->data); */
}

static void
rebuild_channel_summary_server (component_t* component)
{
  destroy_channel_summary_server (component);
  build_channel_summary_server (component);
}
