#include "automata.h"

#include <pthread.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include "table.h"

struct automata_struct {
  pthread_rwlock_t lock;
  pthread_key_t current_aid;
  aid_t next_aid;

  table_t* automaton_table;
  index_t* automaton_index;

  table_t* input_table;
  index_t* input_index;
  table_t* output_table;
  index_t* output_index;
  table_t* internal_table;
  index_t* internal_index;
  table_t* param_table;
  index_t* param_index;

  table_t* composition_table;
  index_t* composition_index;
};

static void
set_current_aid (automata_t* automata, aid_t aid)
{
  pthread_setspecific (automata->current_aid, (void*)aid);
}

typedef struct {
  aid_t aid;
  aid_t parent;
  void* state;
  pthread_mutex_t* lock;
  input_t system_input;
  output_t system_output;
} automaton_entry_t;

static bool
automaton_entry_aid_equal (const void* x0, void* y0)
{
  const automaton_entry_t* x = x0;
  const automaton_entry_t* y = y0;
  return x->aid == y->aid;
}

static automaton_entry_t*
automaton_entry_for_aid (automata_t* automata, aid_t aid, iterator_t* ptr)
{
  assert (automata != NULL);

  automaton_entry_t key = {
    .aid = aid
  };

  return index_find_value (automata->automaton_index,
			   index_begin (automata->automaton_index),
			   index_end (automata->automaton_index),
			   automaton_entry_aid_equal,
			   &key,
			   ptr);
}

static bool
automaton_entry_parent_equal (const void* x0, void* y0)
{
  const automaton_entry_t* x = x0;
  const automaton_entry_t* y = y0;
  return x->parent == y->parent;
}

static automaton_entry_t*
automaton_entry_for_parent (automata_t* automata, aid_t aid, iterator_t* ptr)
{
  assert (automata != NULL);

  automaton_entry_t key = {
    .parent = aid
  };

  return index_find_value (automata->automaton_index,
			   index_begin (automata->automaton_index),
			   index_end (automata->automaton_index),
			   automaton_entry_parent_equal,
			   &key,
			   ptr);
}

typedef struct {
  aid_t aid;
  input_t input;
} input_entry_t;

static bool
input_entry_aid_input_equal (const void* x0, void* y0)
{
  const input_entry_t* x = x0;
  const input_entry_t* y = y0;

  return x->aid == y->aid && x->input == y->input;
}

static bool
input_entry_aid_equal (const void* x0, void* y0)
{
  const input_entry_t* x = x0;
  const input_entry_t* y = y0;

  return x->aid == y->aid;
}

static input_entry_t*
input_entry_for_aid_input (automata_t* automata, aid_t aid, input_t input)
{
  assert (automata != NULL);

  input_entry_t key = {
    .aid = aid,
    .input = input
  };
  
  return index_find_value (automata->input_index,
			   index_begin (automata->input_index),
			   index_end (automata->input_index),
			   input_entry_aid_input_equal,
			   &key,
			   NULL);
}

typedef struct {
  aid_t aid;
  output_t output;
} output_entry_t;

static bool
output_entry_aid_output_equal (const void* x0, void* y0)
{
  const output_entry_t* x = x0;
  const output_entry_t* y = y0;

  return x->aid == y->aid && x->output == y->output;
}

static bool
output_entry_aid_equal (const void* x0, void* y0)
{
  const output_entry_t* x = x0;
  const output_entry_t* y = y0;

  return x->aid == y->aid;
}

static output_entry_t*
output_entry_for_aid_output (automata_t* automata, aid_t aid, output_t output)
{
  assert (automata != NULL);

  output_entry_t key = {
    .aid = aid,
    .output = output
  };
  
  return index_find_value (automata->output_index,
			   index_begin (automata->output_index),
			   index_end (automata->output_index),
			   output_entry_aid_output_equal,
			   &key,
			   NULL);
}

typedef struct {
  aid_t aid;
  internal_t internal;
} internal_entry_t;

static bool
internal_entry_aid_internal_equal (const void* x0, void* y0)
{
  const internal_entry_t* x = x0;
  const internal_entry_t* y = y0;

  return x->aid == y->aid && x->internal == y->internal;
}

static bool
internal_entry_aid_equal (const void* x0, void* y0)
{
  const internal_entry_t* x = x0;
  const internal_entry_t* y = y0;

  return x->aid == y->aid;
}

static internal_entry_t*
internal_entry_for_aid_internal (automata_t* automata, aid_t aid, internal_t internal)
{
  assert (automata != NULL);

  internal_entry_t key = {
    .aid = aid,
    .internal = internal
  };
  
  return index_find_value (automata->internal_index,
			   index_begin (automata->internal_index),
			   index_end (automata->internal_index),
			   internal_entry_aid_internal_equal,
			   &key,
			   NULL);
}

typedef struct {
  aid_t aid;
  void* param;
} param_entry_t;

static bool
param_entry_aid_equal (const void* x0, void* y0)
{
  const param_entry_t* x = x0;
  const param_entry_t* y = y0;

  return x->aid == y->aid;
}

static bool
param_entry_aid_param_equal (const void* x0, void* y0)
{
  const param_entry_t* x = x0;
  const param_entry_t* y = y0;

  return
    x->aid == y->aid &&
    x->param == y->param;
}

static param_entry_t*
param_entry_for_aid_param (automata_t* automata, aid_t aid, void* param)
{
  assert (automata != NULL);
  
  param_entry_t key = {
    .aid = aid,
    .param = param,
  };
  
  return index_find_value (automata->param_index,
			   index_begin (automata->param_index),
			   index_end (automata->param_index),
			   param_entry_aid_param_equal,
			   &key,
			   NULL);
}


typedef struct {
  aid_t aid;
  aid_t out_aid;
  output_t output;
  void* out_param;
  aid_t in_aid;
  input_t input;
  void* in_param;
} composition_entry_t;

static bool
composition_entry_in_aid_input_in_param_equal (const void* x0, void* y0)
{
  const composition_entry_t* x = x0;
  const composition_entry_t* y = y0;

  return
    x->in_aid == y->in_aid &&
    x->input == y->input &&
    x->in_param == y->in_param;
}

static bool
composition_sorted (const void* x0, void* y0)
{
  const composition_entry_t* x = x0;
  const composition_entry_t* y = y0;

  if (x->out_aid != y->out_aid) {
    return x->out_aid < y->out_aid;
  }
  else {
    return x->in_aid < y->in_aid;
  }
}

static composition_entry_t*
composition_entry_for_in_aid_input_in_param (automata_t* automata, aid_t in_aid, input_t input, void* in_param, iterator_t* ptr)
{
  assert (automata != NULL);
  
  composition_entry_t key = {
    .in_aid = in_aid,
    .input = input,
    .in_param = in_param,
  };
  
  return index_find_value (automata->composition_index,
			   index_begin (automata->composition_index),
			   index_end (automata->composition_index),
			   composition_entry_in_aid_input_in_param_equal,
			   &key,
			   ptr);
}

static bool
composition_entry_out_aid_output_out_param_in_aid_equal (const void* x0, void* y0)
{
  const composition_entry_t* x = x0;
  const composition_entry_t* y = y0;

  return
    x->out_aid == y->out_aid &&
    x->output == y->output &&
    x->out_param == y->out_param &&
    x->in_aid == y->in_aid;
}

static composition_entry_t*
composition_entry_for_out_aid_output_out_param_in_aid (automata_t* automata, aid_t out_aid, output_t output, void* out_param, aid_t in_aid)
{
  assert (automata != NULL);
  
  composition_entry_t key = {
    .out_aid = out_aid,
    .output = output,
    .out_param = out_param,
    .in_aid = in_aid,
  };
  
  return index_find_value (automata->composition_index,
			   index_begin (automata->composition_index),
			   index_end (automata->composition_index),
			   composition_entry_out_aid_output_out_param_in_aid_equal,
			   &key,
			   NULL);
}

static bool
composition_entry_any_param_equal (const void* x0, void* y0)
{
  const composition_entry_t* x = x0;
  const composition_entry_t* y = y0;

  return
    (x->out_aid == y->out_aid && x->out_param == y->out_param) ||
    (x->in_aid == y->in_aid && x->in_param == y->in_param);
}

static bool
composition_entry_any_aid_equal (const void* x0, void* y0)
{
  const composition_entry_t* x = x0;
  const composition_entry_t* y = y0;

  return
    x->aid == y->aid ||
    x->out_aid == y->aid ||
    x->in_aid == y->aid;
}

static void
create (automata_t* automata, receipts_t* receipts, runq_t* runq, aid_t parent, descriptor_t* descriptor)
{
  assert (automata != NULL);
  assert (receipts != NULL);
  assert (runq != NULL);

  if (descriptor != NULL &&
      descriptor_check (descriptor)) {
    
    /* Find an aid. */
    while (automaton_entry_for_aid (automata, automata->next_aid, NULL) != NULL) {
      ++automata->next_aid;
      if (automata->next_aid < 0) {
	automata->next_aid = 0;
      }
    }
    
    aid_t aid = automata->next_aid;
    
    /* Insert. */
    pthread_mutex_t* lock = malloc (sizeof (pthread_mutex_t));
    pthread_mutex_init (lock, NULL);
    automaton_entry_t entry = {
      .aid = aid,
      .system_input = descriptor->system_input,
      .lock = lock,
      .system_output = descriptor->system_output
    };
    index_insert (automata->automaton_index, &entry);
    
    size_t idx;
    for (idx = 0; descriptor->inputs[idx] != NULL; ++idx) {
      input_entry_t entry = {
	.aid = aid,
	.input = descriptor->inputs[idx],
      };
      index_insert (automata->input_index, &entry);
    }
    
    for (idx = 0; descriptor->outputs[idx] != NULL; ++idx) {
      output_entry_t entry = {
	.aid = aid,
	.output = descriptor->outputs[idx],
      };
      index_insert (automata->output_index, &entry);
    }
    
    for (idx = 0; descriptor->internals[idx] != NULL; ++idx) {
      internal_entry_t entry = {
	.aid = aid,
	.internal = descriptor->internals[idx],
      };
      index_insert (automata->internal_index, &entry);
    }

    /* Delcare the NULL paramter. */
    {
      param_entry_t entry = {
	.aid = aid,
	.param = NULL,
      };
      index_insert (automata->param_index, &entry);
    }
    
    automaton_entry_t* automaton_entry = automaton_entry_for_aid (automata, aid, NULL);
    
    set_current_aid (automata, aid);
    automaton_entry->state = descriptor->constructor ();
    set_current_aid (automata, -1);

    automaton_entry->parent = parent;
    
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
declare (automata_t* automata, receipts_t* receipts, runq_t* runq, aid_t aid, void* param)
{
  assert (automata != NULL);
  assert (receipts != NULL);
  assert (runq != NULL);

  param_entry_t entry = {
    .aid = aid,
    .param = param,
  };
  index_insert_unique (automata->param_index, param_entry_aid_param_equal, &entry);

  receipts_push_declared (receipts, aid);
  runq_insert_system_input (runq, aid);
}

static void
compose (automata_t* automata, receipts_t* receipts, runq_t* runq, aid_t aid, aid_t out_aid, output_t output, void* out_param, aid_t in_aid, input_t input, void* in_param)
{
  assert (automata != NULL);
  assert (receipts != NULL);
  assert (runq != NULL);

  if (output_entry_for_aid_output (automata, out_aid, output) == NULL ||
      param_entry_for_aid_param (automata, out_aid, out_param) == NULL) {
    /* Output doesn't exist. */
    receipts_push_output_dne (receipts, aid);
    runq_insert_system_input (runq, aid);
  }
  else if (input_entry_for_aid_input (automata, in_aid, input) == NULL ||
	   param_entry_for_aid_param (automata, in_aid, in_param) == NULL) {
    /* Input doesn't exist. */
    receipts_push_input_dne (receipts, aid);
    runq_insert_system_input (runq, aid);
  }
  else if (composition_entry_for_in_aid_input_in_param (automata, in_aid, input, in_param, NULL) != NULL ||
	   (in_param != NULL && aid != in_aid) ||
	   (out_aid == in_aid)) {
    /* Input isn't available.

       The (in_param != NULL && aid != in_aid) deserves some explaining.
       Parameters are private in the sense that they are controlled by the automaton that declares them.
       Any composition involving an action with a non-NULL parameter must be performed by the automaton owning the action.
       Thus, a composition will succeed with two null parameters or one non-null parameter so long at the composing automaton owns the non-null parameter.

       The (out_aid == in_aid) prevents an automaton from being composed with itself.
       This makes locking easier as the automata involved in the action form a set instead of a bag.
     */
    receipts_push_input_unavailable (receipts, aid);
    runq_insert_system_input (runq, aid);
  }
  else if (composition_entry_for_out_aid_output_out_param_in_aid (automata, out_aid, output, out_param, in_aid) != NULL ||
	   (out_param != NULL && aid != out_aid)) {
    /* Output isn't available.
     */
    receipts_push_output_unavailable (receipts, aid);
    runq_insert_system_input (runq, aid);
  }
  else {
    /* Compose. */
    composition_entry_t entry = {
      .aid = aid,
      .out_aid = out_aid,
      .output = output,
      .out_param = out_param,
      .in_aid = in_aid,
      .input = input,
      .in_param = in_param,
    };
    index_insert (automata->composition_index, &entry);
    
    receipts_push_composed (receipts, aid);
    runq_insert_system_input (runq, aid);

    receipts_push_input_composed (receipts, in_aid, input, in_param);
    runq_insert_system_input (runq, in_aid);
    
    receipts_push_output_composed (receipts, out_aid, output, out_param);
    runq_insert_system_input (runq, out_aid);
  }
}

static void
decompose (automata_t* automata, receipts_t* receipts, runq_t* runq, aid_t aid, aid_t out_aid, output_t output, void* out_param, aid_t in_aid, input_t input, void* in_param)
{
  assert (automata != NULL);
  assert (receipts != NULL);
  assert (runq != NULL);

  /* Look up the composition.
     We only need to use the input since it must be unique.
  */
  iterator_t iterator;
  composition_entry_t* entry = composition_entry_for_in_aid_input_in_param (automata, in_aid, input, in_param, &iterator);
  if (entry != NULL &&
      entry->out_aid == out_aid &&
      entry->output == output &&
      entry->out_param == out_param &&
      entry->aid == aid) {
    index_erase (automata->composition_index, iterator);
    
    receipts_push_decomposed (receipts, aid, in_aid, input, in_param);
    runq_insert_system_input (runq, aid);
    
    receipts_push_input_decomposed (receipts, in_aid, input, in_param);
    runq_insert_system_input (runq, in_aid);
    
    receipts_push_output_decomposed (receipts, out_aid, output, out_param);
    runq_insert_system_input (runq, out_aid);
  }
  else {
    receipts_push_not_composed (receipts, aid);
    runq_insert_system_input (runq, aid);
  }
}

typedef struct {
  aid_t aid;
  void* param;
  receipts_t* receipts;
  runq_t* runq;
} rescind_decompose_arg_t;

static void
rescind_decompose (const void* e, void* a)
{
  const composition_entry_t* entry = e;
  rescind_decompose_arg_t* arg = a;

  if ((entry->in_aid == arg->aid && entry->in_param == arg->param) ||
      (entry->out_aid == arg->aid && entry->out_param == arg->param)) {
    /* Composer. */
    receipts_push_decomposed (arg->receipts, entry->aid, entry->in_aid, entry->input, entry->in_param);
    runq_insert_system_input (arg->runq, entry->aid);
    
    /* Input. */
    receipts_push_input_decomposed (arg->receipts, entry->in_aid, entry->input, entry->in_param);
    runq_insert_system_input (arg->runq, entry->in_aid);
    
    /* Output. */
    receipts_push_output_decomposed (arg->receipts, entry->out_aid, entry->output, entry->out_param);
    runq_insert_system_input (arg->runq, entry->out_aid);
  }
}

static void
rescind (automata_t* automata, receipts_t* receipts, runq_t* runq, aid_t aid, void* param)
{
  assert (automata != NULL);
  assert (receipts != NULL);
  assert (runq != NULL);

  /* Params. */
  param_entry_t param_key = {
    .aid = aid,
    .param = param,
  };
  index_remove (automata->param_index,
  		index_begin (automata->param_index),
  		index_end (automata->param_index),
  		param_entry_aid_param_equal,
  		&param_key);

  receipts_push_rescinded (receipts, aid);
  runq_insert_system_input (runq, aid);

  /* Compositions. */
  rescind_decompose_arg_t arg = {
    .aid = aid,
    .param = param,
    .receipts = receipts,
    .runq = runq
  };
  index_for_each (automata->param_index,
  		  index_begin (automata->param_index),
  		  index_end (automata->param_index),
  		  rescind_decompose,
  		  &arg);

  composition_entry_t composition_key = {
    .out_aid = aid,
    .out_param = param,
    .in_aid = aid,
    .in_param = param,
  };
  index_remove (automata->composition_index,
  		index_begin (automata->composition_index),
  		index_end (automata->composition_index),
  		composition_entry_any_param_equal,
  		&composition_key);

  /* Runq. */
  runq_purge_aid_param (runq, aid, param);
}

typedef struct {
  aid_t aid;
  receipts_t* receipts;
  runq_t* runq;
} destroy_decompose_arg_t;

static void
destroy_decompose (const void* e, void* a)
{
  const composition_entry_t* entry = e;
  destroy_decompose_arg_t* arg = a;

  if (arg->aid == entry->aid ||
      arg->aid == entry->in_aid ||
      arg->aid == entry->out_aid) {
    /* Composer. */
    receipts_push_decomposed (arg->receipts, entry->aid, entry->in_aid, entry->input, entry->in_param);
    runq_insert_system_input (arg->runq, entry->aid);
    
    /* Input. */
    receipts_push_input_decomposed (arg->receipts, entry->in_aid, entry->input, entry->in_param);
    runq_insert_system_input (arg->runq, entry->in_aid);
    
    /* Output. */
    receipts_push_output_decomposed (arg->receipts, entry->out_aid, entry->output, entry->out_param);
    runq_insert_system_input (arg->runq, entry->out_aid);
  }
}

static void
destroy_r (automata_t* automata, receipts_t* receipts, runq_t* runq, buffers_t* buffers, aid_t aid)
{
  /* Children. */
  automaton_entry_t* entry;
  while ((entry = automaton_entry_for_parent (automata, aid, NULL)) != NULL) {
    destroy_r (automata, receipts, runq, buffers, entry->aid);
  }

  /* Automaton. */
  iterator_t iterator;
  entry = automaton_entry_for_aid (automata, aid, &iterator);
  free (entry->lock);
  free (entry->state);
  aid_t parent = entry->parent;
  index_erase (automata->automaton_index, iterator);

  /* Inputs. */
  input_entry_t input_key = {
    .aid = aid
  };
  index_remove (automata->input_index,
		index_begin (automata->input_index),
		index_end (automata->input_index),
		input_entry_aid_equal,
		&input_key);

  /* Outputs. */
  output_entry_t output_key = {
    .aid = aid
  };
  index_remove (automata->output_index,
		index_begin (automata->output_index),
		index_end (automata->output_index),
		output_entry_aid_equal,
		&output_key);

  /* Internals. */
  internal_entry_t internal_key = {
    .aid = aid
  };
  index_remove (automata->internal_index,
		index_begin (automata->internal_index),
		index_end (automata->internal_index),
		internal_entry_aid_equal,
		&internal_key);

  /* Params. */
  param_entry_t param_key = {
    .aid = aid
  };
  index_remove (automata->param_index,
		index_begin (automata->param_index),
		index_end (automata->param_index),
		param_entry_aid_equal,
		&param_key);

  /* Compositions. */
  destroy_decompose_arg_t arg = {
    .aid = aid,
    .receipts = receipts,
    .runq = runq
  };
  index_for_each (automata->composition_index,
		  index_begin (automata->composition_index),
		  index_end (automata->composition_index),
		  destroy_decompose,
		  &arg);

  composition_entry_t composition_key = {
    .aid = aid
  };
  index_remove (automata->composition_index,
  		index_begin (automata->composition_index),
  		index_end (automata->composition_index),
  		composition_entry_any_aid_equal,
  		&composition_key);

  /* Runq. */
  runq_purge_aid (runq, aid);

  /* Receipts. */
  receipts_purge_aid (receipts, aid);
  
  /* Buffers. */
  buffers_purge_aid (buffers, aid);

  if (parent != -1) {
    receipts_push_child_destroyed (receipts, parent, aid);
    runq_insert_system_input (runq, parent);
  }
}

static void
destroy (automata_t* automata, receipts_t* receipts, runq_t* runq, buffers_t* buffers, aid_t aid, aid_t target)
{
  assert (automata != NULL);
  assert (receipts != NULL);
  assert (runq != NULL);
  assert (buffers != NULL);

  automaton_entry_t* target_entry = automaton_entry_for_aid (automata, target, NULL);
  if (target_entry != NULL) {
    if (aid == target ||
	aid == target_entry->parent) {
      destroy_r (automata, receipts, runq, buffers, target);
    }
    else {
      receipts_push_not_owner (receipts, aid);
      runq_insert_system_input (runq, aid);
    }
  }
  else {
    receipts_push_automaton_dne (receipts, aid);
    runq_insert_system_input (runq, aid);
  }
}

void
automata_create_automaton (automata_t* automata, receipts_t* receipts, runq_t* runq, descriptor_t* descriptor)
{
  assert (automata != NULL);
  assert (receipts != NULL);
  assert (runq != NULL);

  /* Acquire the write lock. */
  pthread_rwlock_wrlock (&automata->lock);

  create (automata, receipts, runq, -1, descriptor);
  
  /* Release the write lock. */
  pthread_rwlock_unlock (&automata->lock);
}

void
automata_system_input_exec (automata_t* automata, receipts_t* receipts, runq_t* runq, buffers_t* buffers, aid_t aid)
{
  assert (automata != NULL);
  assert (receipts != NULL);
  assert (runq != NULL);
  assert (buffers != NULL);

  /* Acquire the read lock. */
  pthread_rwlock_rdlock (&automata->lock);

  automaton_entry_t* entry = automaton_entry_for_aid (automata, aid, NULL);

  /* Automaton must exist. */
  if (entry != NULL) {
    
    receipt_t receipt;
    if (receipts_pop (receipts, aid, &receipt) == 0) {

      /* Prepare the buffer. */
      bid_t bid = buffers_alloc (buffers, aid, sizeof (receipt_t));
      receipt_t* r = buffers_write_ptr (buffers, aid, bid);
      *r = receipt;
      /* Increment to make read-only. */
      buffers_incref (buffers, aid, bid);
      set_current_aid (automata, aid);
      pthread_mutex_lock (entry->lock);
      entry->system_input (entry->state, NULL, bid);
      pthread_mutex_unlock (entry->lock);
      set_current_aid (automata, -1);
      /* Decrement to destroy. */
      buffers_decref (buffers, aid, bid);
      
      /* Schedule again. */
      if (!receipts_empty (receipts, aid)) {
	runq_insert_system_input (runq, aid);
      }
    }

  }  
  
  /* Release the read lock. */
  pthread_rwlock_unlock (&automata->lock);
}

void
automata_system_output_exec (automata_t* automata, receipts_t* receipts, runq_t* runq, ioq_t* ioq, buffers_t* buffers, aid_t aid)
{
  assert (automata != NULL);
  assert (receipts != NULL);
  assert (runq != NULL);
  assert (ioq != NULL);
  assert (buffers != NULL);

  /* Acquire the write lock. */
  pthread_rwlock_wrlock (&automata->lock);

  automaton_entry_t* entry = automaton_entry_for_aid (automata, aid, NULL);
  /* Automaton must exist. */
  if (entry != NULL) {

    /* Execute. */
    set_current_aid (automata, aid);
    pthread_mutex_lock (entry->lock);
    bid_t bid = entry->system_output (entry->state, NULL);
    pthread_mutex_unlock (entry->lock);
    set_current_aid (automata, -1);
    
    if (bid != -1) {
      if (buffers_size (buffers, aid, bid) == sizeof (order_t)) {
	buffers_incref (buffers, aid, bid);
	order_t order = *(order_t*)buffers_read_ptr (buffers, aid, bid);
	buffers_decref (buffers, aid, bid);

	switch (order.type) {
	case CREATE:
	  create (automata, receipts, runq, aid, order.create.descriptor);
	  break;
	case DECLARE:
	  declare (automata, receipts, runq, aid, order.declare.param);
	  break;
	case COMPOSE:
	  compose (automata, receipts, runq, aid, order.compose.out_aid, order.compose.output, order.compose.out_param, order.compose.in_aid, order.compose.input, order.compose.in_param);
	  break;
	case DECOMPOSE:
	  decompose (automata, receipts, runq, aid, order.decompose.out_aid, order.decompose.output, order.decompose.out_param, order.decompose.in_aid, order.decompose.input, order.decompose.in_param);
	  break;
	case RESCIND:
	  rescind (automata, receipts, runq, aid, order.rescind.param);
	  break;
	case DESTROY:
	  destroy (automata, receipts, runq, buffers, aid, order.destroy.aid);
	  break;
	case SET_ALARM:
	  ioq_insert_alarm (ioq, aid, order.alarm.secs, order.alarm.usecs);
	  break;
	case SET_WRITE_ALARM:
	  ioq_insert_write (ioq, aid, order.write.fd);
	  break;
	default:
	  receipts_push_bad_order (receipts, aid);
	  runq_insert_system_input (runq, aid);
	  break;
	}
      }
      else {
	receipts_push_bad_order (receipts, aid);
	runq_insert_system_input (runq, aid);
      }
    }
  }

  /* Release the write lock. */
  pthread_rwlock_unlock (&automata->lock);
}

typedef struct {
  automata_t* automata;
  buffers_t* buffers;
  automaton_entry_t* out_entry;
  aid_t out_aid;
  output_t output;
  bool output_done;
  bid_t bid;
} output_arg_t;

static void
output_lock (const void* e, void* a)
{
  const composition_entry_t* composition_entry = e;
  output_arg_t* arg = a;

  if (composition_entry->out_aid == arg->out_aid && composition_entry->output == arg->output) {
    if (!arg->output_done && arg->out_aid < composition_entry->in_aid) {
      pthread_mutex_lock (arg->out_entry->lock);
      arg->output_done = true;
    }
    automaton_entry_t* in_entry = automaton_entry_for_aid (arg->automata, composition_entry->in_aid, NULL);
    pthread_mutex_lock (in_entry->lock);
  }
}

static void
output_exec (const void* e, void* a)
{
  const composition_entry_t* composition_entry = e;
  output_arg_t* arg = a;

  if (composition_entry->out_aid == arg->out_aid && composition_entry->output == arg->output) {
    automaton_entry_t* in_entry = automaton_entry_for_aid (arg->automata, composition_entry->in_aid, NULL);
    set_current_aid (arg->automata, composition_entry->in_aid);
    buffers_change_owner (arg->buffers, composition_entry->in_aid, arg->bid);
    composition_entry->input (in_entry->state, composition_entry->in_param, arg->bid);
  }
}

static void
output_unlock (const void* e, void* a)
{
  const composition_entry_t* composition_entry = e;
  output_arg_t* arg = a;

  if (composition_entry->out_aid == arg->out_aid && composition_entry->output == arg->output) {
    if (!arg->output_done && arg->out_aid < composition_entry->in_aid) {
      pthread_mutex_unlock (arg->out_entry->lock);
      arg->output_done = true;
    }
    automaton_entry_t* in_entry = automaton_entry_for_aid (arg->automata, composition_entry->in_aid, NULL);
    pthread_mutex_unlock (in_entry->lock);
  }
}

void
automata_output_exec (automata_t* automata, buffers_t* buffers, aid_t out_aid, output_t output, void* out_param)
{
  assert (automata != NULL);
  assert (buffers != NULL);

  /* Acquire the read lock. */
  pthread_rwlock_rdlock (&automata->lock);

  automaton_entry_t* out_entry = automaton_entry_for_aid (automata, out_aid, NULL);
  param_entry_t* param_entry = param_entry_for_aid_param (automata, out_aid, out_param);

  /* Automaton and param must exist. */
  if (out_entry != NULL &&
      param_entry != NULL) {

    output_arg_t arg = {
      .automata = automata,
      .buffers = buffers,
      .out_entry = out_entry,
      .out_aid = out_aid,
      .output = output,
      .output_done = false,
    };

    /* Lock the automata in order. */
    index_for_each (automata->composition_index,
		    index_begin (automata->composition_index),
		    index_end (automata->composition_index),
		    output_lock,
		    &arg);
    
    /* Execute the output. */
    set_current_aid (automata, out_aid);
    bid_t bid = output (out_entry->state, out_param);
    if (bid != -1) {
      /* Increment the reference count so the following decref will destroy the buffer if no
       * input references the buffer.
       */
      buffers_incref (buffers, out_aid, bid);
      
      arg.bid = bid;
      /* Execute the inputs. */
      index_for_each (automata->composition_index,
		      index_begin (automata->composition_index),
		      index_end (automata->composition_index),
		      output_exec,
		      &arg);

      buffers_change_owner (buffers, -1, bid);
            
      buffers_decref (buffers, out_aid, bid);
    }
    
    set_current_aid (automata, -1);

    arg.output_done = false;
    /* Unlock. */
    index_for_each (automata->composition_index,
		    index_begin (automata->composition_index),
		    index_end (automata->composition_index),
		    output_unlock,
		    &arg);
  }

  /* Release the read lock. */
  pthread_rwlock_unlock (&automata->lock);
}

void
automata_internal_exec (automata_t* automata, aid_t aid, internal_t internal, void* param)
{
  assert (automata != NULL);

  /* Acquire the read lock. */
  pthread_rwlock_rdlock (&automata->lock);

  /* Look up the automaton and param. */
  automaton_entry_t* entry = automaton_entry_for_aid (automata, aid, NULL);
  param_entry_t* param_entry = param_entry_for_aid_param (automata, aid, param);

  /* Automaton and param must exist. */
  if (entry != NULL &&
      param_entry != NULL) {
    
    /* Set the current automaton. */
    set_current_aid (automata, aid);
    
    /* Lock the automaton. */
    pthread_mutex_lock (entry->lock);
    
    /* Execute. */
    internal (entry->state, param);
    
    /* Unlock the automaton. */
    pthread_mutex_unlock (entry->lock);
    
    /* Clear the current automaton. */
    set_current_aid (automata, -1);
  }

  /* Release the read lock. */
  pthread_rwlock_unlock (&automata->lock);
}

aid_t
automata_get_current_aid (automata_t* automata)
{
  assert (automata != NULL);

  return (aid_t)pthread_getspecific (automata->current_aid);
}

bool
automata_output_exists (automata_t* automata, aid_t aid, output_t output, void* param)
{
  assert (automata != NULL);

  return
    (output_entry_for_aid_output (automata, aid, output) != NULL) &&
    (param_entry_for_aid_param (automata, aid, param) != NULL);
}

bool
automata_internal_exists (automata_t* automata, aid_t aid, internal_t internal, void* param)
{
  assert (automata != NULL);

  return
    (internal_entry_for_aid_internal (automata, aid, internal) != NULL) &&
    (param_entry_for_aid_param (automata, aid, param) != NULL);
}

automata_t*
automata_create (void)
{
  automata_t* automata = malloc (sizeof (automata_t));

  pthread_key_create (&automata->current_aid, NULL);
  pthread_rwlock_init (&automata->lock, NULL);
  automata->next_aid = 0;

  automata->automaton_table = table_create (sizeof (automaton_entry_t));
  automata->automaton_index = index_create_list (automata->automaton_table);
  automata->input_table = table_create (sizeof (input_entry_t));
  automata->input_index = index_create_list (automata->input_table);
  automata->output_table = table_create (sizeof (output_entry_t));
  automata->output_index = index_create_list (automata->output_table);
  automata->internal_table = table_create (sizeof (internal_entry_t));
  automata->internal_index = index_create_list (automata->internal_table);
  automata->param_table = table_create (sizeof (param_entry_t));
  automata->param_index = index_create_list (automata->param_table);

  automata->composition_table = table_create (sizeof (composition_entry_t));
  automata->composition_index = index_create_ordered_list (automata->composition_table, composition_sorted);

  return automata;
}

static void
free_state_and_lock (void* e, void* ignored)
{
  automaton_entry_t* entry = e;

  free (entry->state);
  pthread_mutex_destroy (entry->lock);
  free (entry->lock);
}

void
automata_destroy (automata_t* automata)
{
  assert (automata != NULL);

  table_destroy (automata->composition_table);

  table_destroy (automata->param_table);
  table_destroy (automata->internal_table);
  table_destroy (automata->output_table);
  table_destroy (automata->input_table);
  index_transform (automata->automaton_index,
		   index_begin (automata->automaton_index),
		   index_end (automata->automaton_index),
		   free_state_and_lock,
		   NULL);
  table_destroy (automata->automaton_table);

  pthread_rwlock_destroy (&automata->lock);
  pthread_key_delete (automata->current_aid);

  free (automata);
}
