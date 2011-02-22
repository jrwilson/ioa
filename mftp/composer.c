#include "composer.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "mftp.h"
#include "consumer.h"

static char message[] = "Hello, World!!";

typedef struct {
  manager_t* manager;
  aid_t self;

  aid_t msg_sender;
  aid_t msg_receiver;

  aid_t file_server_proxy;
  file_server_create_arg_t file_server_arg;
  aid_t file_server;

  aid_t file_client_proxy;
  file_server_create_arg_t file_client_arg;
  aid_t file_client;

  aid_t consumer;
} composer_t;

static void composer_callback (void* state, void* param, bid_t bid);

static void*
composer_create (void* arg)
{
  composer_t* composer = malloc (sizeof (composer_t));

  composer->manager = manager_create ();

  manager_self_set (composer->manager, &composer->self);

  manager_child_add (composer->manager, &composer->msg_sender, &msg_sender_descriptor, NULL);
  manager_child_add (composer->manager, &composer->msg_receiver, &msg_receiver_descriptor, NULL);

  manager_proxy_add (composer->manager, &composer->file_server_proxy, &composer->msg_sender, msg_sender_request_proxy, composer_callback);
  composer->file_server_arg.file = mftp_File_create_buffer (message, strlen (message) + 1, 37);
  composer->file_server_arg.download = false;
  manager_child_add (composer->manager, &composer->file_server, &file_server_descriptor, &composer->file_server_arg);
  manager_composition_add (composer->manager, &composer->msg_receiver, msg_receiver_announcement_out, NULL, &composer->file_server, file_server_announcement_in, NULL);
  manager_composition_add (composer->manager, &composer->msg_receiver, msg_receiver_request_out, NULL, &composer->file_server, file_server_request_in, NULL);
  manager_composition_add (composer->manager, &composer->msg_receiver, msg_receiver_fragment_out, NULL, &composer->file_server, file_server_fragment_in, NULL);
  manager_composition_add (composer->manager, &composer->file_server, file_server_message_out, NULL, &composer->file_server_proxy, msg_sender_proxy_message_in, NULL);

  manager_proxy_add (composer->manager, &composer->file_client_proxy, &composer->msg_sender, msg_sender_request_proxy, composer_callback);
  composer->file_client_arg.file = mftp_File_create_empty (&composer->file_server_arg.file->fileid);
  composer->file_client_arg.download = true;
  manager_child_add (composer->manager, &composer->file_client, &file_server_descriptor, &composer->file_client_arg);
  manager_composition_add (composer->manager, &composer->msg_receiver, msg_receiver_announcement_out, NULL, &composer->file_client, file_server_announcement_in, NULL);
  manager_composition_add (composer->manager, &composer->msg_receiver, msg_receiver_request_out, NULL, &composer->file_client, file_server_request_in, NULL);
  manager_composition_add (composer->manager, &composer->msg_receiver, msg_receiver_fragment_out, NULL, &composer->file_client, file_server_fragment_in, NULL);
  manager_composition_add (composer->manager, &composer->file_client, file_server_message_out, NULL, &composer->file_client_proxy, msg_sender_proxy_message_in, NULL);

  manager_child_add (composer->manager, &composer->consumer, &consumer_descriptor, NULL);
  manager_composition_add (composer->manager, &composer->msg_receiver, msg_receiver_announcement_out, NULL, &composer->consumer, consumer_announcement_in, NULL);
  manager_composition_add (composer->manager, &composer->msg_receiver, msg_receiver_match_out, NULL, &composer->consumer, consumer_match_in, NULL);
  manager_composition_add (composer->manager, &composer->msg_receiver, msg_receiver_request_out, NULL, &composer->consumer, consumer_request_in, NULL);
  manager_composition_add (composer->manager, &composer->msg_receiver, msg_receiver_fragment_out, NULL, &composer->consumer, consumer_fragment_in, NULL);

  return composer;
}

static void
composer_system_input (void* state, void* param, bid_t bid)
{
  assert (state != NULL);
  composer_t* composer = state;

  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));
  const receipt_t* receipt = buffer_read_ptr (bid);

  manager_apply (composer->manager, receipt);
}

static bid_t
composer_system_output (void* state, void* param)
{
  composer_t* composer = state;
  assert (composer != NULL);

  return manager_action (composer->manager);
}

static void
composer_callback (void* state, void* param, bid_t bid)
{
  composer_t* composer = state;
  assert (composer != NULL);

  assert (buffer_size (bid) == sizeof (proxy_receipt_t));
  const proxy_receipt_t* receipt = buffer_read_ptr (bid);
  manager_proxy_receive (composer->manager, receipt);
}

static input_t composer_free_inputs[] = { composer_callback, NULL };

descriptor_t composer_descriptor = {
  .constructor = composer_create,
  .system_input = composer_system_input,
  .system_output = composer_system_output,
  .free_inputs = composer_free_inputs,
};
