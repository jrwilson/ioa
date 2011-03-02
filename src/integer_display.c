#include "integer_display.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "integer_display_proxy.h"
#include "port_manager.h"

typedef struct {
  integer_display_proxy_create_arg_t integer_display_proxy_create_arg;
  aid_t aid;
} integer_display_proxy_t;

typedef struct {
  manager_t* manager;
} integer_display_t;

static void*
integer_display_create (void* arg)
{
  integer_display_t* integer_display = malloc (sizeof (integer_display_t));

  integer_display->manager = manager_create ();

  return integer_display;
}

static void
integer_display_system_input (void* state, void* param, bid_t bid)
{
  integer_display_t* integer_display = state;
  assert (integer_display != NULL);

  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));
  const receipt_t* receipt = buffer_read_ptr (bid);

  manager_apply (integer_display->manager, receipt);
}

static bid_t
integer_display_system_output (void* state, void* param)
{
  integer_display_t* integer_display = state;
  assert (integer_display != NULL);

  return manager_action (integer_display->manager);
}

void
integer_display_request_proxy (void* state, void* param, bid_t bid)
{
  integer_display_t* integer_display = state;
  assert (integer_display != NULL);

  assert (buffer_size (bid) == sizeof (proxy_request_t));

  const proxy_request_t* request = buffer_read_ptr (bid);
  integer_display_proxy_t* integer_display_proxy = malloc (sizeof (integer_display_proxy_t));
  integer_display_proxy->integer_display_proxy_create_arg.aid = request->callback_aid;
  integer_display_proxy->integer_display_proxy_create_arg.free_input = request->callback_free_input;
  manager_param_add (integer_display->manager,
		     integer_display_proxy);
  manager_child_add (integer_display->manager,
		     &integer_display_proxy->aid,
		     &integer_display_proxy_descriptor,
		     &integer_display_proxy->integer_display_proxy_create_arg);
  assert (schedule_system_output () == 0);
}

static input_t integer_display_free_inputs[] = {
  integer_display_request_proxy,
  NULL
};

descriptor_t integer_display_descriptor = {
  .constructor = integer_display_create,
  .system_input = integer_display_system_input,
  .system_output = integer_display_system_output,
  .free_inputs = integer_display_free_inputs,
};
