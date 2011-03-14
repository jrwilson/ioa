#include <stdlib.h>
#include <assert.h>
#include <automan.h>
#include <ccm.h>
#include <mftp.h>
#include "integer_reflector.h"

#define PORT 64470

static void
composer_callback (void*, 
		   void*,
		   bid_t);

static bid_t
composer_out (void*,
	      void*);

static void
composer_in (void*, 
	     void*,
	     bid_t);

typedef struct {
  aid_t self;
  automan_t* automan;

  msg_sender_create_arg_t msg_sender_arg;
  aid_t msg_sender;
  msg_receiver_create_arg_t msg_receiver_arg;
  aid_t msg_receiver;

  uuid_t id;
  component_create_arg_t component_arg;
  aid_t component;

  aid_t port;
  bool out_composed;
  bool in_composed;
} composer_t;

static void
composer_composed (void* state,
		   void* param,
		   receipt_type_t receipt)
{
  composer_t* composer = state;
  assert (composer != NULL);
  assert (receipt == COMPOSED);

  if (composer->out_composed &&
      composer->in_composed) {
    assert (schedule_output (composer_out, NULL) == 0);
  }
}

static void
composer_component_created (void* state,
			    void* param,
			    receipt_type_t receipt)
{
  composer_t* composer = state;
  assert (composer != NULL);
  assert (receipt == CHILD_CREATED);

  if (composer->component != -1) {
    assert (automan_proxy_add (composer->automan,
			       &composer->port,
			       composer->component,
			       component_request_port,
			       port_request_create (0),
			       composer_callback,
			       NULL,
			       NULL) == 0);
    assert (automan_compose (composer->automan,
			     &composer->out_composed,
			     &composer->self,
			     composer_out,
			     NULL,
			     &composer->port,
			     port_in,
			     NULL,
			     composer_composed,
			     NULL) == 0);
    assert (automan_compose (composer->automan,
			     &composer->in_composed,
			     &composer->port,
			     port_out,
			     NULL,
			     &composer->self,
			     composer_in,
			     NULL,
			     composer_composed,
			     NULL) == 0);

  }
}

static void
composer_sender_receiver_created (void* state,
				  void* param,
				  receipt_type_t receipt)
{
  composer_t* composer = state;
  assert (composer != NULL);
  assert (receipt == CHILD_CREATED);

  if (composer->msg_sender != -1 &&
      composer->msg_receiver != -1) {
    uuid_generate (composer->id);
    component_create_arg_init (&composer->component_arg,
			       &integer_reflector_descriptor,
			       NULL,
			       integer_reflector_request_proxy,
			       composer->id,
			       integer_reflector_port_type_descriptors,
			       composer->msg_sender,
			       composer->msg_receiver);
    
    assert (automan_create (composer->automan,
			    &composer->component,
			    &component_descriptor,
			    &composer->component_arg,
			    composer_component_created,
			    NULL) == 0);
  }
}

static void*
composer_create (const void* arg)
{
  composer_t* composer = malloc (sizeof (composer_t));

  composer->automan = automan_creat (composer, &composer->self);

  composer->msg_sender_arg.port = PORT;
  assert (automan_create (composer->automan,
			  &composer->msg_sender,
			  &msg_sender_descriptor,
			  &composer->msg_sender_arg,
			  composer_sender_receiver_created,
			  NULL) == 0);

  composer->msg_receiver_arg.port = PORT;
  assert (automan_create (composer->automan,
			  &composer->msg_receiver,
			  &msg_receiver_descriptor,
			  &composer->msg_receiver_arg,
			  composer_sender_receiver_created,
			  NULL) == 0);

  return composer;
}

static void
composer_system_input (void* state,
		       void* param,
		       bid_t bid)
{
  composer_t* composer = state;
  assert (composer != NULL);

  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));
  const receipt_t* receipt = buffer_read_ptr (bid);

  automan_apply (composer->automan, receipt);
}

static bid_t
composer_system_output (void* state,
			void* param)
{
  composer_t* composer = state;
  assert (composer != NULL);

  return automan_action (composer->automan);
}

static void
composer_callback (void* state, 
		   void* param,
		   bid_t bid)
{
  composer_t* composer = state;
  assert (composer != NULL);

  assert (buffer_size (bid) == sizeof (proxy_receipt_t));
  const proxy_receipt_t* proxy_receipt = buffer_read_ptr (bid);
  automan_proxy_receive (composer->automan, proxy_receipt);
}

static bid_t
composer_out (void* state,
	      void* param)
{
  composer_t* composer = state;
  assert (composer != NULL);
  assert (composer->out_composed);
  assert (composer->in_composed);

  bid_t ibid = buffer_alloc (sizeof (int));
  int* i = buffer_write_ptr (ibid);
  *i = -37;

  bid_t mbid = buffer_alloc (sizeof (message_t));
  message_t* m = buffer_write_ptr (mbid);

  uuid_copy (m->dst_component, composer->id);
  m->dst_port_type = 0;
  m->dst_port = 0;
  m->dst_message = 0;

  m->bid = ibid;
  buffer_add_child (mbid, ibid);

  return mbid;
}

static void
composer_in (void* state, 
	     void* param,
	     bid_t bid)
{
  assert (buffer_size (bid) == sizeof (message_t));

  buffer_incref (bid);

  const message_t* m = buffer_read_ptr (bid);
  assert (buffer_size (m->bid) == sizeof (int));
  const int* x = buffer_read_ptr (m->bid);
  buffer_decref (bid);

  if (*x == -37) {
    exit (EXIT_SUCCESS);
  }
  else {
    exit (EXIT_FAILURE);
  }
}

static input_t
composer_free_inputs[] = {
  composer_callback,
  NULL
};

static input_t
composer_inputs[] = {
  composer_in,
  NULL,
};

static output_t
composer_outputs[] = {
  composer_out,
  NULL,
};

static const descriptor_t composer_descriptor = {
  .constructor = composer_create,
  .system_input = composer_system_input,
  .system_output = composer_system_output,
  .free_inputs = composer_free_inputs,
  .inputs = composer_inputs,
  .outputs = composer_outputs,
};

int
main (int argc, char** argv)
{
  ueioa_run (&composer_descriptor, NULL, 1);
  exit (EXIT_SUCCESS);
}
