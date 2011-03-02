#include "port_manager.h"

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#include "port_allocator.h"
#include "port.h"

void
port_manager_create_arg_init (port_manager_create_arg_t* arg,
			      descriptor_t* component_descriptor,
			      void* create_arg,
			      input_t request_proxy,
			      size_t component,
			      port_descriptor_t* port_descriptors)
{
  assert (arg != NULL);
  assert (component_descriptor != NULL);
  assert (request_proxy != NULL);
  assert (port_descriptors != NULL);

  arg->component_descriptor = component_descriptor;
  arg->create_arg = create_arg;
  arg->request_proxy = request_proxy;
  arg->component = component;
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
  size_t component;
  port_allocator_t* port_allocator;
  port_t* ports;
  manager_t* manager;
  aid_t self;
} port_manager_t;

static void*
port_manager_create (void* a)
{
  port_manager_create_arg_t* arg = a;
  assert (arg != NULL);
  assert (arg->component_descriptor != NULL);
  assert (arg->port_descriptors != NULL);

  port_manager_t* port_manager = malloc (sizeof (port_manager_t));

  port_manager->port_descriptors = arg->port_descriptors;
  port_manager->request_proxy = arg->request_proxy;
  port_manager->component = arg->component;
  port_manager->port_allocator = port_allocator_create (arg->port_descriptors);
  port_manager->ports = NULL;

  port_manager->manager = manager_create ();

  manager_self_set (port_manager->manager, &port_manager->self);
  manager_child_add (port_manager->manager, &port_manager->component_aid, arg->component_descriptor, arg->create_arg);
  
  return port_manager;
}

static bid_t port_manager_system_output (void* state, void* param);

static void
port_manager_system_input (void* state, void* param, bid_t bid)
{
  port_manager_t* port_manager = state;
  assert (port_manager != NULL);

  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));
  const receipt_t* receipt = buffer_read_ptr (bid);

  manager_apply (port_manager->manager, receipt);

  port_t* port;
  for (port = port_manager->ports;
       port != NULL;
       port = port->next) {
    if (port->port_aid != -1) {
      assert (schedule_free_input (port->port_aid, port_strobe, buffer_alloc (0)) == 0);
    }
  }
}

static bid_t
port_manager_system_output (void* state, void* param)
{
  assert (state != NULL);
  port_manager_t* port_manager = state;

  return manager_action (port_manager->manager);
}

void
port_manager_request_port (void* state, void* param, bid_t bid)
{
  assert (state != NULL);
  port_manager_t* port_manager = state;

  /* Increment the reference count so we can read the child. */
  buffer_incref (bid);

  assert (buffer_size (bid) == sizeof (proxy_request_t));
  const proxy_request_t* proxy_request = buffer_read_ptr (bid);

  assert (buffer_size (proxy_request->bid) == sizeof (port_request_t));
  const port_request_t* request = buffer_read_ptr (proxy_request->bid);

  buffer_decref (bid);

  if (request->port_type >= port_allocator_port_set_count (port_manager->port_allocator)) {
    /* TODO: Out of bounds. */
    assert (0);
  }

  if (!port_allocator_contains_free_port (port_manager->port_allocator, request->port_type)) {
    /* TODO: Out of ports. */
    assert (0);
  }

  port_t* port = malloc (sizeof (port_t));
  /* Set the port type. */
  size_t port_type = request->port_type;
  /* Set the port index. */
  size_t port_idx = port_allocator_get_free_port (port_manager->port_allocator, request->port_type);
  /* Add a child for the port. */
  port_create_arg_init (&port->port_create_arg,
			&port_manager->component_aid,
			port_manager->request_proxy,
			port_manager->port_descriptors,
			port_allocator_input_count (port_manager->port_allocator, request->port_type),
			port_allocator_output_count (port_manager->port_allocator, request->port_type),
			port_manager->component,
			port_type,
			port_idx,
			proxy_request->callback_aid,
			proxy_request->callback_free_input);
  manager_child_add (port_manager->manager, &port->port_aid, &port_descriptor, &port->port_create_arg);

  port->next = port_manager->ports;
  port_manager->ports = port;

  assert (schedule_system_output () == 0);
}

static input_t port_manager_free_inputs[] = {
  port_manager_request_port,
  NULL
};

descriptor_t port_manager_descriptor = {
  .constructor = port_manager_create,
  .system_input = port_manager_system_input,
  .system_output = port_manager_system_output,
  .free_inputs = port_manager_free_inputs,
};

/* TODO: Factor these out. */
void
port_request_init (port_request_t* pr, size_t port_type)
{
  assert (pr != NULL);

  pr->port_type = port_type;
}

void
port_receipt_init (port_receipt_t* pr, size_t port)
{
  assert (pr != NULL);

  pr->port = port;
}
