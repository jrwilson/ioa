#include "integer_display.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "integer_display_proxy.h"
#include "component_manager.h"
#include <automan.h>

typedef struct {
  proxy_request_t request;
  bool declared;
  aid_t aid;
} integer_display_proxy_t;

static void integer_display_proxy_created (void*, void*, receipt_type_t);

typedef struct {
  aid_t self;
  automan_t* automan;
} integer_display_t;

static void*
integer_display_create (const void* arg)
{
  integer_display_t* integer_display = malloc (sizeof (integer_display_t));

  integer_display->automan = automan_creat (integer_display, &integer_display->self);

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

  automan_apply (integer_display->automan, receipt);
}

static bid_t
integer_display_system_output (void* state, void* param)
{
  integer_display_t* integer_display = state;
  assert (integer_display != NULL);

  return automan_action (integer_display->automan);
}

void
integer_display_request_proxy (void* state, void* param, bid_t bid)
{
  integer_display_t* integer_display = state;
  assert (integer_display != NULL);

  assert (buffer_size (bid) == sizeof (proxy_request_t));

  const proxy_request_t* request = buffer_read_ptr (bid);
  integer_display_proxy_t* integer_display_proxy = malloc (sizeof (integer_display_proxy_t));
  integer_display_proxy->request = *request;
  assert (automan_declare (integer_display->automan,
			   &integer_display_proxy->declared,
			   integer_display_proxy,
			   NULL,
			   NULL) == 0);
  assert (automan_create (integer_display->automan,
			  &integer_display_proxy->aid,
			  &integer_display_proxy_descriptor,
			  NULL,
			  integer_display_proxy_created,
			  integer_display_proxy) == 0);
  assert (schedule_system_output () == 0);
}

static void
integer_display_proxy_created (void* state, void* param, receipt_type_t receipt)
{
  integer_display_t* integer_display = state;
  assert (integer_display != NULL);

  integer_display_proxy_t* integer_display_proxy = param;
  assert (integer_display_proxy != NULL);

  assert (automan_proxy_send (integer_display_proxy->aid, -1, &integer_display_proxy->request) == 0);
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
