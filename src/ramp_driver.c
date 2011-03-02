#include "component_manager.h"
#include "ramp.h"
#include "ramp_proxy.h"
#include "integer_display.h"
#include "integer_display_proxy.h"
#include "port.h"
#include "channel.h"
#include <mftp.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

static input_message_t ramp_inputs[] = {
  {
    NULL, NULL, NULL
  }
};

static output_message_t ramp_outputs[] = {
  {
    "display_integer", "integer", ramp_proxy_integer_out
  },
  {
    NULL, NULL, NULL
  }
};

static port_type_descriptor_t ramp_port_descriptors[] = {
  {
    .cardinality = 0,
    .input_messages = ramp_inputs,
    .output_messages = ramp_outputs,
  },
  {
    0, NULL, NULL
  },
};

static input_message_t integer_display_inputs[] = {
  {
    "display_integer", "integer", integer_display_proxy_integer_in
  },
  {
    NULL, NULL, NULL
  }
};

static output_message_t integer_display_outputs[] = {
  {
    NULL, NULL, NULL
  }
};

static port_type_descriptor_t integer_display_port_descriptors[] = {
  {
    .cardinality = 1,
    .input_messages = integer_display_inputs,
    .output_messages = integer_display_outputs,
  },
  {
    0, NULL, NULL
  },
};

static uint32_t ramp_to_display[] = {
  0
};

static uint32_t display_to_ramp[] = {
};

typedef struct {
  manager_t* manager;

  aid_t self;

  aid_t msg_sender;
  aid_t msg_receiver;

  uuid_t ramp_component;
  uint32_t ramp_port_type;
  component_manager_create_arg_t ramp_manager_arg;
  aid_t ramp_manager;

  uuid_t display_component;
  uint32_t display_port_type;
  component_manager_create_arg_t display_manager_arg;
  aid_t display_manager;

  channel_create_arg_t channel_arg;
  aid_t channel;
} composer_t;

static void*
composer_create (void* arg)
{
  composer_t* composer = malloc (sizeof (composer_t));

  composer->manager = manager_create ();

  manager_self_set (composer->manager, &composer->self);

  manager_child_add (composer->manager, &composer->msg_sender, &msg_sender_descriptor, NULL);
  manager_child_add (composer->manager, &composer->msg_receiver, &msg_receiver_descriptor, NULL);

  uuid_generate (composer->ramp_component);
  component_manager_create_arg_init (&composer->ramp_manager_arg,
				     &ramp_descriptor,
				     NULL,
				     ramp_request_proxy,
				     composer->ramp_component,
				     ramp_port_descriptors,
				     &composer->msg_sender,
				     &composer->msg_receiver);
  manager_child_add (composer->manager,
  		     &composer->ramp_manager,
  		     &component_manager_descriptor,
  		     &composer->ramp_manager_arg);

  uuid_generate (composer->display_component);
  component_manager_create_arg_init (&composer->display_manager_arg,
				     &integer_display_descriptor,
				     NULL,
				     integer_display_request_proxy,
				     composer->display_component,
				     integer_display_port_descriptors,
				     &composer->msg_sender,
				     &composer->msg_receiver);
  manager_child_add (composer->manager,
  		     &composer->display_manager,
  		     &component_manager_descriptor,
  		     &composer->display_manager_arg);

  composer->ramp_port_type = 0;
  composer->display_port_type = 0;

  channel_create_arg_init (&composer->channel_arg,
			   &composer->ramp_manager,
			   composer->ramp_component,
			   composer->ramp_port_type,
			   &composer->display_manager,
			   composer->display_component,
			   composer->display_port_type,
			   1,
			   ramp_to_display,
			   0,
			   display_to_ramp);
  manager_child_add (composer->manager,
		     &composer->channel,
		     &channel_descriptor,
		     &composer->channel_arg);

  return composer;
}

static void
composer_system_input (void* state, void* param, bid_t bid)
{
  composer_t* composer = state;
  assert (composer != NULL);

  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));
  const receipt_t* receipt = buffer_read_ptr (bid);

  manager_apply (composer->manager, receipt);
  if (composer->ramp_manager != -1) {
    assert (schedule_free_input (composer->ramp_manager, component_manager_strobe, buffer_alloc (0)) == 0);
  }
  if (composer->display_manager != -1) {
    assert (schedule_free_input (composer->display_manager, component_manager_strobe, buffer_alloc (0)) == 0);
  }
  if (composer->channel != -1) {
    assert (schedule_free_input (composer->channel, channel_strobe, buffer_alloc (0)) == 0);
  }
}

static bid_t
composer_system_output (void* state, void* param)
{
  composer_t* composer = state;
  assert (composer != NULL);

  return manager_action (composer->manager);
}

descriptor_t composer_descriptor = {
  .constructor = composer_create,
  .system_input = composer_system_input,
  .system_output = composer_system_output,
};

int
main (int argc, char** argv)
{
  ueioa_run (&composer_descriptor, NULL, 1);
  exit (EXIT_SUCCESS);
}
