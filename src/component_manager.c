#include "component_manager.h"

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <uuid/uuid.h>

#include "port_allocator.h"
#include "port.h"

void
component_manager_create_arg_init (component_manager_create_arg_t* arg,
				   descriptor_t* component_descriptor,
				   void* create_arg,
				   input_t request_proxy,
				   uuid_t component,
				   port_descriptor_t* port_descriptors)
{
  assert (arg != NULL);
  assert (component_descriptor != NULL);
  assert (request_proxy != NULL);
  assert (port_descriptors != NULL);

  arg->component_descriptor = component_descriptor;
  arg->create_arg = create_arg;
  arg->request_proxy = request_proxy;
  uuid_copy (arg->component, component);
  arg->port_descriptors = port_descriptors;
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
  port_descriptor_t* port_descriptors;
  aid_t component_aid;
  input_t request_proxy;
  uuid_t component;
  port_allocator_t* port_allocator;
  port_t* ports;
  manager_t* manager;
  aid_t self;
} component_manager_t;

static void*
component_manager_create (void* a)
{
  component_manager_create_arg_t* arg = a;
  assert (arg != NULL);
  assert (arg->component_descriptor != NULL);
  assert (arg->port_descriptors != NULL);

  component_manager_t* component_manager = malloc (sizeof (component_manager_t));

  component_manager->port_descriptors = arg->port_descriptors;
  component_manager->request_proxy = arg->request_proxy;
  uuid_copy (component_manager->component, arg->component);
  component_manager->port_allocator = port_allocator_create (arg->port_descriptors);
  component_manager->ports = NULL;

  component_manager->manager = manager_create ();

  manager_self_set (component_manager->manager, &component_manager->self);
  manager_child_add (component_manager->manager, &component_manager->component_aid, arg->component_descriptor, arg->create_arg);
  
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

  manager_apply (component_manager->manager, receipt);

  port_t* port;
  for (port = component_manager->ports;
       port != NULL;
       port = port->next) {
    if (port->port_aid != -1) {
      assert (schedule_free_input (port->port_aid, port_strobe, buffer_alloc (0)) == 0);
    }
  }
}

static bid_t
component_manager_system_output (void* state, void* param)
{
  assert (state != NULL);
  component_manager_t* component_manager = state;

  return manager_action (component_manager->manager);
}

void
component_manager_request_port (void* state, void* param, bid_t bid)
{
  assert (state != NULL);
  component_manager_t* component_manager = state;

  /* Increment the reference count so we can read the child. */
  buffer_incref (bid);

  assert (buffer_size (bid) == sizeof (proxy_request_t));
  const proxy_request_t* proxy_request = buffer_read_ptr (bid);

  assert (buffer_size (proxy_request->bid) == sizeof (port_request_t));
  const port_request_t* request = buffer_read_ptr (proxy_request->bid);

  buffer_decref (bid);

  if (request->port_type >= port_allocator_port_set_count (component_manager->port_allocator)) {
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
			&component_manager->component_aid,
			component_manager->request_proxy,
			component_manager->port_descriptors,
			port_allocator_input_count (component_manager->port_allocator, request->port_type),
			port_allocator_output_count (component_manager->port_allocator, request->port_type),
			component_manager->component,
			port_type,
			port_idx,
			proxy_request->callback_aid,
			proxy_request->callback_free_input);
  manager_child_add (component_manager->manager, &port->port_aid, &port_descriptor, &port->port_create_arg);

  port->next = component_manager->ports;
  component_manager->ports = port;

  assert (schedule_system_output () == 0);
}

static input_t component_manager_free_inputs[] = {
  component_manager_request_port,
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
