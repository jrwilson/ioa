#include <stdlib.h>
#include <assert.h>
#include <automan.h>
#include <ccm.h>
#include <mftp.h>
#include "integer_reflector.h"

#define UDP_PORT 64470

#define PORT 0
#define OUT_MESSAGE 0
#define IN_MESSAGE 0

static void
composer_callback1 (void*, 
		    void*,
		    bid_t);

static void
composer_callback2 (void*, 
		    void*,
		    bid_t);

static bid_t
composer_out (void*,
	      void*);

static void
composer_in (void*, 
	     void*,
	     bid_t);

typedef struct composer_struct {
  aid_t self;
  automan_t* automan;

  msg_sender_create_arg_t msg_sender_arg;
  aid_t msg_sender;
  msg_receiver_create_arg_t msg_receiver_arg;
  aid_t msg_receiver;

  uuid_t id;
  component_create_arg_t component_arg;
  aid_t component;

  aid_t instance_aid;
  bool out_composed;
  bool in_composed;
  uint32_t instance;
} composer_t;

static void
composer_composed (void* state,
		   void* param,
		   receipt_type_t receipt)
{
  composer_t* composer = state;
  assert (composer != NULL);

  if (receipt == COMPOSED) {
    if (composer->out_composed &&
	composer->in_composed) {
      assert (schedule_output (composer_out, NULL) == 0);
    }
  }
  else if (receipt == DECOMPOSED) {
    /* Okay. */
  }
  else {
    assert (0);
  }
}

static void
composer_proxy_created2 (void* state,
			 void* param,
			 proxy_receipt_type_t receipt,
			 bid_t bid)
{
  composer_t* composer = state;
  assert (composer != NULL);

  const instance_receipt_t* instance_receipt = buffer_read_ptr (bid);
  if (instance_receipt->status == INSTANCE_REQUEST_OKAY) {
    exit (EXIT_SUCCESS);
  }
  else {
    exit (EXIT_FAILURE);
  }
}

static void
composer_proxy_created1 (void* state,
			 void* param,
			 proxy_receipt_type_t receipt,
			 bid_t bid)
{
  composer_t* composer = state;
  assert (composer != NULL);

  if (receipt == PROXY_CREATED) {
    const instance_receipt_t* instance_receipt = buffer_read_ptr (bid);
    assert (instance_receipt->status == INSTANCE_REQUEST_OKAY);
    composer->instance = instance_receipt->instance;
  }
  else if (receipt == PROXY_DESTROYED) {
    /* Proxy was destroyed.  Time to go again. */
    assert (automan_proxy_add (composer->automan,
			       &composer->instance_aid,
			       composer->component,
			       component_request_instance,
			       instance_request_create (PORT),
			       composer_callback2,
			       composer_proxy_created2,
			       NULL) == 0);
  }
  else {
    assert (0);
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
			       &composer->instance_aid,
			       composer->component,
			       component_request_instance,
			       instance_request_create (PORT),
			       composer_callback1,
			       composer_proxy_created1,
			       NULL) == 0);
    assert (automan_compose (composer->automan,
			     &composer->out_composed,
			     &composer->self,
			     composer_out,
			     NULL,
			     &composer->instance_aid,
			     instance_in,
			     NULL,
			     composer_composed,
			     NULL) == 0);
    assert (automan_compose (composer->automan,
			     &composer->in_composed,
			     &composer->instance_aid,
			     instance_out,
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
			       integer_reflector_port_descriptors,
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

  composer->msg_sender_arg.port = UDP_PORT;
  assert (automan_create (composer->automan,
			  &composer->msg_sender,
			  &msg_sender_descriptor,
			  &composer->msg_sender_arg,
			  composer_sender_receiver_created,
			  NULL) == 0);

  composer->msg_receiver_arg.port = UDP_PORT;
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
composer_callback1 (void* state, 
		    void* param,
		    bid_t bid)
{
  composer_t* composer = state;
  assert (composer != NULL);

  automan_proxy_receive (composer->automan, bid);
}

static void
composer_callback2 (void* state, 
		    void* param,
		    bid_t bid)
{
  composer_t* composer = state;
  assert (composer != NULL);

  automan_proxy_receive (composer->automan, bid);
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
  m->dst_port = PORT;
  m->dst_instance = composer->instance;
  m->dst_message = OUT_MESSAGE;

  m->bid = ibid;
  buffer_add_child (mbid, ibid);

  return mbid;
}

static void
composer_in (void* state, 
	     void* param,
	     bid_t bid)
{
  composer_t* composer = state;
  assert (composer != NULL);
  assert (buffer_size (bid) == sizeof (message_t));

  buffer_incref (bid);

  const message_t* m = buffer_read_ptr (bid);
  assert (buffer_size (m->bid) == sizeof (int));
  const int* x = buffer_read_ptr (m->bid);
  buffer_decref (bid);

  assert (*x == -37);

  assert (automan_decompose (composer->automan,
			     &composer->out_composed) == 0);
  assert (automan_decompose (composer->automan,
			     &composer->in_composed) == 0);
}

static input_t
composer_free_inputs[] = {
  composer_callback1,
  composer_callback2,
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
