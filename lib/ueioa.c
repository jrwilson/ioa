#include "ueioa.h"

#include <assert.h>
#include <pthread.h>

#include <stdio.h>

#include "runq.h"
#include "automata.h"
#include "receipts.h"

static runq_t* runq;
static automata_t* automata;
static buffers_t* buffers;
static receipts_t* receipts;

/* static void */
/* decomposed (aid_t aid) */
/* { */
/*   receipts_push_decomposed (receipts, aid); */
/*   runq_insert_system_input (runq, aid); */
/* } */

/* static void */
/* output_decomposed (aid_t out_aid, output_t output) */
/* { */
/*   receipts_push_output_decomposed (receipts, out_aid, output); */
/*   runq_insert_system_input (runq, out_aid); */
/* } */

/* static void */
/* not_composed (aid_t aid) */
/* { */
/*   receipts_push_not_composed (receipts, aid); */
/*   runq_insert_system_input (runq, aid); */
/* } */

/* static void */
/* automaton_dne (aid_t aid) */
/* { */
/*   receipts_push_automaton_dne (receipts, aid); */
/*   runq_insert_system_input (runq, aid); */
/* } */

/* static void */
/* not_owner (aid_t aid) */
/* { */
/*   receipts_push_not_owner (receipts, aid); */
/*   runq_insert_system_input (runq, aid); */
/* } */

/* static void */
/* child_destroyed (aid_t parent, aid_t child) */
/* { */
/*   receipts_push_child_destroyed (receipts, parent, child); */
/*   runq_insert_system_input (runq, parent); */
/* } */

static void
create (aid_t parent, descriptor_t* descriptor)
{
  /* Parent can be -1, i.e., for the first automaton. */
  if (descriptor != NULL && descriptor_check (descriptor)) {
    aid_t aid = automata_create_automaton (automata, descriptor, parent);
    /* Tell the automaton that it has been created. */
    receipts_push_self_created (receipts, aid, aid, parent);
    runq_insert_system_input (runq, aid);

    if (parent != -1) {
      /* Tell the parent that the automaton has been created. */
      receipts_push_child_created (receipts, parent, aid);
      runq_insert_system_input (runq, parent);
    }
  }
  else {
    /* Bad descriptor. */
    if (parent != -1) {
      receipts_push_bad_descriptor (receipts, parent);
      runq_insert_system_input (runq, parent);
    }
  }
}

static void
compose (aid_t aid, aid_t out_aid, output_t output, aid_t in_aid, input_t input)
{
  switch (automata_compose (automata, aid, out_aid, output, in_aid, input)) {
  case A_OUTPUT_DNE:
    receipts_push_output_dne (receipts, aid);
    runq_insert_system_input (runq, aid);
    break;
  case A_INPUT_DNE:
    receipts_push_input_dne (receipts, aid);
    runq_insert_system_input (runq, aid);
    break;
  case A_INPUT_UNAVAILABLE:
    receipts_push_input_unavailable (receipts, aid);
    runq_insert_system_input (runq, aid);
    break;
  case A_OUTPUT_UNAVAILABLE:
    receipts_push_output_unavailable (receipts, aid);
    runq_insert_system_input (runq, aid);
    break;
  case A_COMPOSED:
    receipts_push_composed (receipts, aid);
    runq_insert_system_input (runq, aid);

    receipts_push_output_composed (receipts, out_aid, output);
    runq_insert_system_input (runq, out_aid);

    break;
  default:
    /* Unhandled case. */
    assert (0);
  }
}

static void
decompose (aid_t aid, aid_t out_aid, output_t output, aid_t in_aid, input_t input)
{
  switch (automata_decompose (automata, aid, out_aid, output, in_aid, input)) {
  case A_NOT_COMPOSER:
    receipts_push_not_composer (receipts, aid);
    runq_insert_system_input (runq, aid);
    break;
  case A_NOT_COMPOSED:
    receipts_push_not_composed (receipts, aid);
    runq_insert_system_input (runq, aid);
    break;
  case A_DECOMPOSED:
    receipts_push_decomposed (receipts, aid);
    runq_insert_system_input (runq, aid);

    receipts_push_output_decomposed (receipts, out_aid, output);
    runq_insert_system_input (runq, out_aid);

    break;
  }
}

static void
destroy (aid_t destroyer, aid_t target)
{
  assert (0);
  /* if (automaton_exists (target)) { */
  /*   if (destroyer == target || destroyer == automaton_parent (target)) { */
  /*     /\* Recur on children. *\/ */
  /*     while (!automaton_children_empty (target)) { */
  /* 	aid_t child = automaton_children_front (target); */
  /* 	automaton_children_remove (target, child); */
  /* 	destroy (target, child); */
  /*     } */

  /*     /\* Purge the runq. *\/ */
  /*     runq_purge (runq, target); */

  /*     size_t idx; */

  /*     /\* Decompose inputs. *\/ */
  /*     for (idx = automaton_inputs_first (target); */
  /* 	   idx != automaton_inputs_last (target); */
  /* 	   idx = automaton_inputs_next (target,idx)) { */
  /* 	input_t input = automaton_inputs_input (target, idx); */
  /* 	if (automaton_input_composed (target, input)) { */
  /* 	  aid_t out_automaton; */
  /* 	  output_t output; */
  /* 	  aid_t automaton; */
  /* 	  automaton_input_output (target, input, &out_automaton, &output, &automaton); */
  /* 	  /\* Decompose. *\/ */
  /* 	  automaton_composition_decompose (automaton, out_automaton, output, target, input); */
  /* 	  automaton_output_decompose (out_automaton, output, target); */
  /* 	  /\* Tell the composer and output. *\/ */
  /* 	  decomposed (automaton); */
  /* 	  output_decomposed (out_automaton, output); */
  /* 	} */
  /*     } */

  /*     /\* Decompose outputs. *\/ */
  /*     for (idx = automaton_outputs_first (target); */
  /* 	   idx != automaton_outputs_last (target); */
  /* 	   idx = automaton_outputs_next (target, idx)) { */
  /* 	output_t output = automaton_outputs_output (target, idx); */
  /* 	size_t idx2; */
  /* 	for (idx2 = automaton_output_first (target, output); */
  /* 	     idx2 != automaton_output_last (target, output); */
  /* 	     idx2 = automaton_output_next (target, output, idx2)) { */
  /* 	  aid_t in_automaton; */
  /* 	  input_t input; */
  /* 	  aid_t automaton; */
  /* 	  automaton_output_input (target, output, idx2, &in_automaton, &input, &automaton); */
  /* 	  /\* Decompose. *\/ */
  /* 	  automaton_composition_decompose (automaton, target, output, in_automaton, input); */
  /* 	  automaton_input_decompose (in_automaton, input); */
  /* 	  /\* Tell the composer. *\/ */
  /* 	  decomposed (automaton); */
  /* 	} */
  /*     } */

  /*     /\* Decompose. *\/ */
  /*     for (idx = automaton_compositions_first (target); */
  /* 	   idx != automaton_compositions_last (target); */
  /* 	   idx = automaton_compositions_next (target, idx)) { */
  /* 	aid_t out_automaton; */
  /* 	output_t output; */
  /* 	aid_t in_automaton; */
  /* 	input_t input; */
  /* 	automaton_compositions_composition (target, idx, &out_automaton, &output, &in_automaton, &input); */
  /* 	/\* Decompose. *\/ */
  /* 	automaton_output_decompose (out_automaton, output, target); */
  /* 	automaton_input_decompose (in_automaton, input); */
  /* 	/\* Tell the composer and output. *\/ */
  /* 	output_decomposed (out_automaton, output); */
  /*     } */

  /*     aid_t parent = automaton_parent (target); */
  /*     if (parent != NULL) { */
  /* 	/\* Remove from parent. *\/ */
  /* 	automaton_children_remove (parent, target); */
	
  /* 	/\* Tell parent. *\/ */
  /* 	child_destroyed (parent, target); */
  /*     } */

  /*     /\* Destroy. *\/ */
  /*     automaton_destroy (target); */
  /*   } */
  /*   else { */
  /*     /\* Not the parent. *\/ */
  /*     not_owner (destroyer); */
  /*   } */
  /* } */
  /* else { */
  /*   /\* Target doesn't exist. *\/ */
  /*   automaton_dne (destroyer); */
  /* } */
}

void
ueioa_run (descriptor_t* descriptor)
{
  assert (descriptor != NULL);
  assert (descriptor_check (descriptor));

  runq = runq_create ();
  automata = automata_create ();
  buffers = buffers_create ();
  receipts = receipts_create ();
  
  create (-1, descriptor);

  for (;;) {
    printf ("runq size = %lu\n", runq_size (runq));
    runnable_t runnable;
    runq_pop (runq, &runnable);
    switch (runnable.type) {
    case SYSTEM_INPUT:
      {
	receipt_t receipt;
	if (receipts_pop (receipts, runnable.aid, &receipt) == 0) {
	  automata_system_input_exec (automata, buffers, runnable.aid, &receipt);
	  /* Schedule again. */
	  if (!receipts_empty (receipts, runnable.aid)) {
	    runq_insert_system_input (runq, runnable.aid);
	  }
	}
      }
      break;
    case SYSTEM_OUTPUT:
      {
	order_t order;
	int s = automata_system_output_exec (automata, buffers, runnable.aid, &order);
	if (s == A_GOOD_ORDER) {
	  switch (order.type) {
	  case CREATE:
	    create (runnable.aid, order.create.descriptor);
	    break;
	  case COMPOSE:
	    compose (runnable.aid, order.compose.out_aid, order.compose.output, order.compose.in_aid, order.compose.input);
	    break;
	  case DECOMPOSE:
	    decompose (runnable.aid, order.decompose.out_aid, order.decompose.output, order.decompose.in_aid, order.decompose.input);
	    break;
	  case DESTROY:
	    destroy (runnable.aid, order.destroy.aid);
	    break;
	  default:
	    receipts_push_bad_order (receipts, runnable.aid);
	    runq_insert_system_input (runq, runnable.aid);
	    break;
	  }
	}
	else if (s == A_BAD_ORDER) {
	  receipts_push_bad_order (receipts, runnable.aid);
	  runq_insert_system_input (runq, runnable.aid);
	}
      }
      break;
    case OUTPUT:
      automata_output_exec (automata, buffers, runnable.aid, runnable.output.output);
      break;
    case INTERNAL:
      automata_internal_exec (automata, runnable.aid, runnable.internal.internal);
      break;
    }
  }

  receipts_destroy (receipts);
  buffers_destroy (buffers);
  automata_destroy (automata);
  runq_destroy (runq);
}

void
schedule_system_output (void)
{
  runq_insert_system_output (runq, automata_get_current_aid (automata));
}

int
schedule_output (output_t output)
{
  /* Get the current automaton.  Guaranteed to exist. */
  aid_t aid = automata_get_current_aid (automata);

  if (automata_output_exists (automata, aid, output)) {
    runq_insert_output (runq, aid, output);
    return 0;
  }
  else {
    return -1;
  }
}

int
schedule_internal (internal_t internal)
{
  /* Get the current automaton.  Guaranteed to exist. */
  aid_t aid = automata_get_current_aid (automata);

  if (automata_internal_exists (automata, aid, internal)) {
    runq_insert_internal (runq, aid, internal);
    return 0;
  }
  else {
    return -1;
  }
}

bid_t
buffer_alloc (size_t size)
{
  return buffers_alloc (buffers, automata_get_current_aid (automata), size);
}

void*
buffer_write_ptr (bid_t bid)
{
  return buffers_write_ptr (buffers, automata_get_current_aid (automata), bid);
}

const void*
buffer_read_ptr (bid_t bid)
{
  return buffers_read_ptr (buffers, automata_get_current_aid (automata), bid);
}

size_t
buffer_size (bid_t bid)
{
  return buffers_size (buffers, automata_get_current_aid (automata), bid);
}

void
buffer_incref (bid_t bid)
{
  buffers_incref (buffers, automata_get_current_aid (automata), bid);
}

void
buffer_decref (bid_t bid)
{
  buffers_decref (buffers, automata_get_current_aid (automata), bid);
}

void
buffer_add_child (bid_t parent, bid_t child)
{
  buffers_add_child (buffers, automata_get_current_aid (automata), parent, child);
}

void
buffer_remove_child (bid_t parent, bid_t child)
{
  buffers_remove_child (buffers, automata_get_current_aid (automata), parent, child);
}

bid_t
buffer_dup (bid_t bid)
{
  return buffers_dup (buffers, automata_get_current_aid (automata), bid);
}
