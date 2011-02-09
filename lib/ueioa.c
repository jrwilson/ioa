#include "ueioa.h"

#include <assert.h>
#include <pthread.h>

#include <stdio.h>

#include "runq.h"
#include "automata.h"
#include "receipts.h"

static runq_t* runq;
static automata_t* automata;
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
create (aid_t parent, automaton_descriptor_t* descriptor)
{
  /* Parent can be -1, i.e., for the first automaton. */
  if (descriptor != NULL && automaton_descriptor_check (descriptor)) {
    aid_t aid = automaton_create (automata, descriptor, parent);
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
  switch (automaton_compose (automata, aid, out_aid, output, in_aid, input)) {
  case COMPOSED:
    receipts_push_composed (receipts, aid);
    runq_insert_system_input (runq, aid);

    receipts_push_output_composed (receipts, out_aid, output);
    runq_insert_system_input (runq, out_aid);

    break;
  case OUTPUT_DNE:
    receipts_push_output_dne (receipts, aid);
    runq_insert_system_input (runq, aid);
    break;
  case INPUT_DNE:
    receipts_push_input_dne (receipts, aid);
    runq_insert_system_input (runq, aid);
    break;
  case INPUT_UNAVAILABLE:
    receipts_push_input_unavailable (receipts, aid);
    runq_insert_system_input (runq, aid);
    break;
  case OUTPUT_UNAVAILABLE:
    receipts_push_output_unavailable (receipts, aid);
    runq_insert_system_input (runq, aid);
    break;
  default:
    /* Unhandled case. */
    assert (0);
  }
}

static void
decompose (aid_t automaton, aid_t out_automaton, output_t output, aid_t in_automaton, input_t input)
{
  assert (0);
  /* if (automaton_exists (out_automaton) && */
  /*     automaton_exists (in_automaton) && */
  /*     automaton_composition_composed (automaton, out_automaton, output, in_automaton, input)) { */
  /*   /\* Decompose. *\/ */
  /*   automaton_composition_decompose (automaton, out_automaton, output, in_automaton, input); */
  /*   automaton_output_decompose (out_automaton, output, in_automaton); */
  /*   automaton_input_decompose (in_automaton, input); */

  /*   /\* Tell the composer and output. *\/ */
  /*   decomposed (automaton); */
  /*   output_decomposed (out_automaton, output); */
  /* } */
  /* else { */
  /*   /\* Tell the composer. *\/ */
  /*   not_composed (automaton); */
  /* } */
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
ueioa_run (automaton_descriptor_t* descriptor)
{
  assert (descriptor != NULL);
  assert (automaton_descriptor_check (descriptor));

  runq = runq_create ();
  automata = automata_create ();
  receipts = receipts_create ();
  
  create (-1, descriptor);

  for (;;) {
    printf ("runq size = %lu\n", runq_size (runq));
    runnable_t* runnable = runq_pop (runq);
    switch (runnable_type (runnable)) {
    case SYSTEM_INPUT:
      {
	/* Unpack. */
	aid_t aid = runnable_aid (runnable);
	system_receipt_t receipt;
	if (receipts_pop (receipts, aid, &receipt) == 0) {
	  automaton_system_input_exec (automata, aid, &receipt);
	  /* Schedule again. */
	  if (!receipts_empty (receipts, aid)) {
	    runq_insert_system_input (runq, aid);
	  }
	}
      }
      break;
    case SYSTEM_OUTPUT:
      {
	aid_t aid = runnable_aid (runnable);
	system_order_t order;
	int s = automaton_system_output_exec (automata, aid, &order);
	if (s == 1) {
	  switch (order.type) {
	  case SYS_CREATE:
	    create (aid, order.create.descriptor);
	    break;
	  case SYS_COMPOSE:
	    compose (aid, order.compose.out_automaton, order.compose.output, order.compose.in_automaton, order.compose.input);
	    break;
	  case SYS_DECOMPOSE:
	    decompose (aid, order.decompose.out_automaton, order.decompose.output, order.decompose.in_automaton, order.decompose.input);
	    break;
	  case SYS_DESTROY:
	    destroy (aid, order.destroy.automaton);
	    break;
	  default:
	    /* TODO: Bad order. */
	    assert (0);
	    break;
	  }
	}
	else if (s == -1) {
	  /* TODO: Bad order. */
	  assert (0);
	}
      }
      break;
    case OUTPUT:
      automaton_output_exec (automata, runnable_aid (runnable), runnable_output_output (runnable));
      break;
    case INTERNAL:
      automaton_internal_exec (automata, runnable_aid (runnable), runnable_internal_internal (runnable));
      break;
    }
    runnable_destroy (runnable);
  }

  receipts_destroy (receipts);
  automata_destroy (automata);
  runq_destroy (runq);
}

void
ueioa_schedule_system_output (void)
{
  runq_insert_system_output (runq, automaton_get_current_aid (automata));
}

int
ueioa_schedule_output (output_t output)
{
  /* Get the current automaton.  Guaranteed to exist. */
  aid_t aid = automaton_get_current_aid (automata);

  if (automaton_output_exists (automata, aid, output)) {
    runq_insert_output (runq, aid, output);
    return 0;
  }
  else {
    return -1;
  }
}

int
ueioa_schedule_internal (internal_t internal)
{
  /* Get the current automaton.  Guaranteed to exist. */
  aid_t aid = automaton_get_current_aid (automata);

  if (automaton_internal_exists (automata, aid, internal)) {
    runq_insert_internal (runq, aid, internal);
    return 0;
  }
  else {
    return -1;
  }
}

bid_t
ueioa_buffer_alloc (size_t size)
{
  return automaton_buffer_alloc (automata, size);
}

void*
ueioa_buffer_write_ptr (bid_t bid)
{
  return automaton_buffer_write_ptr (automata, bid);
}

const void*
ueioa_buffer_read_ptr (bid_t bid)
{
  return automaton_buffer_read_ptr (automata, bid);
}

size_t
ueioa_buffer_size (bid_t bid)
{
  return automaton_buffer_size (automata, bid);
}
