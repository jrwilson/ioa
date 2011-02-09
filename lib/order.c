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
order_compose_init (order_t* order, aid_t out_aid, output_t output, aid_t in_aid, input_t input)
{
  assert (order != NULL);

  order->type = COMPOSE;
  order->compose.out_aid = out_aid;
  order->compose.output = output;
  order->compose.in_aid = in_aid;
  order->compose.input = input;
}

void
order_decompose_init (order_t* order, aid_t out_aid, output_t output, aid_t in_aid, input_t input)
{
  assert (order != NULL);

  order->type = DECOMPOSE;
  order->decompose.out_aid = out_aid;
  order->decompose.output = output;
  order->decompose.in_aid = in_aid;
  order->decompose.input = input;
}

void
order_destroy_init (order_t* order, aid_t aid)
{
  assert (order != NULL);

  order->type = DESTROY;
  order->destroy.aid = aid;
}
