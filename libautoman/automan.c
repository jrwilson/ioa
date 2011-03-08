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

#include "decls.h"

#include <assert.h>
#include <stdlib.h>

/******************************************************************************************
 * PRIVATE FUNCTIONS
 ******************************************************************************************/

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

  automan->sequence_status = NORMAL;
  automan->si_table = table_create (sizeof (sequence_item_t));
  automan->si_index = index_create_list (automan->si_table);

  automan->ii_table = table_create (sizeof (input_item_t));
  automan->ii_index = index_create_list (automan->ii_table);

  automan->oi_table = table_create (sizeof (output_item_t));
  automan->oi_index = index_create_list (automan->oi_table);

  automan->proxy_status = NORMAL;
  automan->pi_table = table_create (sizeof (proxy_item_t));
  automan->pi_index = index_create_list (automan->pi_table);

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

      /* Fire the proxies. */
      pi_process (automan);
    }
    break;
  case CHILD_CREATED:
    {
      assert (automan->sequence_status == OUTSTANDING && automan->last_order.type == CREATE);
      *automan->last_sequence.aid_ptr = receipt->child_created.child;
      if (automan->last_sequence.handler != NULL) {
	automan->last_sequence.handler (automan->state, automan->last_sequence.hparam, receipt->type);
      }
      something_changed = true;
      automan->sequence_status = NORMAL;
    }
    break;
  case BAD_DESCRIPTOR:
    /* Currently, the only way to have a bad descriptor is to create an order with a NULL descriptor.
       We don't allow this.*/
    assert (0);
    break;
  case DECLARED:
    {
      assert (automan->sequence_status == OUTSTANDING && automan->last_order.type == DECLARE);
      *automan->last_sequence.flag_ptr = true;
      if (automan->last_sequence.handler != NULL) {
	automan->last_sequence.handler (automan->state, automan->last_sequence.hparam, receipt->type);
      }
      something_changed = true;
      automan->sequence_status = NORMAL;
    }
    break;
  case OUTPUT_DNE:
  case INPUT_DNE:
  case OUTPUT_UNAVAILABLE:
  case INPUT_UNAVAILABLE:
    {
      assert (automan->sequence_status == OUTSTANDING && automan->last_order.type == COMPOSE);

      si_balance_compose (automan, automan->last_sequence.flag_ptr);
      si_decompose_flag_ptr (automan, automan->last_sequence.flag_ptr, receipt->type);

      something_changed = true;
      automan->sequence_status = NORMAL;
    }
    break;
  case COMPOSED:
    {
      assert (automan->sequence_status == OUTSTANDING && automan->last_order.type == COMPOSE);
      *automan->last_sequence.flag_ptr = true;
      if (automan->last_sequence.handler != NULL) {
	automan->last_sequence.handler (automan->state, automan->last_sequence.hparam, receipt->type);
      }
      something_changed = true;
      automan->sequence_status = NORMAL;
    }
    break;
  case INPUT_COMPOSED:
    {
      /* Look up the input. */
      input_item_t* input_item = ii_find_input (automan,
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
      output_item_t* output_item = oi_find_output (automan,
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
    /* Consider the four events:
         A the decompose order is sent
	 B the decompose order is received
	 C the composition is decomposed
	 D the receipt is sent
	 E the receipt is received

       The following sequences are valid.  What will the receipt be?
       C A B D E - NOT_COMPOSED
       A C B D E - NOT_COMPOSED
       A B C D E - DECOMPOSED
       
       However, we will always receive a DECOMPOSED after a NOT_COMPOSED.
       Thus, we need not do anything and let the DECOMPOSED case handle it.
     */
    break;
  case DECOMPOSED:
    {
      /* A composition was decomposed either intentionally or unintentionally, i.e., a child died.

	 If the decompose was intentional, then the sequence is in good order, i.e., the compose and decompose will be found and removed together.

	 The only way something can be decomposed unintentionally is if a child dies.
	 See the CHILD_DESTROYED case.
	 If a child dies, ueioa might send DECOMPOSED events before the CHILD_DESTROYED.
	 To maintain a good sequence, we will pretend as though the user ordered the decompose by inserting a decompose order if necessary.
      */
      if (automan->sequence_status == OUTSTANDING &&
	  automan->last_order.type == DECOMPOSE &&
	  *automan->last_sequence.compose.in_aid_ptr == receipt->decomposed.in_aid &&
	  automan->last_sequence.compose.input == receipt->decomposed.input &&
	  automan->last_sequence.compose.in_param == receipt->decomposed.in_param) {
	/* Decomposed intentionally. */
	si_decompose_flag_ptr (automan, automan->last_sequence.flag_ptr, receipt->type);
	something_changed = true;
	automan->sequence_status = NORMAL;
      }
      else {
	/* Decomposed unintentionally. */

	/* Find the compose. */
	iterator_t compose_pos;
	sequence_item_t* compose = index_find_value (automan->si_index,
						     index_begin (automan->si_index),
						     index_end (automan->si_index),
						     si_decomposed_input_equal,
						     receipt,
						     &compose_pos);
	assert (compose != NULL);
	assert (compose->order_type == COMPOSE);
	bool* flag_ptr = compose->flag_ptr;

	si_balance_compose (automan, flag_ptr);
	si_decompose_flag_ptr (automan, flag_ptr, receipt->type);

	something_changed = true;
      }
    }
    break;
  case INPUT_DECOMPOSED:
    {
      /* Look up the input. */
      input_item_t* input_item = ii_find_input (automan,
						receipt->input_composed.input,
						receipt->input_composed.in_param);
      if (input_item != NULL) {
	/* We are tracking.  Set the flag to false. */
	*input_item->flag_ptr = false;
	/* Call the handler. */
	if (input_item->handler != NULL) {
	  input_item->handler (automan->state, input_item->hparam, receipt->type);
	}
      }
    }
    break;
  case OUTPUT_DECOMPOSED:
    {
      /* Look up the output. */
      output_item_t* output_item = oi_find_output (automan,
						   receipt->output_composed.output,
						   receipt->output_composed.out_param);
      if (output_item != NULL) {
	/* We are tracking.  Set the flag to false. */
	*output_item->flag_ptr = false;
	/* Call the handler. */
	if (output_item->handler != NULL) {
	  output_item->handler (automan->state, output_item->hparam, receipt->type);
	}
      }
    }
    break;
  case RESCINDED:
    {
      assert (automan->sequence_status == OUTSTANDING && automan->last_order.type == RESCIND);
      /* Remove the declare. */
      

      *automan->last_sequence.flag_ptr = false;
      if (automan->last_sequence.handler != NULL) {
	automan->last_sequence.handler (automan->state, automan->last_sequence.hparam, receipt->type);
      }
      something_changed = true;
      automan->sequence_status = NORMAL;
    }
    break;
  case AUTOMATON_DNE:
    /* Consider the four events:
         A the destroy order is sent
	 B the destroy order is received
	 C the automaton is destroyed
	 D the receipt is sent
	 E the receipt is received

       The following sequences are valid.  What will the receipt be?
       C A B D E - AUTOMATON_DNE
       A C B D E - AUTOMATON_DNE
       A B C D E - CHILD_DESTROYED
       
       However, we will always receive a CHILD_DESTROYED after the AUTOMATON_DNE.
       Thus, we need not do anything and let the CHILD_DESTROYED case handle it.
     */
    break;
  case NOT_OWNER:
    /* Destroying an automaton that we didn't create is a logical error. */
    assert (0);
    break;
  case CHILD_DESTROYED:
    {
      /* A child died either intentionally or unintentionally.

	 If a child died intentionally, then the sequence is in good order according to the automan_destroy procedure.
	 By good order, I mean that decomposes have been added, processed, and removed so that the only thing left is the create and destroy.
	 Note that this destroy procedure will still work correctly if we assume that the composes and decompoes still exist and need to be processed.

	 If a child dies unintentionally and we only remove the create and destroy, then the sequence could be left in a bad order due to straggling composes and decomposes.
	 This is possible because ueioa does not specify the order in which CHILD_DESTROYED and DECOMPOSED events are delivered.
	 See also the DECOMPOSED case.

	 To handle this, we will manipulate the sequence to appear as though we expected it.
	 First, we will append an appropriate destroy if one doesn't already exist.
	 This should add any needed decomposes.
	 Then, we'll process it at though we expected it to happen.
	 We'll make use of the fact that the destroy procedure will process composes and decomposes.
	 */

      if (automan->sequence_status == OUTSTANDING &&
	  automan->last_order.type == DESTROY &&
	  *automan->last_sequence.aid_ptr == receipt->child_destroyed.child) {
	/* Child died intentionally. */
	si_destroy_aid_ptr (automan, automan->last_sequence.aid_ptr);
	something_changed = true;
	automan->sequence_status = NORMAL;
      }
      else {
	/* Child died unintentionally. */

	/* Find the create. */
	iterator_t create_pos;
	sequence_item_t* create = index_find_value (automan->si_index,
						    index_begin (automan->si_index),
						    index_end (automan->si_index),
						    si_child_destroyed_aid_equal,
						    receipt,
						    &create_pos);
	assert (create != NULL);
	assert (create->order_type == CREATE);
	aid_t* aid_ptr = create->aid_ptr;

	si_balance_create (automan, aid_ptr);
	si_destroy_aid_ptr (automan, aid_ptr);

	something_changed = true;
      }
    }
    break;
  }

  if ((automan->sequence_status == NORMAL) && something_changed) {
    assert (schedule_system_output () == 0);
  }
}
