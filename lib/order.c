#include <ueioa.h>

#include <assert.h>

void
order_create_init (order_t* order, descriptor_t* descriptor)
{
  assert (order != NULL);

  order->type = CREATE;
  order->create.descriptor = descriptor;
}

void
order_declare_init (order_t* order, void* param)
{
  assert (order != NULL);
  order->type = DECLARE;
  order->declare.param = param;
}

void
order_compose_init (order_t* order, aid_t out_aid, output_t output, void* out_param, aid_t in_aid, input_t input, void* in_param)
{
  assert (order != NULL);

  order->type = COMPOSE;
  order->compose.out_aid = out_aid;
  order->compose.output = output;
  order->compose.out_param = out_param;
  order->compose.in_aid = in_aid;
  order->compose.input = input;
  order->compose.in_param = in_param;
}

void
order_decompose_init (order_t* order, aid_t out_aid, output_t output, void* out_param, aid_t in_aid, input_t input, void* in_param)
{
  assert (order != NULL);

  order->type = DECOMPOSE;
  order->decompose.out_aid = out_aid;
  order->decompose.output = output;
  order->decompose.out_param = out_param;
  order->decompose.in_aid = in_aid;
  order->decompose.input = input;
  order->decompose.in_param = in_param;
}

void
order_rescind_init (order_t* order, void* param)
{
  assert (order != NULL);
  order->type = RESCIND;
  order->rescind.param = param;
}

void
order_destroy_init (order_t* order, aid_t aid)
{
  assert (order != NULL);

  order->type = DESTROY;
  order->destroy.aid = aid;
}

void
order_set_alarm_init (order_t* order, time_t secs, long usecs)
{
  assert (order != NULL);

  order->type = SET_ALARM;
  order->timer.secs = secs;
  order->timer.usecs = usecs;
}
