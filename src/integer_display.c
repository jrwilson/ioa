#include "integer_display.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "integer_display_proxy.h"
#include "component_manager.h"

typedef struct {
  aid_t aid;
  aid_t callback_aid;
  input_t callback_free_input;
} integer_display_proxy_t;

static void integer_display_proxy_created (void*, void*);

typedef struct {
  aid_t self;
  manager_t* manager;
} integer_display_t;

static void*
integer_display_create (const void* arg)
{
  integer_display_t* integer_display = malloc (sizeof (integer_display_t));

  integer_display->manager = manager_create (&integer_display->self);

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
  integer_display_proxy->callback_aid = request->callback_aid;
  integer_display_proxy->callback_free_input = request->callback_free_input;
  manager_param_add (integer_display->manager,
		     integer_display_proxy);
  manager_child_add (integer_display->manager,
		     &integer_display_proxy->aid,
		     &integer_display_proxy_descriptor,
		     NULL,
		     integer_display_proxy_created,
		     integer_display_proxy);
  assert (schedule_system_output () == 0);
}

static void
integer_display_proxy_created (void* state, void* param)
{
  integer_display_t* integer_display = state;
  assert (integer_display != NULL);

  integer_display_proxy_t* integer_display_proxy = param;
  assert (integer_display_proxy != NULL);

  assert (schedule_free_input (integer_display_proxy->callback_aid, integer_display_proxy->callback_free_input, proxy_receipt_create (integer_display_proxy->aid, -1)) == 0);
}

static input_t integer_display_free_inputs[] = {
  integer_display_request_proxy,
  NULL
};

static internal_t integer_display_internals[] = {
  integer_display_proxy_created,
  NULL
};

descriptor_t integer_display_descriptor = {
  .constructor = integer_display_create,
  .system_input = integer_display_system_input,
  .system_output = integer_display_system_output,
  .free_inputs = integer_display_free_inputs,
  .internals = integer_display_internals,
};
