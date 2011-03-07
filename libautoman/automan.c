#include <automan.h>

#include <assert.h>
#include <stdlib.h>

#include <table.h>

/*

  Automan (Automata Manager)
  ==========================

  Automan automates the construction and maintenance of constellations of I/O automata.

  Creating new automata and composing them allows us to build complex systems from simple automata.
  To build constellations of automata, the system (ueioa) provides operations for creating and destroy automata, declaring and rescinding parameters (for dynamic compositions and unconstrained fan-in), and composing and decomposing actions.
  The caveat is that the system only accepts one operation at a time, thus, the programmer is responsible for sending a correct sequence of system orders to create the desired constellation.
  To complicate matters, the constellation itself is dynamic as automata (and their compositions) might disappear due to suicide.
  Automan was designed to automate the task of building and maintaining constellations of I/O automata.

  The key idea behind automan is a sequence of operations (create/destroy, declare/rescind, compose/decompose) that if executed serially and atomically would build the desired constellation.
  Automan maintains an equivalent sequence and works to complete all of the actions in the sequence.
  Automan maintains the state of the constellation in state variables provided by the user.
  For child automata, automan requires a pointer to a automaton id (aid_t*).
  For parameters and compositions, automan requires a pointer to a boolean variable (bool*).

  The user can extend the sequence at any time but all extensions must be consistent with the existing sequence.
  In the following example, we will assume that automan executes the sequence in order and that its progress is marked by the '.'.
  Initially, the sequence is empty.
  [.]
  The user creates two automata using pointers a1 and a2.
  [. create(a1) create(a2)]
  Automan creates the first automaton.
  [create(a1) . create(a2)]
  Extending the sequence with another create(a1) is not allowed because the old aid would be lost.
  ILLEGAL [create(a1) . create(a2) create(a1)]
  The sequence could be extended by first destroying a1 and then creating it again.
  [create(a1) . create(a2)]
  [create(a1) . create(a2) destroy(a1) create(a1)]
  [create(a1) create(a2) . destroy(a1) create(a1)]
  After automan executes the destroy(a1), the first create and destroy can be removed as the destroy cancels the create.
  This is what was meant by the term "equivalent sequence" in the preceding paragraph.
  [create(a1) create(a2) destroy(a1) . create(a1)]
  [create(a2) . create(a1)]
  [create(a2) create(a1) .]
  Operations can be reordered if they are independent, i.e., the resulting constellation would be identical.
  [create(a2) create(a1) .] === [create(a1) create(a2) .]
  Automan reorders the sequences to make progress when possible.

  Automan uses the following pointers to establish identity for certain operations:
  create(a) - aid_t* pointer
  destroy(a) - aid_t* pointer
  declare(b,p) - bool* pointer and parameter
  rescind(b) - bool* pointer
  compose(b,i) - bool* pointer and the input aid_t* pointer, input, and input parameter combination
  decompose(b) - bool* pointer
  For rescind and decompose, the pointer implies the additional information.

  The expression "closed with respect to x" to describes situations where the last operation in the sequence involving x is a destroy, rescind, or decompose.
  The variable x can either be an aid pointer, bool pointer, parameter, or input combination.
  Sequences that don't contain x are closed with respect to x.
  The expression "open with respect to x" describes situations where the last operation in the sequence involving x is a create, declare, or compose.
  The associated operation (create, declare, or rescind) is relevant when checking for openness.

  Child automata pairs (create(a)/destroy(a)) and parameter pairs (declare(b,p)/rescind(b)) are totally independent and can be interleaved in any order that conserves their relative ordering.
  For a create(a) to succeed, the sequence must be closed with respect to a.
  For a destroy(a) to succeed, the sequence must be open with respect to a.
  For a declare(b,p) to succeed, the sequence must be closed with respect to b and p.
  Note that after delcare(b,p) is appended to the sequence, b and p are linked as the openness of the sequence with respect to b is equivalent to the openness of the sequence with respect to p.
  For a rescind(b) to succeed, the sequence must be open with respect to b.

  Compositions that do not involve parameters:
    For a compose(b,i) to succeed, the sequence must be closed with respect to b and i.
    For a decompose(b) to succeed, the sequence must be open with respect to b.
  Compositions that involve output parameter (p):
    For a compose(b,i) to succeed, the sequence must be open with respect to p and closed with respect to b and i.
    (Note that the output automaton must be self.)
    For a decompose(b) to succeed, the sequence must be open with respect to b.
  Compositions that involve input parameter (p):
    For a compose(b,i) to succeed, the sequence must be open with respect p and closed with respect to b and i.
    (Note that the input automaton must be self.)
    For a decompose(b) to succeed, the sequence must be open with respect to b.

  In an effort to ease the burden on users, automan will add decompositions to preserve a valid sequence on a rescind or destroy.
  Thus, users don't have to decompose an automaton before destroying it.

  Compositions proceed when the output and input automata exist as indicated by the state variable.
  Consequently, compositions can can involve automata that are managed by someone else so long as they follow the convention that that the aid_t variable contains -1 when the automaton is not valid.

  Handlers
  --------
  
  Users can associate a synchronous handler that is called with the state given when automan is constructed and the given parameter with a create, declare, or compose.
  A corresponding destroy, rescind, or decompose inherits the handler.
  Handlers associated with create/destroy and compose/decompose may be called with out regards to a sequence operation if an externally controlled automaton is destroyed.
*/

/******************************************************************************************
 * TYPES
 ******************************************************************************************/

typedef enum {
  NORMAL,
  OUTSTANDING
} status_t;

typedef struct sequence_item_struct {
  order_type_t order_type;
  automan_handler_t handler;
  void* hparam;
  union {
    aid_t* aid_ptr;
    bool* flag_ptr;
  };
  union {
    struct {
      const descriptor_t* descriptor;
      const void* ctor_arg;
    } create;
    struct {
      void* param;
    } declare;
    struct {
      aid_t* out_aid_ptr;
      output_t output;
      void* out_param;
      aid_t* in_aid_ptr;
      input_t input;
      void* in_param;
    } compose;
  };
} sequence_item_t;

typedef struct input_item_struct {
  automan_handler_t handler;
  void* hparam;
  bool* flag_ptr;
  input_t input;
  void* in_param;
} input_item_t;

typedef struct output_item_struct {
  automan_handler_t handler;
  void* hparam;
  bool* flag_ptr;
  output_t output;
  void* out_param;
} output_item_t;

struct automan_struct {
  void* state;
  aid_t* self_ptr;
  status_t syscall_status;
  order_t last_order;
  sequence_item_t last_action;
  table_t* si_table;
  index_t* si_index;
  table_t* ii_table;
  index_t* ii_index;
  table_t* oi_table;
  index_t* oi_index;

/*   status_t proxy_status; */
/*   proxies_key_t last_proxy; */
/*   hashtable_t* proxies; */
/*   hashtable_t* dependencies; */
};

/******************************************************************************************
 * PRIVATE FUNCTIONS
 ******************************************************************************************/

static bool
aid_ptr_equal (const void* x0, void* y0)
{
  const sequence_item_t* x = x0;
  const sequence_item_t* y = y0;

  return x->aid_ptr == y->aid_ptr;
}

static bool
flag_ptr_equal (const void* x0, void* y0)
{
  const sequence_item_t* x = x0;
  const sequence_item_t* y = y0;

  return x->flag_ptr == y->flag_ptr;
}

static bool
param_equal (const void* x0, void* y0)
{
  const sequence_item_t* x = x0;
  const sequence_item_t* y = y0;

  switch (x->order_type) {
  case CREATE:
  case COMPOSE:
  case DECOMPOSE:
  case DESTROY:
    return false;
  case DECLARE:
  case RESCIND:
    return x->declare.param == y->declare.param;
  }
  /* Not reached. */
  assert (0);
  return false;
}

static bool
input_equal (const void* x0, void* y0)
{
  const sequence_item_t* x = x0;
  const sequence_item_t* y = y0;

  switch (x->order_type) {
  case CREATE:
  case DECLARE:
  case RESCIND:
  case DESTROY:
    return false;
  case COMPOSE:
  case DECOMPOSE:
    return
      x->compose.in_aid_ptr == y->compose.in_aid_ptr &&
      x->compose.input == y->compose.input &&
      x->compose.in_param == y->compose.in_param;
}
  /* Not reached. */
  assert (0);
  return false;
}

static bool
parameterized_composition (const void* x0, void* y0)
{
  const sequence_item_t* x = x0;
  const sequence_item_t* y = y0;

  switch (x->order_type) {
  case CREATE:
  case DECLARE:
  case DECOMPOSE:
  case RESCIND:
  case DESTROY:
    return false;
  case COMPOSE:
    return x->compose.out_param == y->declare.param || x->compose.in_param == y->declare.param;
  }
  /* Not reached. */
  assert (0);
  return false;
}

static bool
aid_composition (const void* x0, void* y0)
{
  const sequence_item_t* x = x0;
  const sequence_item_t* y = y0;
  
  switch (x->order_type) {
  case CREATE:
  case DECLARE:
  case DECOMPOSE:
  case RESCIND:
  case DESTROY:
    return false;
  case COMPOSE:
    return x->compose.out_aid_ptr == y->aid_ptr || x->compose.in_aid_ptr == y->aid_ptr;
  }
  /* Not reached. */
  assert (0);
  return false;
}

static bool
closed_aid_ptr (automan_t* automan,
		aid_t* aid_ptr)
{
  if (aid_ptr == automan->self_ptr) {
    /* Alway open with respect to ourself. */
    return false;
  }

  sequence_item_t key;
  key.aid_ptr = aid_ptr;
  
  sequence_item_t* sequence_item = index_rfind_value (automan->si_index,
						      index_rbegin (automan->si_index),
						      index_rend (automan->si_index),
						      aid_ptr_equal,
						      &key,
						      NULL);

  if (sequence_item != NULL) {
    /* Found one. */
    return
      sequence_item->order_type == DESTROY ||
      sequence_item->order_type == RESCIND ||
      sequence_item->order_type == DECOMPOSE;
  }
  else {
    /* Closed because it doesn't exist. */
    return true;
  }
}

static bool
open_aid_ptr_create (automan_t* automan,
		     aid_t* aid_ptr)
{
  if (aid_ptr == automan->self_ptr) {
    /* Alway open with respect to ourself. */
    return true;
  }

  sequence_item_t key;
  key.aid_ptr = aid_ptr;
  
  sequence_item_t* sequence_item = index_rfind_value (automan->si_index,
						      index_rbegin (automan->si_index),
						      index_rend (automan->si_index),
						      aid_ptr_equal,
						      &key,
						      NULL);

  if (sequence_item != NULL) {
    /* Found one. */
    return
      sequence_item->order_type == CREATE;
  }
  else {
    /* Closed because it doesn't exist. */
    return false;
  }
}

static bool
closed_flag_ptr (automan_t* automan,
		 bool* flag_ptr)
{
  if ((aid_t*)flag_ptr == automan->self_ptr) {
    /* Alway open with respect to ourself. */
    return false;
  }

  sequence_item_t key;
  key.flag_ptr = flag_ptr;
  
  sequence_item_t* sequence_item = index_rfind_value (automan->si_index,
						      index_rbegin (automan->si_index),
						      index_rend (automan->si_index),
						      flag_ptr_equal,
						      &key,
						      NULL);

  if (sequence_item != NULL) {
    /* Found one. */
    return
      sequence_item->order_type == DESTROY ||
      sequence_item->order_type == RESCIND ||
      sequence_item->order_type == DECOMPOSE;
  }
  else {
    /* Closed because it doesn't exist. */
    return true;
  }
}

static bool
open_flag_ptr_declare (automan_t* automan,
		       bool* flag_ptr)
{
  sequence_item_t key;
  key.flag_ptr = flag_ptr;
  
  sequence_item_t* sequence_item = index_rfind_value (automan->si_index,
						      index_rbegin (automan->si_index),
						      index_rend (automan->si_index),
						      flag_ptr_equal,
						      &key,
						      NULL);
  
  if (sequence_item != NULL) {
    /* Found one. */
    return
      sequence_item->order_type == DECLARE;
  }
  else {
    /* Closed because it doesn't exist. */
    return false;
  }
}

static bool
open_flag_ptr_compose (automan_t* automan,
		       bool* flag_ptr)
{
  sequence_item_t key;
  key.flag_ptr = flag_ptr;
  
  sequence_item_t* sequence_item = index_rfind_value (automan->si_index,
						      index_rbegin (automan->si_index),
						      index_rend (automan->si_index),
						      flag_ptr_equal,
						      &key,
						      NULL);
  
  if (sequence_item != NULL) {
    /* Found one. */
    return
      sequence_item->order_type == COMPOSE;
  }
  else {
    /* Closed because it doesn't exist. */
    return false;
  }
}

static bool
closed_param (automan_t* automan,
	      void* param)
{
  sequence_item_t key;
  key.declare.param = param;
  
  sequence_item_t* sequence_item = index_rfind_value (automan->si_index,
						      index_rbegin (automan->si_index),
						      index_rend (automan->si_index),
						      param_equal,
						      &key,
						      NULL);
  
  if (sequence_item != NULL) {
    /* Found one. */
    return
      sequence_item->order_type == RESCIND;
  }
  else {
    /* Closed because it doesn't exist. */
    return true;
  }
}

static bool
open_param (automan_t* automan,
	      void* param)
{
  sequence_item_t key;
  key.declare.param = param;
  
  sequence_item_t* sequence_item = index_rfind_value (automan->si_index,
						      index_rbegin (automan->si_index),
						      index_rend (automan->si_index),
						      param_equal,
						      &key,
						      NULL);
  
  if (sequence_item != NULL) {
    /* Found one. */
    return
      sequence_item->order_type == DECLARE;
  }
  else {
    /* Closed because it doesn't exist. */
    return false;
  }
}

static bool
closed_input (automan_t* automan,
	      aid_t* in_aid_ptr,
	      input_t input,
	      void* in_param)
{
  sequence_item_t key;
  key.compose.in_aid_ptr = in_aid_ptr;
  key.compose.input = input;
  key.compose.in_param = in_param;
  
  sequence_item_t* sequence_item = index_rfind_value (automan->si_index,
						      index_rbegin (automan->si_index),
						      index_rend (automan->si_index),
						      input_equal,
						      &key,
						      NULL);
  
  if (sequence_item != NULL) {
    /* Found one. */
    return
      sequence_item->order_type == DECOMPOSE;
  }
  else {
    /* Closed because it doesn't exist. */
    return true;
  }
}

static void
append_create (automan_t* automan,
	       aid_t* aid_ptr,
	       const descriptor_t* descriptor,
	       const void* ctor_arg,
	       automan_handler_t handler,
	       void* hparam)
{
  sequence_item_t sequence_item;
  sequence_item.order_type = CREATE;
  sequence_item.handler = handler;
  sequence_item.hparam = hparam;
  sequence_item.aid_ptr = aid_ptr;
  sequence_item.create.descriptor = descriptor;
  sequence_item.create.ctor_arg = ctor_arg;

  index_push_back (automan->si_index, &sequence_item);
}

static void
append_destroy (automan_t* automan,
		aid_t* aid_ptr,
		automan_handler_t handler,
		void* hparam)
{
  sequence_item_t sequence_item;
  sequence_item.order_type = DESTROY;
  sequence_item.handler = handler;
  sequence_item.hparam = hparam;
  sequence_item.aid_ptr = aid_ptr;

  index_push_back (automan->si_index, &sequence_item);
}

static void
append_declare (automan_t* automan,
		bool* flag_ptr,
		void* param,
		automan_handler_t handler,
		void* hparam)
{
  sequence_item_t sequence_item;
  sequence_item.order_type = DECLARE;
  sequence_item.handler = handler;
  sequence_item.hparam = hparam;
  sequence_item.flag_ptr = flag_ptr;
  sequence_item.declare.param = param;
  
  index_push_back (automan->si_index, &sequence_item);
}

static void
append_rescind (automan_t* automan,
		bool* flag_ptr,
		void* param,
		automan_handler_t handler,
		void* hparam)
{
  sequence_item_t sequence_item;
  sequence_item.order_type = RESCIND;
  sequence_item.handler = handler;
  sequence_item.hparam = hparam;
  sequence_item.flag_ptr = flag_ptr;
  sequence_item.declare.param = param;
  
  index_push_back (automan->si_index, &sequence_item);
}

static void
append_compose (automan_t* automan,
		bool* flag_ptr,
		aid_t* out_aid_ptr,
		output_t output,
		void* out_param,
		aid_t* in_aid_ptr,
		input_t input,
		void* in_param,
		automan_handler_t handler,
		void* hparam)
{
  sequence_item_t sequence_item;
  sequence_item.order_type = COMPOSE;
  sequence_item.handler = handler;
  sequence_item.hparam = hparam;
  sequence_item.flag_ptr = flag_ptr;
  sequence_item.compose.out_aid_ptr = out_aid_ptr;
  sequence_item.compose.output = output;
  sequence_item.compose.out_param = out_param;
  sequence_item.compose.in_aid_ptr = in_aid_ptr;
  sequence_item.compose.input = input;
  sequence_item.compose.in_param = in_param;

  index_push_back (automan->si_index, &sequence_item);
}

static void
append_decompose (automan_t* automan,
		  bool* flag_ptr,
		  aid_t* out_aid_ptr,
		  output_t output,
		  void* out_param,
		  aid_t* in_aid_ptr,
		  input_t input,
		  void* in_param,
		  automan_handler_t handler,
		  void* hparam)
{
  sequence_item_t sequence_item;
  sequence_item.order_type = DECOMPOSE;
  sequence_item.handler = handler;
  sequence_item.hparam = hparam;
  sequence_item.flag_ptr = flag_ptr;
  sequence_item.compose.out_aid_ptr = out_aid_ptr;
  sequence_item.compose.output = output;
  sequence_item.compose.out_param = out_param;
  sequence_item.compose.in_aid_ptr = in_aid_ptr;
  sequence_item.compose.input = input;
  sequence_item.compose.in_param = in_param;

  index_push_back (automan->si_index, &sequence_item);
}

static void
append_input (automan_t* automan,
	      bool* flag_ptr,
	      input_t input,
	      void* in_param,
	      automan_handler_t handler,
	      void* hparam)
{
  input_item_t input_item;
  input_item.handler = handler;
  input_item.hparam = hparam;
  input_item.flag_ptr = flag_ptr;
  input_item.input = input;
  input_item.in_param = in_param;

  index_push_back (automan->ii_index, &input_item);
}

static void
append_output (automan_t* automan,
	      bool* flag_ptr,
	      output_t output,
	      void* out_param,
	      automan_handler_t handler,
	      void* hparam)
{
  output_item_t output_item;
  output_item.handler = handler;
  output_item.hparam = hparam;
  output_item.flag_ptr = flag_ptr;
  output_item.output = output;
  output_item.out_param = out_param;

  index_push_back (automan->oi_index, &output_item);
}

static bool
param_declared (automan_t* automan,
		void* param)
{
  if (param == NULL) {
    return true;
  }
  
  sequence_item_t key;
  key.declare.param = param;
  
  sequence_item_t* sequence_item = index_find_value (automan->si_index,
						     index_begin (automan->si_index),
						     index_end (automan->si_index),
						     param_equal,
						     &key,
						     NULL);
  
  if (sequence_item != NULL) {
    return *sequence_item->flag_ptr;
  }
  else {
    /* Doesn't exist. */
    return false;
  }
}

static sequence_item_t*
find_flag_ptr (automan_t* automan,
	       bool* flag_ptr,
	       iterator_t begin,
	       iterator_t* iterator)
{
  sequence_item_t key;
  key.flag_ptr = flag_ptr;
  
  /* Find the compose. */
  return index_find_value (automan->si_index,
			   begin,
			   index_end (automan->si_index),
			   flag_ptr_equal,
			   &key,
			   iterator);
}

static sequence_item_t*
rfind_flag_ptr (automan_t* automan,
		bool* flag_ptr,
		riterator_t* riterator)
{
  sequence_item_t key;
  key.flag_ptr = flag_ptr;
  
  /* Find the compose. */
  return index_rfind_value (automan->si_index,
			    index_rbegin (automan->si_index),
			    index_rend (automan->si_index),
			    flag_ptr_equal,
			    &key,
			    riterator);
}

static sequence_item_t*
rfind_aid_ptr (automan_t* automan,
	       aid_t* aid_ptr,
	       riterator_t* riterator)
{
  sequence_item_t key;
  key.aid_ptr = aid_ptr;
  
  /* Find the compose. */
  return index_rfind_value (automan->si_index,
			    index_rbegin (automan->si_index),
			    index_rend (automan->si_index),
			    aid_ptr_equal,
			    &key,
			    riterator);
}

static bool
process_sequence_items (automan_t* automan)
{
  assert (automan != NULL);

  iterator_t iterator;
  for (iterator = index_begin (automan->si_index);
       iterator_ne (iterator, index_end (automan->si_index));
       iterator = index_advance (automan->si_index, iterator)) {
    sequence_item_t* sequence_item = index_value (automan->si_index, iterator);
    switch (sequence_item->order_type) {
    case CREATE:
      {
	if (*sequence_item->aid_ptr == -1) {
	  /* Create an automaton. */
	  order_create_init (&automan->last_order,
			     sequence_item->create.descriptor,
			     sequence_item->create.ctor_arg);
	  automan->last_action = *sequence_item;
	  return true;
	}
      }
      break;
    case DECLARE:
      {
	if (*sequence_item->flag_ptr == false) {
	  /* We need to declare a parameter. */
	  order_declare_init (&automan->last_order,
			      sequence_item->declare.param);
	  automan->last_action = *sequence_item;
	  return true;
	}
      }
      break;
    case COMPOSE:
      {
      	if (*sequence_item->flag_ptr == false &&
      	    *sequence_item->compose.out_aid_ptr != -1 &&
      	    param_declared (automan, sequence_item->compose.out_param) &&
      	    *sequence_item->compose.in_aid_ptr != -1 &&
      	    param_declared (automan, sequence_item->compose.in_param)) {
      	  /* We need to compose. */
      	  order_compose_init (&automan->last_order,
      			      *sequence_item->compose.out_aid_ptr,
      			      sequence_item->compose.output,
      			      sequence_item->compose.out_param,
      			      *sequence_item->compose.in_aid_ptr,
      			      sequence_item->compose.input,
      			      sequence_item->compose.in_param);
      	  automan->last_action = *sequence_item;
      	  return true;
      	}
      }
      break;
    case DECOMPOSE:
      {
	if (*sequence_item->flag_ptr == true) {
	  /* We need to decompose. */
	  order_decompose_init (&automan->last_order,
				*sequence_item->compose.out_aid_ptr,
				sequence_item->compose.output,
				sequence_item->compose.out_param,
				*sequence_item->compose.in_aid_ptr,
				sequence_item->compose.input,
				sequence_item->compose.in_param);
	  automan->last_action = *sequence_item;
	  return true;
	}
      }
      break;
    case RESCIND:
      {
	if (*sequence_item->flag_ptr == true) {
	  /* We need to rescind a parameter. */
	  order_rescind_init (&automan->last_order,
			      sequence_item->declare.param);
	  automan->last_action = *sequence_item;
	  return true;
	}
      }
      break;
    case DESTROY:
      {
	if (*sequence_item->aid_ptr != -1) {
	  /* Create an automaton. */
	  order_destroy_init (&automan->last_order,
			      *sequence_item->aid_ptr);
	  automan->last_action = *sequence_item;
	  return true;
	}
      }
      break;
    }
  }

  return false;
}

static bool
ii_flag_ptr_equal (const void* x0,
		   void* y0)
{
  const input_item_t* x = x0;
  const input_item_t* y = y0;

  return x->flag_ptr == y->flag_ptr;
}

static input_item_t*
find_ii_flag_ptr (automan_t* automan,
		  bool* flag_ptr)
{
  input_item_t key;
  key.flag_ptr = flag_ptr;
  
  return index_find_value (automan->ii_index,
			   index_begin (automan->ii_index),
			   index_end (automan->ii_index),
			   ii_flag_ptr_equal,
			   &key,
			   NULL);
}

static bool
ii_input_equal (const void* x0,
		void* y0)
{
  const input_item_t* x = x0;
  const input_item_t* y = y0;
  
  return
    x->input == y->input &&
    x->in_param == y->in_param;
}

static input_item_t*
find_ii_input (automan_t* automan,
	       input_t input,
	       void* in_param)
{
  input_item_t key;
  key.input = input;
  key.in_param = in_param;
  
  return index_find_value (automan->ii_index,
			   index_begin (automan->ii_index),
			   index_end (automan->ii_index),
			   ii_input_equal,
			   &key,
			   NULL);
}

static bool
oi_flag_ptr_equal (const void* x0,
		   void* y0)
{
  const output_item_t* x = x0;
  const output_item_t* y = y0;

  return x->flag_ptr == y->flag_ptr;
}

static output_item_t*
find_oi_flag_ptr (automan_t* automan,
		  bool* flag_ptr)
{
  output_item_t key;
  key.flag_ptr = flag_ptr;
  
  return index_find_value (automan->oi_index,
			   index_begin (automan->oi_index),
			   index_end (automan->oi_index),
			   oi_flag_ptr_equal,
			   &key,
			   NULL);
}

static bool
oi_output_equal (const void* x0,
		void* y0)
{
  const output_item_t* x = x0;
  const output_item_t* y = y0;
  
  return
    x->output == y->output &&
    x->out_param == y->out_param;
}

static output_item_t*
find_oi_output (automan_t* automan,
	       output_t output,
	       void* out_param)
{
  output_item_t key;
  key.output = output;
  key.out_param = out_param;
  
  return index_find_value (automan->oi_index,
			   index_begin (automan->oi_index),
			   index_end (automan->oi_index),
			   oi_output_equal,
			   &key,
			   NULL);
}

/******************************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************************/

automan_t*
automan_creat (void* state, aid_t* self_ptr)
{
  assert (state != NULL);
  assert (self_ptr != NULL);

  automan_t* automan = malloc (sizeof (automan_t));

  automan->state = state;
  automan->self_ptr = self_ptr;
  *self_ptr = -1;
  automan->syscall_status = NORMAL;
  automan->si_table = table_create (sizeof (sequence_item_t));
  automan->si_index = index_create_list (automan->si_table);
  automan->ii_table = table_create (sizeof (input_item_t));
  automan->ii_index = index_create_list (automan->ii_table);
  automan->oi_table = table_create (sizeof (output_item_t));
  automan->oi_index = index_create_list (automan->oi_table);


  /* automan->proxy_status = NORMAL; */
  /* automan->create = hashtable_create (sizeof (create_key_t), create_key_equal, create_key_hash); */
  /* automan->proxies = hashtable_create (sizeof (proxies_key_t), proxies_key_equal, proxies_key_hash); */
  /* automan->dependencies = hashtable_create (sizeof (dependencies_key_t), dependencies_key_equal, dependencies_key_hash); */
  /* automan->outputs = hashtable_create (sizeof (outputs_key_t), outputs_key_equal, outputs_key_hash); */

  return automan;
}

void
automan_apply (automan_t* automan,
	       const receipt_t* receipt)
{
  assert (automan != NULL);
  assert (receipt != NULL);

  bool something_changed = false;

  switch (receipt->type) {
  case BAD_ORDER:
    /* We should never give a bad order. */
    assert (0);
    break;
  case SELF_CREATED:
    {
      *automan->self_ptr = receipt->self_created.self;
      something_changed = true;
    }
    break;
  case CHILD_CREATED:
    {
      assert (automan->syscall_status == OUTSTANDING && automan->last_order.type == CREATE);
      *automan->last_action.aid_ptr = receipt->child_created.child;
      if (automan->last_action.handler != NULL) {
	automan->last_action.handler (automan->state, automan->last_action.hparam, receipt->type);
      }
      something_changed = true;
      automan->syscall_status = NORMAL;
    }
    break;
  case BAD_DESCRIPTOR:
    /* Currently, the only way to have a bad descriptor is to create an order with a NULL descriptor.
       We don't allow this.*/
    assert (0);
    break;
  case DECLARED:
    {
      assert (automan->syscall_status == OUTSTANDING && automan->last_order.type == DECLARE);
      *automan->last_action.flag_ptr = true;
      if (automan->last_action.handler != NULL) {
	automan->last_action.handler (automan->state, automan->last_action.hparam, receipt->type);
      }
      something_changed = true;
      automan->syscall_status = NORMAL;
    }
    break;
  case OUTPUT_DNE:
  case INPUT_DNE:
  case OUTPUT_UNAVAILABLE:
  case INPUT_UNAVAILABLE:
    {
      assert (automan->syscall_status == OUTSTANDING && automan->last_order.type == COMPOSE);
      iterator_t pos;
      /* Find the composition. */
      find_flag_ptr (automan,
		     automan->last_action.flag_ptr,
		     index_begin (automan->si_index),
		     &pos);
      /* Erase the composition. */
      pos = index_erase (automan->si_index, pos);
      /* Find the decomposition. */
      sequence_item_t* decompose = find_flag_ptr (automan,
						  automan->last_action.flag_ptr,
						  index_begin (automan->si_index),
						  &pos);
      /* Erase it if it exists. */
      if (decompose != NULL) {
	index_erase (automan->si_index, pos);
      }
      if (automan->last_action.handler != NULL) {
	automan->last_action.handler (automan->state, automan->last_action.hparam, receipt->type);
      }
      something_changed = true;
      automan->syscall_status = NORMAL;
    }
    break;
  case COMPOSED:
    {
      assert (automan->syscall_status == OUTSTANDING && automan->last_order.type == COMPOSE);
      *automan->last_action.flag_ptr = true;
      if (automan->last_action.handler != NULL) {
	automan->last_action.handler (automan->state, automan->last_action.hparam, receipt->type);
      }
      something_changed = true;
      automan->syscall_status = NORMAL;
    }
    break;
  case INPUT_COMPOSED:
    {
      /* Look up the input. */
      input_item_t* input_item = find_ii_input (automan,
						receipt->input_composed.input,
						receipt->input_composed.in_param);
      if (input_item != NULL) {
	/* We are tracking.  Set the flag to true. */
	*input_item->flag_ptr = true;
	/* Call the handler. */
	if (input_item->handler != NULL) {
	  input_item->handler (automan->state, input_item->hparam, receipt->type);
	}
      }
    }
    break;
  case OUTPUT_COMPOSED:
    {
      /* Look up the output. */
      output_item_t* output_item = find_oi_output (automan,
						   receipt->output_composed.output,
						   receipt->output_composed.out_param);
      if (output_item != NULL) {
	/* We are tracking.  Set the flag to true. */
	*output_item->flag_ptr = true;
	/* Call the handler. */
	if (output_item->handler != NULL) {
	  output_item->handler (automan->state, output_item->hparam, receipt->type);
	}
      }
    }
    break;
  case NOT_COMPOSED:
    /* TODO */
    assert (0);
    break;
  case DECOMPOSED:
    {
      if (automan->syscall_status == OUTSTANDING &&
	  automan->last_order.type == DECOMPOSE &&
	  *automan->last_action.compose.in_aid_ptr == receipt->decomposed.in_aid &&
	  automan->last_action.compose.input == receipt->decomposed.input &&
	  automan->last_action.compose.in_param == receipt->decomposed.in_param) {
	/* We asked for this one. */
	
	iterator_t pos;
	
	/* Remove the composition. */
	automan->last_action.order_type = COMPOSE;
	pos = index_find (automan->si_index,
			  index_begin (automan->si_index),
			  index_end (automan->si_index),
			  flag_ptr_equal,
			  &automan->last_action);
	assert (iterator_ne (pos, index_end (automan->si_index)));
	pos = index_erase (automan->si_index, pos);
	
	/* Remove the decomposition. */
	automan->last_action.order_type = DECOMPOSE;
	pos = index_find (automan->si_index,
			  pos,
			  index_end (automan->si_index),
			  flag_ptr_equal,
			  &automan->last_action);
	assert (iterator_ne (pos, index_end (automan->si_index)));
	index_erase (automan->si_index, pos);
	
	/* Set the flag. */
	*automan->last_action.flag_ptr = false;
	
	if (automan->last_action.handler != NULL) {
	  automan->last_action.handler (automan->state, automan->last_action.hparam, receipt->type);
	}
	something_changed = true;
      automan->syscall_status = NORMAL;
      }
      else {
	/* TODO: Spurious decomposition. */
	assert (0);
      }
    }
    break;
  case INPUT_DECOMPOSED:
    /* TODO */
    assert (0);
    break;
  case OUTPUT_DECOMPOSED:
    /* TODO */
    assert (0);
    break;
  case RESCINDED:
    {
      assert (automan->syscall_status == OUTSTANDING && automan->last_order.type == RESCIND);
      *automan->last_action.flag_ptr = false;
      if (automan->last_action.handler != NULL) {
	automan->last_action.handler (automan->state, automan->last_action.hparam, receipt->type);
      }
      something_changed = true;
      automan->syscall_status = NORMAL;
    }
    break;
  case AUTOMATON_DNE:
    /* TODO */
    assert (0);
    break;
  case NOT_OWNER:
    /* TODO */
    assert (0);
    break;
  case CHILD_DESTROYED:
    {
      if (automan->syscall_status == OUTSTANDING &&
	  automan->last_order.type == DESTROY &&
	  *automan->last_action.aid_ptr == receipt->child_destroyed.child) {
	*automan->last_action.aid_ptr = -1;
	if (automan->last_action.handler != NULL) {
	  automan->last_action.handler (automan->state, automan->last_action.hparam, receipt->type);
	}
	something_changed = true;
	automan->syscall_status = NORMAL;
      }
      else {
	/* TODO: Spurious child death. */
	assert (0);
      }
    }
    break;
  }

  /* /\* Fire the proxy. *\/ */
  /* proxy_fire (automan); */

  if ((automan->syscall_status == NORMAL) && something_changed) {
    assert (schedule_system_output () == 0);
  }
}

bid_t
automan_action (automan_t* automan)
{
  assert (automan != NULL);

/*   proxy_fire (automan); */
/*   dependencies_fire (automan); */

  switch (automan->syscall_status) {
  case NORMAL:
    {
      if (process_sequence_items (automan)) {
	bid_t bid = buffer_alloc (sizeof (order_t));
	order_t* order = buffer_write_ptr (bid);
	*order = automan->last_order;
	automan->syscall_status = OUTSTANDING;
	return bid;
      }
      else {
	return -1;
      }
    }
    break;
  case OUTSTANDING:
    return -1;
    break;
  }

  /* Not reached. */
  assert (0);
  return -1;
}

int
automan_create (automan_t* automan,
		aid_t* aid_ptr,
		const descriptor_t* descriptor,
		const void* ctor_arg,
		automan_handler_t handler,
		void* hparam)
{
  assert (automan != NULL);
  assert (aid_ptr != NULL);
  assert (descriptor != NULL);
  assert (hparam == NULL || handler != NULL);

  if (!closed_aid_ptr (automan, aid_ptr)) {
    /* Duplicate create. */
    return -1;
  }
  
  /* Clear the automaton. */
  *aid_ptr = -1;

  /* Add an action item. */
  append_create (automan,
		 aid_ptr,
		 descriptor,
		 ctor_arg,
		 handler,
		 hparam);

  return 0;
}

int
automan_declare (automan_t* automan,
		 bool* flag_ptr,
		 void* param,
		 automan_handler_t handler,
		 void* hparam)
{
  assert (automan != NULL);
  assert (flag_ptr != NULL);
  assert (param != NULL);
  assert (hparam == NULL || handler != NULL);

  if (!closed_flag_ptr (automan, flag_ptr) ||
      !closed_param (automan, param)) {
    /* Duplicate declare. */
    return -1;
  }

  /* Clear the flag. */
  *flag_ptr = false;
  
  /* Add an action item. */
  append_declare (automan,
		  flag_ptr,
		  param,
		  handler,
		  hparam);
  
  return 0;
}

int
automan_compose (automan_t* automan,
		 bool* flag_ptr,
		 aid_t* out_aid_ptr,
		 output_t output,
		 void* out_param,
		 aid_t* in_aid_ptr,
		 input_t input,
		 void* in_param,
		 automan_handler_t handler,
		 void* hparam)
{
  assert (automan != NULL);
  assert (flag_ptr != NULL);
  assert (out_aid_ptr != NULL);
  assert (output != NULL);
  assert (in_aid_ptr != NULL);
  assert (input != NULL);
  assert (out_param == NULL || in_param == NULL);
  assert (out_param == NULL || automan->self_ptr == out_aid_ptr);
  assert (in_param == NULL || automan->self_ptr == in_aid_ptr);

  assert (hparam == NULL || handler != NULL);
  
  /* Check for parameter management. */
  if (out_param != NULL &&
      !open_param (automan, out_param)) {
    return -1;
  }
  else if (in_param != NULL &&
	   !open_param (automan, in_param)) {
    return -1;
  }

  if (!closed_flag_ptr (automan, flag_ptr) ||
      !closed_input (automan, in_aid_ptr, input, in_param)) {
    /* Duplicate composition. */
    return -1;
  }
  
  /* Clear the flag. */
  *flag_ptr = false;
  
  append_compose (automan,
		  flag_ptr,
		  out_aid_ptr,
		  output,
		  out_param,
		  in_aid_ptr,
		  input,
		  in_param,
		  handler,
		  hparam);
  
  return 0;
}

int
automan_decompose (automan_t* automan,
		   bool* flag_ptr)
{
  assert (automan != NULL);
  assert (flag_ptr != NULL);

  if (!open_flag_ptr_compose (automan, flag_ptr)) {
    /* Duplicate decompose. */
    return -1;
  }
  
  /* Find the compose. */
  sequence_item_t* sequence_item = rfind_flag_ptr (automan, flag_ptr, NULL);
  
  append_decompose (automan,
		    sequence_item->flag_ptr,
		    sequence_item->compose.out_aid_ptr,
		    sequence_item->compose.output,
		    sequence_item->compose.out_param,
		    sequence_item->compose.in_aid_ptr,
		    sequence_item->compose.input,
		    sequence_item->compose.in_param,
		    sequence_item->handler,
		    sequence_item->hparam);
  
  return 0;
}

static sequence_item_t*
find_composition_param (automan_t* automan,
			bool* flag_ptr,
			void* param,
			iterator_t begin,
			iterator_t* pos)
{
  sequence_item_t key;
  key.flag_ptr = flag_ptr;
  key.declare.param = param;

  return index_find_value (automan->si_index,
			   begin,
			   index_end (automan->si_index),
			   parameterized_composition,
			   &key,
			   pos);
}

static sequence_item_t*
find_composition_aid (automan_t* automan,
		      aid_t* aid_ptr,
		      iterator_t begin,
		      iterator_t* pos)
{
  sequence_item_t key;
  key.aid_ptr = aid_ptr;

  return index_find_value (automan->si_index,
			   begin,
			   index_end (automan->si_index),
			   aid_composition,
			   &key,
			   pos);
}

int
automan_rescind (automan_t* automan,
		 bool* flag_ptr)
{
  assert (automan != NULL);
  assert (flag_ptr != NULL);

  if (!open_flag_ptr_declare (automan, flag_ptr)) {
    /* Duplicate rescind. */
    return -1;
  }
  
  /* Find the declare. */
  riterator_t declare_pos;
  sequence_item_t* sequence_item = rfind_flag_ptr (automan,
						   flag_ptr,
						   &declare_pos);

  /* Every composition that uses the parameter must be paired with a decomposition. */
  iterator_t compose_pos = riterator_reverse (automan->si_index, declare_pos);
  for (;;) {
    /* Find a composition using this parameter. */
    sequence_item_t* composition = find_composition_param (automan,
							   flag_ptr,
							   sequence_item->declare.param,
							   compose_pos,
							   &compose_pos);

    if (composition != NULL) {
      /* Found a composition using this parameter. Look for the decomposition. */
      sequence_item_t* decomposition = find_flag_ptr (automan,
						      composition->flag_ptr,
						      compose_pos,
						      NULL);
      if (decomposition == NULL) {
      	/* No corresponding decomposition so add one. */
	append_decompose (automan,
			  composition->flag_ptr,
			  composition->compose.out_aid_ptr,
			  composition->compose.output,
			  composition->compose.out_param,
			  composition->compose.in_aid_ptr,
			  composition->compose.input,
			  composition->compose.in_param,
			  composition->handler,
			  composition->hparam);
	/* Technically, we don't need to add the decompositions.
	   We could just rescind the parameter and let the system tell us about all of the decompositions.
	   However, it would violate the invariant that the user could extend the sequence by leaving unpaired compositions.
	*/
      }
      
      /* Advance. */
      compose_pos = index_advance (automan->si_index, compose_pos);
    }
    else {
      /* No subsequent compositions use this paramter.  Done. */
      break;
    }
  }
  
  /* Add an action item. */
  append_rescind (automan,
		  sequence_item->flag_ptr,
		  sequence_item->declare.param,
		  sequence_item->handler,
		  sequence_item->hparam);
  
  return 0;
}

int
automan_destroy (automan_t* automan,
		 aid_t* aid_ptr)
{
  assert (automan != NULL);
  assert (aid_ptr != NULL);

  if (!open_aid_ptr_create (automan, aid_ptr)) {
    /* Duplicate destroy. */
    return -1;
  }

  /* sequence_item_t key; */
  /* key.aid_ptr = aid_ptr; */
  
  /* Find the create. */
  riterator_t create_pos;
  sequence_item_t* sequence_item = rfind_aid_ptr (automan,
						  aid_ptr,
						  &create_pos);

  /* Every composition that uses this aid must be paired with a decomposition. */
  iterator_t compose_pos = riterator_reverse (automan->si_index, create_pos);
  for (;;) {
    /* Find a composition using this aid. */
    sequence_item_t* composition = find_composition_aid (automan,
							 aid_ptr,
							 compose_pos,
							 &compose_pos);
    if (composition != NULL) {
      /* Found a composition using this parameter. Look for the decomposition. */
      sequence_item_t* decomposition = find_flag_ptr (automan,
						      composition->flag_ptr,
						      compose_pos,
						      NULL);
      if (decomposition == NULL) {
      	/* No corresponding decomposition so add one. */
  	append_decompose (automan,
  			  composition->flag_ptr,
  			  composition->compose.out_aid_ptr,
  			  composition->compose.output,
  			  composition->compose.out_param,
  			  composition->compose.in_aid_ptr,
  			  composition->compose.input,
  			  composition->compose.in_param,
  			  composition->handler,
  			  composition->hparam);
  	/* Technically, we don't need to add the decompositions.
  	   We could just destroy the automaton and let the system tell us about all of the decompositions.
  	   However, it would violate the invariant that the user could extend the sequence by leaving unpaired compositions.
  	*/
      }
      
      /* Advance. */
      compose_pos = index_advance (automan->si_index, compose_pos);
    }
    else {
      /* No subsequent compositions use this paramter.  Done. */
      break;
    }
  }
  
  /* Add an action item. */
  append_destroy (automan,
		  sequence_item->aid_ptr,
		  sequence_item->handler,
		  sequence_item->hparam);

  return 0;
}

int
automan_input_add (automan_t* automan,
		   bool* flag_ptr,
		   input_t input,
		   void* in_param,
		   automan_handler_t handler,
		   void* hparam)
{
  assert (automan != NULL);
  assert (flag_ptr != NULL);
  assert (input != NULL);
  assert (hparam == NULL || handler != NULL);

  if (find_ii_flag_ptr (automan,
			flag_ptr) != NULL ||
      find_ii_input (automan,
		     input,
		     in_param) != NULL) {
    /* Already tracking. */
    return -1;
  }

  append_input (automan,
		flag_ptr,
		input,
		in_param,
		handler,
		hparam);

  return 0;
}

int
automan_output_add (automan_t* automan,
		    bool* flag_ptr,
		    output_t output,
		    void* out_param,
		    automan_handler_t handler,
		    void* hparam)
{
  assert (automan != NULL);
  assert (flag_ptr != NULL);
  assert (output != NULL);
  assert (hparam == NULL || handler != NULL);
  
  if (find_oi_flag_ptr (automan,
			flag_ptr) != NULL ||
      find_oi_output (automan,
		      output,
		      out_param) != NULL) {
    /* Already tracking. */
    return -1;
  }
  
  append_output (automan,
		 flag_ptr,
		 output,
		 out_param,
		 handler,
		 hparam);

  return 0;
}







/* void */
/* automan_proxy_add (automan_t* automan, */
/* 		   aid_t* proxy_aid_ptr, */
/* 		   aid_t* source_aid_ptr, */
/* 		   input_t source_free_input, */
/* 		   input_t callback, */
/* 		   bid_t bid) */
/* { */
/*   assert (automan != NULL); */
/*   assert (proxy_aid_ptr != NULL); */
/*   assert (source_aid_ptr != NULL); */
/*   assert (source_free_input != NULL); */
/*   assert (callback != NULL); */
  
/*   proxies_key_t key = { */
/*     .proxy_aid_ptr = proxy_aid_ptr, */
/*     .source_aid_ptr = source_aid_ptr, */
/*     .source_free_input = source_free_input, */
/*     .callback = callback, */
/*     .bid = bid, */
/*   }; */
/*   assert (!hashtable_contains_key (automan->proxies, &key)); */
/*   hashtable_insert (automan->proxies, &key); */

/*   /\* Clear the automaton. *\/ */
/*   *proxy_aid_ptr = -1; */

/*   /\* Increment the reference count. *\/ */
/*   if (bid != -1) { */
/*     buffer_incref (bid); */
/*   } */
/* } */

/* void */
/* automan_dependency_add (automan_t* automan, */
/* 			aid_t* child, */
/* 			aid_t* dependent, */
/* 			input_t free_input, */
/* 			bid_t bid) */
/* { */
/*   assert (automan != NULL); */
/*   assert (child != NULL); */
/*   assert (dependent != NULL); */
/*   assert (free_input != NULL); */
/*   assert (bid != -1); */

/*   dependency_state_t state; */
/*   if (*child != -1) { */
/*     state = DEP_FINISHED; */
/*   } */
/*   else if (*dependent != -1) { */
/*     state = DEP_WAITING; */
/*   } */
/*   else { */
/*     state = DEP_START; */
/*   } */

/*   dependencies_key_t key = { */
/*     .child = child, */
/*     .dependent = dependent, */
/*     .free_input = free_input, */
/*     .bid = bid, */
/*     .state = state, */
/*   }; */
/*   assert (!hashtable_contains_key (automan->dependencies, &key)); */
/*   hashtable_insert (automan->dependencies, &key); */

/*   buffer_incref (bid); */
/* } */

/* void */
/* automan_dependency_remove (automan_t* automan, */
/* 			   aid_t* child, */
/* 			   aid_t* dependent) */
/* { */
/*   assert (automan != NULL); */
/*   assert (child != NULL); */
/*   assert (dependent != NULL); */

/*   dependencies_key_t key = { */
/*     .child = child, */
/*     .dependent = dependent, */
/*   }; */
/*   dependencies_key_t* k = hashtable_lookup (automan->dependencies, &key); */
/*   assert (k != NULL); */
/*   buffer_decref (k->bid); */
/*   hashtable_remove (automan->dependencies, &key); */
/* } */

/* static void */
/* proxy_fire (automan_t* automan) */
/* { */
/*   assert (*automan->self_ptr != -1); */
/*   if (automan->proxy_status == NORMAL) { */
/*     size_t idx; */
/*     /\* Go through the proxies. *\/ */
/*     for (idx = hashtable_first (automan->proxies); */
/* 	 idx != hashtable_last (automan->proxies); */
/* 	 idx = hashtable_next (automan->proxies, idx)) { */
/*       const proxies_key_t* key = hashtable_key_at (automan->proxies, idx); */
/*       if (*key->proxy_aid_ptr == -1 && *key->source_aid_ptr != -1) { */
/*         /\* Request a proxy. *\/ */
/* 	bid_t bid = proxy_request_create (key->bid, *automan->self_ptr, key->callback); */
/* 	assert (schedule_free_input (*key->source_aid_ptr, *key->source_free_input, bid) == 0); */
/* 	automan->last_proxy = *key; */
/* 	automan->proxy_status = OUTSTANDING; */
/* 	break; */
/*       } */
/*     } */
/*   } */
/* } */

/* static void */
/* dependencies_fire (automan_t* automan) */
/* { */
/*   size_t idx; */

/*   /\* Go through the dependencies. *\/ */
/*   for (idx = hashtable_first (automan->dependencies); */
/*        idx != hashtable_last (automan->dependencies); */
/*        idx = hashtable_next (automan->dependencies, idx)) { */
/*     dependencies_key_t* key = hashtable_key_at (automan->dependencies, idx); */
/*     switch (key->state) { */
/*     case DEP_START: */
/*       { */
/* 	/\* Only one thing can happen at a time. *\/ */
/* 	if (*key->child != -1) { */
/* 	  /\* Child created before dependent.  Finished. *\/ */
/* 	  key->state = DEP_FINISHED; */
/* 	} */
/* 	else if (*key->dependent != -1) { */
/* 	  /\* Dependent created before child.  Wait. *\/ */
/* 	  key->state = DEP_WAITING; */
/* 	} */
/*       } */
/*       break; */
/*     case DEP_WAITING: */
/*       { */
/* 	if (*key->child != -1) { */
/* 	  assert (schedule_free_input (*key->dependent, key->free_input, key->bid) == 0); */
/* 	  key->state = DEP_FINISHED; */
/* 	} */
/*       } */
/*       break; */
/*     case DEP_FINISHED: */
/*       /\* Done. *\/ */
/*       break; */
/*     } */
/*   } */

/* } */

/* void */
/* automan_proxy_receive (automan_t* automan, */
/* 		       const proxy_receipt_t* proxy_receipt) */
/* { */
/*   assert (automan != NULL); */
/*   assert (proxy_receipt != NULL); */

/*   assert (automan->proxy_status == OUTSTANDING); */

/*   *automan->last_proxy.proxy_aid_ptr = proxy_receipt->proxy_aid; */
/*   automan->proxy_status = NORMAL; */

/*   proxy_fire (automan); */

/*   assert (schedule_system_output () == 0); */
/* } */

/* static bid_t */
/* proxy_request_create (bid_t bid, */
/* 		      aid_t callback_aid, */
/* 		      input_t callback_free_input) */
/* { */
/*   bid_t b = buffer_alloc (sizeof (proxy_request_t)); */
/*   proxy_request_t* proxy_request = buffer_write_ptr (b); */
/*   proxy_request->bid = bid; */
/*   if (bid != -1) { */
/*     buffer_add_child (b, bid); */
/*   } */
/*   proxy_request->callback_aid = callback_aid; */
/*   proxy_request->callback_free_input = callback_free_input; */
/*   return b; */
/* } */

/* bid_t */
/* proxy_receipt_create (aid_t proxy_aid, */
/* 		      bid_t bid) */
/* { */
/*   assert (proxy_aid != -1); */

/*   bid_t b = buffer_alloc (sizeof (proxy_receipt_t)); */
/*   proxy_receipt_t* proxy_receipt = buffer_write_ptr (b); */
/*   proxy_receipt->proxy_aid = proxy_aid; */
/*   proxy_receipt->bid = bid; */
/*   if (bid != -1) { */
/*     buffer_add_child (b, bid); */
/*   } */
/*   return b; */
/* } */

/* typedef struct proxies_key_struct { */
/*   aid_t* proxy_aid_ptr; */
/*   aid_t* source_aid_ptr; */
/*   input_t source_free_input; */
/*   input_t callback; */
/*   bid_t bid; */
/* } proxies_key_t; */

/* static bool */
/* proxies_key_equal (const void* x0, */
/* 		   const void* y0) */
/* { */
/*   const proxies_key_t* x = x0; */
/*   const proxies_key_t* y = y0; */
/*   return x->proxy_aid_ptr == y->proxy_aid_ptr; */
/* } */

/* static size_t */
/* proxies_key_hash (const void* x0) */
/* { */
/*   const proxies_key_t* x = x0; */
  
/*   return (size_t)x->proxy_aid_ptr; */
/* } */

/* typedef enum { */
/*   DEP_START, */
/*   DEP_WAITING, */
/*   DEP_FINISHED, */
/* } dependency_state_t; */

/* typedef struct { */
/*   aid_t* child; */
/*   aid_t* dependent; */
/*   input_t free_input; */
/*   bid_t bid; */
/*   dependency_state_t state; */
/* } dependencies_key_t; */

/* static bool */
/* dependencies_key_equal (const void* x0, */
/* 			const void* y0) */
/* { */
/*   const dependencies_key_t* x = x0; */
/*   const dependencies_key_t* y = y0; */
/*   return */
/*     x->child == y->child && */
/*     x->dependent == y->dependent; */
/* } */

/* static size_t */
/* dependencies_key_hash (const void* x0) */
/* { */
/*   const dependencies_key_t* x = x0; */

/*   return */
/*     (size_t)x->child + */
/*     (size_t)x->dependent; */
/* } */
