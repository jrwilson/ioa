#include <stdlib.h>
#include <assert.h>
#include <automan.h>

static const descriptor_t proxy_descriptor = {
};

static void proxy_generator_proxy_created (void* state, void* param, receipt_type_t receipt);

typedef struct {
  aid_t aid;
  proxy_request_t proxy_request;
} pg_proxy_t;

typedef struct {
  aid_t self;
  automan_t* automan;
} proxy_generator_t;

static void*
proxy_generator_create (const void* arg)
{
  proxy_generator_t* proxy_generator = malloc (sizeof (proxy_generator_t));

  proxy_generator->automan = automan_creat (proxy_generator, &proxy_generator->self);

  return proxy_generator;
}

static void
proxy_generator_system_input (void* state, void* param, bid_t bid)
{
  proxy_generator_t* proxy_generator = state;
  assert (proxy_generator != NULL);

  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));
  const receipt_t* receipt = buffer_read_ptr (bid);

  automan_apply (proxy_generator->automan, receipt);
}

static bid_t
proxy_generator_system_output (void* state, void* param)
{
  proxy_generator_t* proxy_generator = state;
  assert (proxy_generator != NULL);

  return automan_action (proxy_generator->automan);
}

static void
proxy_generator_request_proxy (void* state, void* param, bid_t bid)
{
  proxy_generator_t* proxy_generator = state;
  assert (proxy_generator != NULL);

  assert (buffer_size (bid) == sizeof (proxy_request_t));
  const proxy_request_t* proxy_request = buffer_read_ptr (bid);

  pg_proxy_t* pg_proxy = malloc (sizeof (pg_proxy_t));

  pg_proxy->proxy_request = *proxy_request;

  assert (automan_create (proxy_generator->automan,
			  &pg_proxy->aid,
			  &proxy_descriptor,
			  NULL,
			  proxy_generator_proxy_created,
			  pg_proxy) == 0);
}

static void
proxy_generator_proxy_created (void* state, void* param, receipt_type_t receipt)
{
  proxy_generator_t* proxy_generator = state;
  assert (proxy_generator != NULL);
  assert (receipt == CHILD_CREATED);

  pg_proxy_t* pg_proxy = param;
  assert (pg_proxy != NULL);

  assert (automan_proxy_send_created (pg_proxy->aid, -1, &pg_proxy->proxy_request) == 0);
}

static const input_t proxy_generator_free_inputs[] = {
  proxy_generator_request_proxy,
  NULL,
};

static const descriptor_t proxy_generator_descriptor = {
  .constructor = proxy_generator_create,
  .system_input = proxy_generator_system_input,
  .system_output = proxy_generator_system_output,
  .free_inputs = proxy_generator_free_inputs,
};

static void automaton_proxy_generator_created (void* state, void* param, receipt_type_t receipt);
static void automaton_proxy_created (void* state, void* param, proxy_receipt_type_t receipt, bid_t bid);
static void automaton_receive_proxy (void* state, void* param, bid_t bid);

typedef struct {
  aid_t self;
  automan_t* automan;
  aid_t proxy_generator;
  aid_t proxy;
} automaton_t;

static void*
automaton_create (const void* arg)
{
  automaton_t* automaton = malloc (sizeof (automaton_t));

  automaton->automan = automan_creat (automaton, &automaton->self);
  assert (automan_create (automaton->automan,
			  &automaton->proxy_generator,
			  &proxy_generator_descriptor,
			  NULL,
			  automaton_proxy_generator_created,
			  NULL) == 0);
  return automaton;
}

static void
automaton_system_input (void* state, void* param, bid_t bid)
{
  automaton_t* automaton = state;
  assert (automaton != NULL);

  assert (bid != -1);
  assert (buffer_size (bid) == sizeof (receipt_t));
  const receipt_t* receipt = buffer_read_ptr (bid);

  automan_apply (automaton->automan, receipt);
}

static bid_t
automaton_system_output (void* state, void* param)
{
  automaton_t* automaton = state;
  assert (automaton != NULL);

  return automan_action (automaton->automan);
}

static void
automaton_proxy_generator_created (void* state, void* param, receipt_type_t receipt)
{
  automaton_t* automaton = state;
  assert (automaton != NULL);
  assert (receipt == CHILD_CREATED);
  assert (automaton->proxy_generator != -1);
  
  assert (automan_proxy_add (automaton->automan,
  			     &automaton->proxy,
  			     automaton->proxy_generator,
  			     proxy_generator_request_proxy,
  			     -1,
  			     automaton_receive_proxy,
  			     automaton_proxy_created,
  			     NULL) == 0);
}

static void
automaton_receive_proxy (void* state, void* param, bid_t bid)
{
  automaton_t* automaton = state;
  assert (automaton != NULL);

  automan_proxy_receive (automaton->automan,
			 bid);
}

static void
automaton_proxy_created (void* state, void* param, proxy_receipt_type_t receipt, bid_t bid)
{
  automaton_t* automaton = state;
  assert (automaton != NULL);
  assert (receipt == PROXY_CREATED);
  assert (automaton->proxy != -1);

  exit (EXIT_SUCCESS);
}

static const input_t automaton_free_inputs[] = {
  automaton_receive_proxy,
  NULL
};

static const descriptor_t automaton_descriptor = {
  .constructor = automaton_create,
  .system_input = automaton_system_input,
  .system_output = automaton_system_output,
  .free_inputs = automaton_free_inputs,
};

int
main (int argc, char** argv)
{
  ueioa_run (&automaton_descriptor, NULL, 1);
  exit (EXIT_SUCCESS);
}
