#ifndef __component_h__
#define __component_h__

#include <json/json.h>

#include <automan.h>
#include <mftp.h>
#include <ccm.h>
#include "instance.h"
#include "instance_allocator.h"

#define COMPONENT_DESCRIPTION 0
#define COMPONENT_INSTANCE_SUMMARY 1

typedef struct instance_struct instance_t;
struct instance_struct {
  proxy_request_t request;
  uint32_t port;
  uint32_t instance;
  bool declared;
  instance_create_arg_t create_arg;
  aid_t aid;
  aid_t aid2;
  instance_t* next;
};

typedef struct {
  aid_t self;
  automan_t* automan;

  instance_allocator_t* instance_allocator;

  aid_t automaton;
  input_t request_proxy;
  const port_descriptor_t* port_descriptors;
  uuid_t component_id;
  instance_t* instances;
  bidq_t* instance_requestq;

  file_server_create_arg_t description_arg;
  aid_t description;

  file_server_create_arg_t instance_summary_arg;
  aid_t instance_summary;
} component_t;

json_object* 
encode_description (component_t* component);

void
build_instance_summary_server (component_t* component);
void
rebuild_instance_summary_server (component_t* component);

#endif
