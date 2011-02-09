#include "ueioa.h"

#include <assert.h>

void
order_create_init (order_t* order, descriptor_t* descriptor)
{
  assert (order != NULL);

  order->type = CREATE;
  order->create.descriptor = descriptor;
}

void
order_compose_init (order_t* order, aid_t out_automaton, output_t output, aid_t in_automaton, input_t input)
{
  assert (order != NULL);

  order->type = COMPOSE;
  order->compose.out_automaton = out_automaton;
  order->compose.output = output;
  order->compose.in_automaton = in_automaton;
  order->compose.input = input;
}

void
system_order_decompose (order_t* order, aid_t out_automaton, output_t output, aid_t in_automaton, input_t input)
{
  assert (order != NULL);

  order->type = DECOMPOSE;
  order->decompose.out_automaton = out_automaton;
  order->decompose.output = output;
  order->decompose.in_automaton = in_automaton;
  order->decompose.input = input;
}

void
system_order_destroy (order_t* order, aid_t automaton)
{
  assert (order != NULL);

  order->type = DESTROY;
  order->destroy.automaton = automaton;
}

void
system_receipt_decomposed (receipt_t* receipt)
{
  assert (receipt != NULL);

  receipt->type = DECOMPOSED;
}

void
system_receipt_output_decomposed (receipt_t* receipt, output_t output)
{
  assert (receipt != NULL);

  receipt->type = OUTPUT_DECOMPOSED;
  receipt->output_decomposed.output = output;
}

void
system_receipt_not_composed (receipt_t* receipt)
{
  assert (receipt != NULL);

  receipt->type = NOT_COMPOSED;
}

void
system_receipt_automaton_dne (receipt_t* receipt)
{
  assert (receipt != NULL);

  receipt->type = AUTOMATON_DNE;
}

void
system_receipt_not_owner (receipt_t* receipt)
{
  assert (receipt != NULL);

  receipt->type = NOT_OWNER;
}

void
system_receipt_child_destroyed (receipt_t* receipt, aid_t child)
{
  assert (receipt != NULL);

  receipt->type = CHILD_DESTROYED;
  receipt->child_destroyed.child = child;
}
