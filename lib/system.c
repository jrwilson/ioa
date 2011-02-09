#include "ueioa.h"

#include <assert.h>

void
system_order_create (system_order_t* order, automaton_descriptor_t* descriptor)
{
  assert (order != NULL);

  order->type = SYS_CREATE;
  order->create.descriptor = descriptor;
}

void
system_order_compose (system_order_t* order, aid_t out_automaton, output_t output, aid_t in_automaton, input_t input)
{
  assert (order != NULL);

  order->type = SYS_COMPOSE;
  order->compose.out_automaton = out_automaton;
  order->compose.output = output;
  order->compose.in_automaton = in_automaton;
  order->compose.input = input;
}

void
system_order_decompose (system_order_t* order, aid_t out_automaton, output_t output, aid_t in_automaton, input_t input)
{
  assert (order != NULL);

  order->type = SYS_DECOMPOSE;
  order->decompose.out_automaton = out_automaton;
  order->decompose.output = output;
  order->decompose.in_automaton = in_automaton;
  order->decompose.input = input;
}

void
system_order_destroy (system_order_t* order, aid_t automaton)
{
  assert (order != NULL);

  order->type = SYS_DESTROY;
  order->destroy.automaton = automaton;
}

void
system_receipt_decomposed (system_receipt_t* receipt)
{
  assert (receipt != NULL);

  receipt->type = SYS_DECOMPOSED;
}

void
system_receipt_output_decomposed (system_receipt_t* receipt, output_t output)
{
  assert (receipt != NULL);

  receipt->type = SYS_OUTPUT_DECOMPOSED;
  receipt->output_decomposed.output = output;
}

void
system_receipt_not_composed (system_receipt_t* receipt)
{
  assert (receipt != NULL);

  receipt->type = SYS_NOT_COMPOSED;
}

void
system_receipt_automaton_dne (system_receipt_t* receipt)
{
  assert (receipt != NULL);

  receipt->type = SYS_AUTOMATON_DNE;
}

void
system_receipt_not_owner (system_receipt_t* receipt)
{
  assert (receipt != NULL);

  receipt->type = SYS_NOT_OWNER;
}

void
system_receipt_child_destroyed (system_receipt_t* receipt, aid_t child)
{
  assert (receipt != NULL);

  receipt->type = SYS_CHILD_DESTROYED;
  receipt->child_destroyed.child = child;
}
