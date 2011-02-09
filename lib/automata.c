#include "automata.h"

#include <pthread.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include "table.h"
#include "buffers.h"

struct automata_struct {
  pthread_rwlock_t lock;
  pthread_key_t current_aid;
  aid_t next_aid;
  buffers_t* buffers;

  table_t* automaton_table;
  index_t* automaton_index;
  table_t* automaton_edge_table;
  index_t* automaton_edge_index;

  table_t* input_table;
  index_t* input_index;
  table_t* output_table;
  index_t* output_index;
  table_t* internal_table;
  index_t* internal_index;

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
  void* state;
  pthread_mutex_t* lock;
  input_t system_input;
  output_t system_output;
} automaton_entry_t;

static bool
automaton_entry_aid_equal (const void* x0, const void* y0)
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

typedef struct {
  aid_t parent;
  aid_t child;
} automaton_edge_entry_t;

typedef struct {
  aid_t aid;
  input_t input;
  bool scheduled;
} input_entry_t;

static bool
input_entry_aid_input_equal (const void* x0, const void* y0)
{
  const input_entry_t* x = x0;
  const input_entry_t* y = y0;

  return x->aid == y->aid && x->input == y->input;
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
  bool scheduled;
} output_entry_t;

static bool
output_entry_aid_output_equal (const void* x0, const void* y0)
{
  const output_entry_t* x = x0;
  const output_entry_t* y = y0;

  return x->aid == y->aid && x->output == y->output;
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
  bool scheduled;
} internal_entry_t;

static bool
internal_entry_aid_internal_equal (const void* x0, const void* y0)
{
  const internal_entry_t* x = x0;
  const internal_entry_t* y = y0;

  return x->aid == y->aid && x->internal == y->internal;
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
  aid_t out_aid;
  output_t output;
  aid_t in_aid;
  input_t input;
} composition_entry_t;

static bool
composition_entry_in_aid_input_equal (const void* x0, const void* y0)
{
  const composition_entry_t* x = x0;
  const composition_entry_t* y = y0;

  return
    x->in_aid == y->in_aid &&
    x->input == y->input;
}

static bool
composition_sorted (const void* x0, const void* y0)
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

static internal_entry_t*
composition_entry_for_in_aid_input (automata_t* automata, aid_t in_aid, input_t input)
{
  assert (automata != NULL);
  
  composition_entry_t key = {
    .in_aid = in_aid,
    .input = input,
  };
  
  return index_find_value (automata->composition_index,
			   index_begin (automata->composition_index),
			   index_end (automata->composition_index),
			   composition_entry_in_aid_input_equal,
			   &key,
			   NULL);
}

static bool
composition_entry_out_aid_output_in_aid_equal (const void* x0, const void* y0)
{
  const composition_entry_t* x = x0;
  const composition_entry_t* y = y0;

  return
    x->out_aid == y->out_aid &&
    x->output == y->output &&
    x->in_aid == y->in_aid;
}

static internal_entry_t*
composition_entry_for_out_aid_output_in_aid (automata_t* automata, aid_t out_aid, output_t output, aid_t in_aid)
{
  assert (automata != NULL);
  
  composition_entry_t key = {
    .out_aid = out_aid,
    .output = output,
    .in_aid = in_aid,
  };
  
  return index_find_value (automata->composition_index,
			   index_begin (automata->composition_index),
			   index_end (automata->composition_index),
			   composition_entry_out_aid_output_in_aid_equal,
			   &key,
			   NULL);
}

aid_t
automaton_create (automata_t* automata, automaton_descriptor_t* descriptor, aid_t parent)
{
  assert (automata != NULL);

  pthread_rwlock_wrlock (&automata->lock);

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
  for (idx = 0; idx < descriptor->input_count; ++idx) {
    input_entry_t entry = {
      .aid = aid,
      .input = descriptor->inputs[idx],
      .scheduled = false,
    };
    index_insert (automata->input_index, &entry);
  }

  for (idx = 0; idx < descriptor->output_count; ++idx) {
    output_entry_t entry = {
      .aid = aid,
      .output = descriptor->outputs[idx],
      .scheduled = false,
    };
    index_insert (automata->output_index, &entry);
  }

  for (idx = 0; idx < descriptor->internal_count; ++idx) {
    internal_entry_t entry = {
      .aid = aid,
      .internal = descriptor->internals[idx],
      .scheduled = false,
    };
    index_insert (automata->internal_index, &entry);
  }

  automaton_entry_t* automaton_entry = automaton_entry_for_aid (automata, aid, NULL);

  set_current_aid (automata, aid);
  automaton_entry->state = descriptor->constructor ();
  set_current_aid (automata, -1);

  if (parent != -1) {
    automaton_edge_entry_t entry = {
      .parent = parent,
      .child = aid,
    };
    index_insert (automata->automaton_edge_index, &entry);
  }

  pthread_rwlock_unlock (&automata->lock);

  return aid;
}

int
automaton_compose (automata_t* automata, aid_t aid, aid_t out_aid, output_t output, aid_t in_aid, input_t input)
{
  assert (automata != NULL);

  if (output_entry_for_aid_output (automata, out_aid, output) == NULL) {
    /* Output doesn't exist. */
    return OUTPUT_DNE;
  }
  else if (input_entry_for_aid_input (automata, in_aid, input) == NULL) {
    /* Input doesn't exist. */
    return INPUT_DNE;
  }
  else if (composition_entry_for_in_aid_input (automata, in_aid, input) != NULL) {
    /* Input isn't available. */
    return INPUT_UNAVAILABLE;
  }
  else if (composition_entry_for_out_aid_output_in_aid (automata, out_aid, output, in_aid) != NULL) {
    /* Output isn't available. */
    return OUTPUT_UNAVAILABLE;
  }
  else {
    /* Compose. */
    composition_entry_t entry = {
      .aid = aid,
      .out_aid = out_aid,
      .output = output,
      .in_aid = in_aid,
      .input = input
    };
    index_insert (automata->composition_index, &entry);
    return COMPOSED;
  }
  
}

void
automaton_system_input_exec (automata_t* automata, aid_t aid, const system_receipt_t* receipt)
{
  assert (automata != NULL);
  assert (receipt != NULL);

  /* Acquire the read lock. */
  pthread_rwlock_rdlock (&automata->lock);

  automaton_entry_t* entry = automaton_entry_for_aid (automata, aid, NULL);
  assert (entry != NULL);

  /* Prepare the buffer. */
  bid_t bid = buffer_alloc (automata->buffers, aid, sizeof (system_receipt_t));
  system_receipt_t* r = buffer_write_ptr (automata->buffers, aid, bid);
  *r = *receipt;
  
  buffer_incref (automata->buffers, aid, bid);
  set_current_aid (automata, aid);
  pthread_mutex_lock (entry->lock);
  
  entry->system_input (entry->state, bid);
  
  pthread_mutex_unlock (entry->lock);
  set_current_aid (automata, -1);
  buffer_decref (automata->buffers, aid, bid);
  
  /* Release the read lock. */
  pthread_rwlock_unlock (&automata->lock);
}

int
automaton_system_output_exec (automata_t* automata, aid_t aid, system_order_t* order)
{
  assert (automata != NULL);
  assert (order != NULL);

  int retval = BAD_ORDER;

  /* Acquire the read lock. */
  pthread_rwlock_rdlock (&automata->lock);

  automaton_entry_t* entry = automaton_entry_for_aid (automata, aid, NULL);
  assert (entry != NULL);

  /* Execute. */
  set_current_aid (automata, aid);
  pthread_mutex_lock (entry->lock);
  bid_t bid = entry->system_output (entry->state);
  pthread_mutex_unlock (entry->lock);
  set_current_aid (automata, -1);
  
  if (bid != -1) {
    buffer_incref (automata->buffers, aid, bid);
    
    if (buffer_size (automata->buffers, aid, bid) == sizeof (system_order_t)) {
      *order = *(system_order_t*)buffer_read_ptr (automata->buffers, aid, bid);
      retval = GOOD_ORDER;
    }
    else {
      retval = BAD_ORDER;
    }
    
    buffer_decref (automata->buffers, aid, bid);
  }
  else {
    /* No bid. */
    retval = NO_ORDER;
  }

  /* Release the read lock. */
  pthread_rwlock_unlock (&automata->lock);

  return retval;
}

typedef struct {
  automata_t* automata;
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
    buffer_change_owner (arg->automata->buffers, composition_entry->in_aid, arg->bid);
    composition_entry->input (in_entry->state, arg->bid);
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
automaton_output_exec (automata_t* automata, aid_t out_aid, output_t output)
{
  assert (automata != NULL);

  /* Acquire the read lock. */
  pthread_rwlock_rdlock (&automata->lock);

  automaton_entry_t* out_entry = automaton_entry_for_aid (automata, out_aid, NULL);
  assert (out_entry != NULL);

  output_arg_t arg = {
    .automata = automata,
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
  bid_t bid = output (out_entry->state);
  if (bid != -1) {
    /* Increment the reference count so the following decref will destroy the buffer if no
     * input references the buffer.
     */
    buffer_incref (automata->buffers, out_aid, bid);
      
    arg.bid = bid;
    /* Execute the inputs. */
    index_for_each (automata->composition_index,
		    index_begin (automata->composition_index),
		    index_end (automata->composition_index),
		    output_exec,
		    &arg);

    buffer_change_owner (automata->buffers, -1, bid);
            
    buffer_decref (automata->buffers, out_aid, bid);
  }
    
  set_current_aid (automata, -1);

  arg.output_done = false;
  /* Unlock. */
  index_for_each (automata->composition_index,
		  index_begin (automata->composition_index),
		  index_end (automata->composition_index),
		  output_unlock,
		  &arg);

  /* Release the read lock. */
  pthread_rwlock_unlock (&automata->lock);
}

void
automaton_internal_exec (automata_t* automata, aid_t aid, internal_t internal)
{
  assert (automata != NULL);

  /* Acquire the read lock. */
  pthread_rwlock_rdlock (&automata->lock);

  /* Look up the automaton. */
  automaton_entry_t* entry = automaton_entry_for_aid (automata, aid, NULL);
  assert (entry != NULL);

  /* Set the current automaton. */
  set_current_aid (automata, aid);
  
  /* Lock the automaton. */
  pthread_mutex_lock (entry->lock);
  
  /* Execute. */
  internal (entry->state);
  
  /* Unlock the automaton. */
  pthread_mutex_unlock (entry->lock);
  
  /* Clear the current automaton. */
  set_current_aid (automata, -1);

  /* Release the read lock. */
  pthread_rwlock_unlock (&automata->lock);
}

aid_t
automaton_get_current_aid (automata_t* automata)
{
  assert (automata != NULL);

  return (aid_t)pthread_getspecific (automata->current_aid);
}

bool
automaton_output_exists (automata_t* automata, aid_t aid, output_t output)
{
  assert (automata != NULL);

  return output_entry_for_aid_output (automata, aid, output) != NULL;
}

bool
automaton_internal_exists (automata_t* automata, aid_t aid, internal_t internal)
{
  assert (automata != NULL);
  
  return internal_entry_for_aid_internal (automata, aid, internal) != NULL;
}

bid_t
automaton_buffer_alloc (automata_t* automata, size_t size)
{
  assert (automata != NULL);
  return buffer_alloc (automata->buffers, automaton_get_current_aid (automata), size);
}

void*
automaton_buffer_write_ptr (automata_t* automata, bid_t bid)
{
  assert (automata != NULL);
  return buffer_write_ptr (automata->buffers, automaton_get_current_aid (automata), bid);
}

const void*
automaton_buffer_read_ptr (automata_t* automata, bid_t bid)
{
  assert (automata != NULL);
  return buffer_read_ptr (automata->buffers, automaton_get_current_aid (automata), bid);
}

size_t
automaton_buffer_size (automata_t* automata, bid_t bid)
{
  assert (automata != NULL);
  return buffer_size (automata->buffers, automaton_get_current_aid (automata), bid);
}

automata_t*
automata_create (void)
{
  automata_t* automata = malloc (sizeof (automata_t));

  pthread_key_create (&automata->current_aid, NULL);
  pthread_rwlock_init (&automata->lock, NULL);
  automata->next_aid = 0;
  automata->buffers = buffers_create ();

  automata->automaton_table = table_create (sizeof (automaton_entry_t));
  automata->automaton_index = index_create_list (automata->automaton_table);
  automata->automaton_edge_table = table_create (sizeof (automaton_entry_t));
  automata->automaton_edge_index = index_create_list (automata->automaton_edge_table);
  automata->input_table = table_create (sizeof (input_entry_t));
  automata->input_index = index_create_list (automata->input_table);
  automata->output_table = table_create (sizeof (output_entry_t));
  automata->output_index = index_create_list (automata->output_table);
  automata->internal_table = table_create (sizeof (internal_entry_t));
  automata->internal_index = index_create_list (automata->internal_table);

  automata->composition_table = table_create (sizeof (composition_entry_t));
  automata->composition_index = index_create_ordered_list (automata->composition_table, composition_sorted);

  return automata;
}

void
automata_destroy (automata_t* automata)
{
  assert (automata != NULL);

  table_destroy (automata->composition_table);

  table_destroy (automata->internal_table);
  table_destroy (automata->output_table);
  table_destroy (automata->input_table);
  table_destroy (automata->automaton_edge_table);
  assert (0); /* TODO: Free dynamic structures. */
  table_destroy (automata->automaton_table);

  buffers_destroy (automata->buffers);

  pthread_rwlock_destroy (&automata->lock);
  pthread_key_delete (automata->current_aid);

  free (automata);
}
