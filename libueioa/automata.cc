#include "automata.hh"

#include <cassert>
#include <iostream>

automata::automata () :
  m_next_aid (0)
{
  pthread_key_create (&m_current_aid, NULL);
  pthread_rwlock_init (&m_lock, NULL);
}

automata::~automata()
{
  std::for_each (m_automaton_entries.begin (),
		 m_automaton_entries.end (),
		 free_state_lock ());
  
  pthread_rwlock_destroy (&m_lock);
  pthread_key_delete (m_current_aid);
}

void
automata::set_current_aid (aid_t aid)
{
  pthread_setspecific (m_current_aid, (void*)aid);
}

void
automata::create (Receipts& receipts, Runq& runq, const aid_t parent, const descriptor_t* descriptor, const void* arg)
{
  if (descriptor != NULL) {

    /* Find an aid. */
    do {
      ++m_next_aid;
      if (m_next_aid < 0) {
	m_next_aid = 0;
      }
    } while (std::find_if (m_automaton_entries.begin (),
			   m_automaton_entries.end (),
			   automaton_entry_aid_equal (m_next_aid)) != m_automaton_entries.end ());
    
    aid_t aid = m_next_aid;

    set_current_aid (aid);
    void* state = NULL;
    if (descriptor->constructor != NULL) {
      state = descriptor->constructor (arg);
    }
    set_current_aid (-1);
    
    /* Insert the automaton. */
    pthread_mutex_t* lock = (pthread_mutex_t*)malloc (sizeof (pthread_mutex_t));
    pthread_mutex_init (lock, NULL);
    automaton_entry entry;
    entry.aid = aid;
    entry.parent = parent;
    entry.state = state;
    entry.lock = lock;
    entry.system_input = descriptor->system_input;
    entry.system_output = descriptor->system_output;
    entry.alarm_input = descriptor->alarm_input;
    entry.read_input = descriptor->read_input;
    entry.write_input = descriptor->write_input;
    m_automaton_entries.push_back (entry);

    size_t idx;

    /* Insert the free inputs. */
    if (descriptor->free_inputs != NULL) {
      for (idx = 0; descriptor->free_inputs[idx] != NULL; ++idx) {
	m_free_input_entries.insert (input_entry (aid, descriptor->free_inputs[idx]));
      }
    }
   
    /* Insert the inputs. */
    if (descriptor->inputs != NULL) {
      for (idx = 0; descriptor->inputs[idx] != NULL; ++idx) {
	m_input_entries.insert (input_entry (aid, descriptor->inputs[idx]));
      }
    }
    
    /* Insert the outputs. */
    if (descriptor->outputs != NULL) {
      for (idx = 0; descriptor->outputs[idx] != NULL; ++idx) {
	m_output_entries.insert (output_entry (aid, descriptor->outputs[idx]));
      }
    }
    
    /* Insert the internals. */
    if (descriptor->internals != NULL) {
      for (idx = 0; descriptor->internals[idx] != NULL; ++idx) {
	m_internal_entries.insert (internal_entry (aid, descriptor->internals[idx]));
      }
    }

    /* Declare the NULL parameter. */
    {
      m_param_entries.insert (param_entry (aid, NULL));
    }
    
    /* Tell the automaton that it has been created. */
    receipts.push_self_created (aid, aid);
    runq.insert_system_input (aid);
    
    if (parent != -1) {
      /* Tell the parent that the automaton has been created. */
      receipts.push_child_created (parent, aid);
      runq.insert_system_input (parent);
    }
  }
  else {
    /* Bad descriptor. */
    if (parent != -1) {
      receipts.push_bad_descriptor (parent);
      runq.insert_system_input (parent);
    }
    else {
      std::cerr << "Bad root descriptor" << std::endl;
      exit (EXIT_FAILURE);
    }
  }
}

void
automata::declare (Receipts& receipts, Runq& runq, const aid_t aid, void* param)
{
  m_param_entries.insert (param_entry (aid, param));

  receipts.push_declared (aid);
  runq.insert_system_input (aid);
}

void
automata::compose (Receipts& receipts, Runq& runq, aid_t aid, aid_t out_aid, output_t output, void* out_param, aid_t in_aid, input_t input, void* in_param)
{
  if (m_output_entries.find (output_entry (out_aid, output)) == m_output_entries.end () ||
      m_param_entries.find (param_entry (out_aid, out_param)) == m_param_entries.end ()) {
    /* Output doesn't exist. */
    receipts.push_output_dne (aid);
    runq.insert_system_input (aid);
  }
  else if (m_input_entries.find (input_entry (in_aid, input)) == m_input_entries.end () ||
	   m_param_entries.find (param_entry (in_aid, in_param)) == m_param_entries.end ()) {
    /* Input doesn't exist. */
    receipts.push_input_dne (aid);
    runq.insert_system_input (aid);
  }
  else if ((std::find_if (m_composition_entries.begin (),
			  m_composition_entries.end (),
			  composition_entry_in_aid_input_in_param_equal (in_aid, input, in_param)) != m_composition_entries.end ()) ||
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
    receipts.push_input_unavailable (aid);
    runq.insert_system_input (aid);
  }
  else if ((std::find_if (m_composition_entries.begin (),
			  m_composition_entries.end (),
			  composition_entry_out_aid_output_out_param_in_aid_equal (out_aid, output, out_param, in_aid)) != m_composition_entries.end ()) ||
	   (out_param != NULL && aid != out_aid)) {
    /* Output isn't available.
     */
    receipts.push_output_unavailable (aid);
    runq.insert_system_input (aid);
  }
  else {
    /* Compose. */
    composition_entry entry;
    entry.aid = aid;
    entry.out_aid = out_aid;
    entry.output = output;
    entry.out_param = out_param;
    entry.in_aid = in_aid;
    entry.input = input;
    entry.in_param = in_param;
    m_composition_entries.insert (entry);
    
    receipts.push_composed (aid);
    runq.insert_system_input (aid);

    receipts.push_input_composed (in_aid, input, in_param);
    runq.insert_system_input (in_aid);
    
    receipts.push_output_composed (out_aid, output, out_param);
    runq.insert_system_input (out_aid);
  }
}

void
automata::decompose (Receipts& receipts, Runq& runq, aid_t aid, aid_t out_aid, output_t output, void* out_param, aid_t in_aid, input_t input, void* in_param)
{
  /* Look up the composition.
     We only need to use the input since it must be unique.
  */
  composition_list::const_iterator pos = std::find_if (m_composition_entries.begin (),
						       m_composition_entries.end (),
						       composition_entry_in_aid_input_in_param_equal (in_aid, input, in_param));
  if (pos != m_composition_entries.end () &&
      pos->out_aid == out_aid &&
      pos->output == output &&
      pos->out_param == out_param &&
      pos->aid == aid) {
    m_composition_entries.erase (pos);
    
    receipts.push_decomposed (aid, in_aid, input, in_param);
    runq.insert_system_input (aid);
    
    receipts.push_input_decomposed (in_aid, input, in_param);
    runq.insert_system_input (in_aid);
    
    receipts.push_output_decomposed (out_aid, output, out_param);
    runq.insert_system_input (out_aid);
  }
  else {
    receipts.push_not_composed (aid);
    runq.insert_system_input (aid);
  }
}

void
automata::rescind (Receipts& receipts, Runq& runq, aid_t aid, void* param)
{
  /* Params. */

  m_param_entries.erase (param_entry (aid, param));

  receipts.push_rescinded (aid);
  runq.insert_system_input (aid);

  /* Compositions. */
  composition_list::iterator pos = m_composition_entries.begin ();
  while (pos != m_composition_entries.end ()) {
    if ((pos->in_aid == aid && pos->in_param == param) ||
	(pos->out_aid == aid && pos->out_param == param)) {
      /* Composer. */
      receipts.push_decomposed (pos->aid, pos->in_aid, pos->input, pos->in_param);
      runq.insert_system_input (pos->aid);
      
      /* Input. */
      receipts.push_input_decomposed (pos->in_aid, pos->input, pos->in_param);
      runq.insert_system_input (pos->in_aid);
      
      /* Output. */
      receipts.push_output_decomposed (pos->out_aid, pos->output, pos->out_param);
      runq.insert_system_input (pos->out_aid);

      m_composition_entries.erase (pos);
      // TODO: Does an erase invalidate iterators?
      /* Start over to be cautious. */
      pos = m_composition_entries.begin ();
    }
    else {
      ++pos;
    }
  }

  /* Runq. */
  runq.purge_aid_param (aid, param);
}

void
automata::destroy_r (Receipts& receipts, Runq& runq, buffers& buffers, aid_t aid)
{
  /* Children. */
  automaton_list::const_iterator child_pos;
  while ((child_pos = std::find_if (m_automaton_entries.begin (),
				    m_automaton_entries.end (),
				    automaton_entry_parent_equal (aid))) != m_automaton_entries.end ()) {
    destroy_r (receipts, runq, buffers, child_pos->aid);
  }

  /* Automaton. */
  automaton_list::iterator pos = std::find_if (m_automaton_entries.begin (),
					       m_automaton_entries.end (),
					       automaton_entry_aid_equal (aid));
  free (pos->lock);
  free (pos->state);
  aid_t parent = pos->parent;
  m_automaton_entries.erase (pos);

  /* Free inputs. */
  input_list::iterator input_pos;
  input_pos = m_free_input_entries.begin ();
  while (input_pos != m_free_input_entries.end ()) {
    if (input_pos->aid == aid) {
      m_free_input_entries.erase (input_pos);
      // TODO: Does an erase invalidate iterators?
      /* Start over to be cautious. */
      input_pos = m_free_input_entries.begin ();
    }
    else {
      ++input_pos;
    }
  }

  /* Inputs. */
  input_pos = m_input_entries.begin ();
  while (input_pos != m_input_entries.end ()) {
    if (input_pos->aid == aid) {
      m_input_entries.erase (input_pos);
      // TODO: Does an erase invalidate iterators?
      /* Start over to be cautious. */
      input_pos = m_input_entries.begin ();
    }
    else {
      ++input_pos;
    }
  }

  /* Outputs. */
  output_list::iterator output_pos;
  output_pos = m_output_entries.begin ();
  while (output_pos != m_output_entries.end ()) {
    if (output_pos->aid == aid) {
      m_output_entries.erase (output_pos);
      // TODO: Does an erase invalidate iterators?
      /* Start over to be cautious. */
      output_pos = m_output_entries.begin ();
    }
    else {
      ++output_pos;
    }
  }

  /* Internals. */
  internal_list::iterator internal_pos;
  internal_pos = m_internal_entries.begin ();
  while (internal_pos != m_internal_entries.end ()) {
    if (internal_pos->aid == aid) {
      m_internal_entries.erase (internal_pos);
      // TODO: Does an erase invalidate iterators?
      /* Start over to be cautious. */
      internal_pos = m_internal_entries.begin ();
    }
    else {
      ++internal_pos;
    }
  }

  /* Params. */
  param_list::iterator param_pos;
  param_pos = m_param_entries.begin ();
  while (param_pos != m_param_entries.end ()) {
    if (param_pos->aid == aid) {
      m_param_entries.erase (param_pos);
      // TODO: Does an erase invalidate iterators?
      /* Start over to be cautious. */
      param_pos = m_param_entries.begin ();
    }
    else {
      ++param_pos;
    }
  }

  /* Compositions. */
  composition_list::iterator composition_pos = m_composition_entries.begin ();
  while (composition_pos != m_composition_entries.end ()) {
    if (aid == composition_pos->aid ||
	aid == composition_pos->in_aid ||
	aid == composition_pos->out_aid) {
      /* Composer. */
      receipts.push_decomposed (composition_pos->aid, composition_pos->in_aid, composition_pos->input, composition_pos->in_param);
      runq.insert_system_input (composition_pos->aid);
      
      /* Input. */
      receipts.push_input_decomposed (composition_pos->in_aid, composition_pos->input, composition_pos->in_param);
      runq.insert_system_input (composition_pos->in_aid);
      
      /* Output. */
      receipts.push_output_decomposed (composition_pos->out_aid, composition_pos->output, composition_pos->out_param);
      runq.insert_system_input (composition_pos->out_aid);

      m_composition_entries.erase (composition_pos);
      // TODO: Does an erase invalidate iterators?
      /* Start over to be cautious. */
      composition_pos = m_composition_entries.begin ();
    }
    else {
      ++composition_pos;
    }
  }

  /* Runq. */
  runq.purge_aid (aid);

  /* Receipts. */
  receipts.purge_aid (aid);
  
  /* buffers. */
  buffers.purge_aid (aid);

  if (parent != -1) {
    receipts.push_child_destroyed (parent, aid);
    runq.insert_system_input (parent);
  }
}

void
automata::destroy (Receipts& receipts, Runq& runq, buffers& buffers, aid_t aid, aid_t target)
{
  automaton_list::const_iterator pos = std::find_if (m_automaton_entries.begin (),
						     m_automaton_entries.end (),
						     automaton_entry_aid_equal (target));
  if (pos != m_automaton_entries.end ()) {
    if (aid == target ||
	aid == pos->parent) {
      destroy_r (receipts, runq, buffers, target);
    }
    else {
      receipts.push_not_owner (aid);
      runq.insert_system_input (aid);
    }
  }
  else {
    receipts.push_automaton_dne (aid);
    runq.insert_system_input (aid);
  }
}

void
automata::create_automaton (Receipts& receipts, Runq& runq, const descriptor_t* descriptor, const void* arg)
{
  /* Acquire the write lock. */
  pthread_rwlock_wrlock (&m_lock);

  create (receipts, runq, -1, descriptor, arg);
  
  /* Release the write lock. */
  pthread_rwlock_unlock (&m_lock);
}

void
automata::system_input_exec (Receipts& receipts, Runq& runq, buffers& buffers, aid_t aid)
{
  /* Acquire the read lock. */
  pthread_rwlock_rdlock (&m_lock);

  automaton_list::const_iterator pos = std::find_if (m_automaton_entries.begin (),
						     m_automaton_entries.end (),
						     automaton_entry_aid_equal (aid));
  /* Automaton must exist. */
  if (pos != m_automaton_entries.end ()) {
    
    receipt_t receipt;
    if (receipts.pop (aid, &receipt) == 0) {

      /* Prepare the buffer. */
      bid_t bid = buffers.alloc (-1, sizeof (receipt_t));
      receipt_t* r = (receipt_t*)buffers.write_ptr (-1, bid);
      *r = receipt;
      /* Change owner to make read-only. */
      buffers.change_owner (aid, bid);
      set_current_aid (aid);
      pthread_mutex_lock (pos->lock);
      if (pos->system_input != NULL) {
	pos->system_input (pos->state, NULL, bid);
      }
      pthread_mutex_unlock (pos->lock);
      set_current_aid (-1);
      /* Decrement to destroy if unused. */
      buffers.decref (-1, bid);
      
      /* Schedule again. */
      if (!receipts.empty (aid)) {
	runq.insert_system_input (aid);
      }
    }

  }  
  
  /* Release the read lock. */
  pthread_rwlock_unlock (&m_lock);
}

void
automata::system_output_exec (Receipts& receipts, Runq& runq, ioq& ioq, buffers& buffers, aid_t aid)
{
  /* Acquire the write lock. */
  pthread_rwlock_wrlock (&m_lock);

  automaton_list::const_iterator pos = std::find_if (m_automaton_entries.begin (),
						     m_automaton_entries.end (),
						     automaton_entry_aid_equal (aid));
  /* Automaton must exist. */
  if (pos != m_automaton_entries.end () && pos->system_output != NULL) {

    /* Execute. */
    set_current_aid (aid);
    pthread_mutex_lock (pos->lock);
    bid_t bid = pos->system_output (pos->state, NULL);
    pthread_mutex_unlock (pos->lock);
    set_current_aid (-1);
    
    /* Bid must exist. */
    if (buffers.exists (aid, bid)) {
      if (buffers.size (aid, bid) == sizeof (order_t)) {
	order_t order = *(order_t*)buffers.read_ptr (aid, bid);
	buffers.decref (aid, bid);

	switch (order.type) {
	case CREATE:
	  create (receipts, runq, aid, order.create.descriptor, order.create.arg);
	  break;
	case DECLARE:
	  declare (receipts, runq, aid, order.declare.param);
	  break;
	case COMPOSE:
	  compose (receipts, runq, aid, order.compose.out_aid, order.compose.output, order.compose.out_param, order.compose.in_aid, order.compose.input, order.compose.in_param);
	  break;
	case DECOMPOSE:
	  decompose (receipts, runq, aid, order.decompose.out_aid, order.decompose.output, order.decompose.out_param, order.decompose.in_aid, order.decompose.input, order.decompose.in_param);
	  break;
	case RESCIND:
	  rescind (receipts, runq, aid, order.rescind.param);
	  break;
	case DESTROY:
	  destroy (receipts, runq, buffers, aid, order.destroy.aid);
	  break;
	default:
	  receipts.push_bad_order (aid);
	  runq.insert_system_input (aid);
	  break;
	}
      }
      else {
	receipts.push_bad_order (aid);
	runq.insert_system_input (aid);
      }
    }
  }

  /* Release the write lock. */
  pthread_rwlock_unlock (&m_lock);
}

void
automata::alarm_input_exec (buffers& buffers, aid_t aid)
{
  /* Acquire the read lock. */
  pthread_rwlock_rdlock (&m_lock);

  automaton_list::const_iterator pos = std::find_if (m_automaton_entries.begin (),
						     m_automaton_entries.end (),
						     automaton_entry_aid_equal (aid));
  /* Automaton must exist. */
  if (pos != m_automaton_entries.end () && pos->alarm_input != NULL) {
    
    /* Prepare the buffer. */
    bid_t bid = buffers.alloc (-1, 0);
    /* Change owner to make read-only. */
    buffers.change_owner (aid, bid);
    set_current_aid (aid);
    pthread_mutex_lock (pos->lock);
    pos->alarm_input (pos->state, NULL, bid);
    pthread_mutex_unlock (pos->lock);
    set_current_aid (-1);
    /* Decrement to destroy. */
    buffers.decref (-1, bid);
  }  
  
  /* Release the read lock. */
  pthread_rwlock_unlock (&m_lock);
}

void
automata::read_input_exec (buffers& buffers, aid_t aid)
{
  /* Acquire the read lock. */
  pthread_rwlock_rdlock (&m_lock);

  automaton_list::const_iterator pos = std::find_if (m_automaton_entries.begin (),
						     m_automaton_entries.end (),
						     automaton_entry_aid_equal (aid));
  /* Automaton must exist. */
  if (pos != m_automaton_entries.end () && pos->read_input != NULL) {
    
    /* Prepare the buffer. */
    bid_t bid = buffers.alloc (-1, 0);
    /* Change owner to make read-only. */
    buffers.change_owner (aid, bid);
    set_current_aid (aid);
    pthread_mutex_lock (pos->lock);
    pos->read_input (pos->state, NULL, bid);
    pthread_mutex_unlock (pos->lock);
    set_current_aid (-1);
    /* Decrement to destroy. */
    buffers.decref (-1, bid);
  }  
  
  /* Release the read lock. */
  pthread_rwlock_unlock (&m_lock);
}

void
automata::write_input_exec (buffers& buffers, aid_t aid)
{
  /* Acquire the read lock. */
  pthread_rwlock_rdlock (&m_lock);

  automaton_list::const_iterator pos = std::find_if (m_automaton_entries.begin (),
						     m_automaton_entries.end (),
						     automaton_entry_aid_equal (aid));
  /* Automaton must exist. */
  if (pos != m_automaton_entries.end () && pos->write_input != NULL) {
    
    /* Prepare the buffer. */
    bid_t bid = buffers.alloc (-1, 0);
    /* Change owner to make read-only. */
    buffers.change_owner (aid, bid);
    set_current_aid (aid);
    pthread_mutex_lock (pos->lock);
    pos->write_input (pos->state, NULL, bid);
    pthread_mutex_unlock (pos->lock);
    set_current_aid (-1);
    /* Decrement to destroy. */
    buffers.decref (-1, bid);
  }  
  
  /* Release the read lock. */
  pthread_rwlock_unlock (&m_lock);
}

void
automata::free_input_exec (buffers& buffers, aid_t caller_aid, aid_t aid, input_t free_input, bid_t bid)
{
  /* Acquire the read lock. */
  pthread_rwlock_rdlock (&m_lock);

  input_list::const_iterator free_input_pos = m_free_input_entries.find (input_entry (aid, free_input));

  /* Free input and buffer exist. */
  if (free_input_pos != m_free_input_entries.end () && buffers.exists (caller_aid, bid)) {
    automaton_list::const_iterator pos = std::find_if (m_automaton_entries.begin (),
						       m_automaton_entries.end (),
						       automaton_entry_aid_equal (aid));
    buffers.change_owner (aid, bid);
    set_current_aid (aid);
    pthread_mutex_lock (pos->lock);
    free_input (pos->state, NULL, bid);
    pthread_mutex_unlock (pos->lock);
    set_current_aid (-1);
    buffers.decref (caller_aid, bid);
  }
  
  /* Release the read lock. */
  pthread_rwlock_unlock (&m_lock);
}

void
automata::output_exec (buffers& buffers, aid_t out_aid, output_t output, void* out_param)
{
  /* Acquire the read lock. */
  pthread_rwlock_rdlock (&m_lock);

  /* Look up the output.  Success implies the automaton exists. */
  output_list::const_iterator output_pos = m_output_entries.find (output_entry (out_aid, output));
  /* Look up the parameter. */
  param_list::const_iterator param_pos = m_param_entries.find (param_entry (out_aid, out_param));

  /* Automaton and param must exist. */
  if (output_pos != m_output_entries.end () &&
      param_pos != m_param_entries.end ()) {

    automaton_list::const_iterator out_pos = std::find_if (m_automaton_entries.begin (),
							   m_automaton_entries.end (),
							   automaton_entry_aid_equal (out_aid));
    assert (out_pos != m_automaton_entries.end ());

    /* Lock the automata in order. */
    std::for_each (m_composition_entries.begin (),
		   m_composition_entries.end (),
		   output_lock (*this, out_aid, output, out_param, out_pos));

    /* Execute the output. */
    set_current_aid (out_aid);
    bid_t bid = output (out_pos->state, out_param);
    set_current_aid (-1);

    /* Check the bid. */
    if (buffers.exists (out_aid, bid)) {
      /* Execute the inputs. */
      std::for_each (m_composition_entries.begin (),
		     m_composition_entries.end (),
		     output_execute (*this, buffers, out_aid, output, out_param, bid));

      /* Decrement to clean unused buffers. */
      buffers.decref (out_aid, bid);
    }
    
    /* Unlock. */
    std::for_each (m_composition_entries.begin (),
		   m_composition_entries.end (),
		   output_unlock (*this, out_aid, output, out_param, out_pos));
  }

  /* Release the read lock. */
  pthread_rwlock_unlock (&m_lock);
}

void
automata::internal_exec (aid_t aid, internal_t internal, void* param)
{
  /* Acquire the read lock. */
  pthread_rwlock_rdlock (&m_lock);

  /* Look up the internal.  Success implies the automaton exists. */
  internal_list::const_iterator internal_pos = m_internal_entries.find (internal_entry (aid, internal));
  /* Look up the parameter. */
  param_list::const_iterator param_pos = m_param_entries.find (param_entry (aid, param));

  /* Automaton and param must exist. */
  if (internal_pos != m_internal_entries.end () &&
      param_pos != m_param_entries.end ()) {

    automaton_list::const_iterator automaton_pos = std::find_if (m_automaton_entries.begin (),
								 m_automaton_entries.end (),
								 automaton_entry_aid_equal (aid));
    assert (automaton_pos != m_automaton_entries.end ());
    
    /* Set the current automaton. */
    set_current_aid (aid);
    
    /* Lock the automaton. */
    pthread_mutex_lock (automaton_pos->lock);
    
    /* Execute. */
    internal (automaton_pos->state, param);
    
    /* Unlock the automaton. */
    pthread_mutex_unlock (automaton_pos->lock);
    
    /* Clear the current automaton. */
    set_current_aid (-1);
  }

  /* Release the read lock. */
  pthread_rwlock_unlock (&m_lock);
}

aid_t
automata::get_current_aid ()
{
  return (aid_t)pthread_getspecific (m_current_aid);
}

bool
automata::system_output_exists (aid_t aid)
{
  automaton_list::const_iterator pos = std::find_if (m_automaton_entries.begin (),
						     m_automaton_entries.end (),
						     automaton_entry_aid_equal (aid));
  return pos->system_output != NULL;
}

bool
automata::alarm_input_exists (aid_t aid)
{
  automaton_list::const_iterator pos = std::find_if (m_automaton_entries.begin (),
						     m_automaton_entries.end (),
						     automaton_entry_aid_equal (aid));
  return pos->alarm_input != NULL;
}

bool
automata::read_input_exists (aid_t aid)
{
  automaton_list::const_iterator pos = std::find_if (m_automaton_entries.begin (),
						     m_automaton_entries.end (),
						     automaton_entry_aid_equal (aid));
  return pos->read_input != NULL;
}

bool
automata::write_input_exists (aid_t aid)
{
  automaton_list::const_iterator pos = std::find_if (m_automaton_entries.begin (),
						     m_automaton_entries.end (),
						     automaton_entry_aid_equal (aid));
  return pos->write_input != NULL;
}

bool
automata::free_input_exists (aid_t aid, input_t free_input)
{
  return m_free_input_entries.find (input_entry (aid, free_input)) != m_free_input_entries.end ();
}

bool
automata::output_exists (aid_t aid, output_t output, void* param)
{
  return
    m_output_entries.find (output_entry (aid, output)) != m_output_entries.end () &&
    m_param_entries.find (param_entry (aid, param)) != m_param_entries.end ();
}

bool
automata::internal_exists (aid_t aid, internal_t internal, void* param)
{
  return
    m_internal_entries.find (internal_entry (aid, internal)) != m_internal_entries.end () &&
    m_param_entries.find (param_entry (aid, param)) != m_param_entries.end ();
}

