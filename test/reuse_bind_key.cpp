#include "minunit.h"

#include <iostream>
#include <ioa/ioa.hpp>
#include <ioa/simple_scheduler.hpp>

/*
  Observation:
  A program using only create, bind, and suicide caused the automaton implementation received a BIND_KEY_EXISTS_RESULT.
  This should never happen because the automaton implementation is designed to prohibit this.
  
  Hypothesis:
  Bind keys are erased when an automaton commits suicide.
  
  Experiment:
  Create automaton A.
  Create automaton B1.
  Bind A and B1 using key K.
  B1 commits suicide.
  Create automaton B2.
  Bind A and B2 using key K.

  Analysis:
  The experiment generated the error as predicted.
  Inspection of the code in model.cpp showed that bind keys were only removed for explicit calls to unbind.

  Solution:
  The model was changed to allow for the indirect addition and removal of bind keys.
  The struct that records bindings in action_executor.hpp was modified to add and remove bind keys using the new interface.
    
 */

template <class OI, class OM, class II, class IM>
class static_binding :
  private ioa::observer,
  private ioa::system_binding_manager_interface
{
private:
  bool m_bound;
  size_t m_unbound_count;
  ioa::automaton* m_automaton;
  ioa::automaton_handle_interface<OI>* m_output;
  OM OI::*m_output_member_ptr;
  ioa::automaton_handle_interface<II>* m_input;
  IM II::*m_input_member_ptr;
  ioa::automaton_handle<OI> m_output_handle;
  ioa::automaton_handle<II> m_input_handle;

  void observe (ioa::observable* o) {
    if (o == m_output) {
      m_output_handle = m_output->get_handle ();
    }
    else if (o == m_input) {
      m_input_handle = m_input->get_handle ();
    }

    process ();
  }

  void process () {
    if (m_output_handle != -1 &&
	m_input_handle != -1) {
      m_automaton->bind (this);
    }
  }

  ioa::shared_ptr<ioa::bind_executor_interface> get_executor () const {
    return make_bind_executor (this->m_output_handle, this->m_output_member_ptr,
			       this->m_input_handle, this->m_input_member_ptr);
  }

  void bound (const ioa::bound_t result) {
    switch (result) {
    case ioa::BIND_KEY_EXISTS_RESULT:
      assert (false);
    case ioa::OUTPUT_AUTOMATON_DNE_RESULT:
      assert (false);
      break;
    case ioa::INPUT_AUTOMATON_DNE_RESULT:
      assert (false);
      break;
    case ioa::BINDING_EXISTS_RESULT:
      assert (false);
      break;
    case ioa::OUTPUT_ACTION_UNAVAILABLE_RESULT:
      assert (false);
      break;
    case ioa::INPUT_ACTION_UNAVAILABLE_RESULT:
      assert (false);
      break;
    case ioa::BOUND_RESULT:
      // Okay.
      m_bound = true;
      break;
    }
  }

  void unbound (const ioa::unbound_t result) {
    switch (result) {
    case ioa::BIND_KEY_DNE_RESULT:
      assert (false);
      break;
    case ioa::UNBOUND_RESULT:
      // Okay.
      m_bound = false;
      ++m_unbound_count;
      if (m_unbound_count == 2) {
	delete this;
      }
      break;
    }
  }

public:

  static_binding (ioa::automaton* automaton) :
    m_bound (false),
    m_unbound_count (0),
    m_automaton (automaton)
  { }

  void set_output (ioa::automaton_handle_interface<OI>* output,
		   OM OI::*output_member_ptr) {
    assert (output != 0);
    m_output = output;
    m_output_member_ptr = output_member_ptr;
    m_output_handle = m_output->get_handle ();
    add_observable (m_output);
    process ();
  }

  void set_input (ioa::automaton_handle_interface<II>* input,
		  IM II::*input_member_ptr) {
    assert (input != 0);
    m_input = input;
    m_input_member_ptr = input_member_ptr;
    m_input_handle = m_input->get_handle ();
    add_observable (m_input);
    process ();
  }

  bool is_bound () const {
    return m_bound;
  }
};

class automaton_B :
  public ioa::automaton
{
private:
  void schedule () const { }
  void input_effect () {
    self_destruct ();
  }
public:
  UV_UP_INPUT (automaton_B, input);
};

class automaton_A :
  public ioa::automaton,
  private ioa::observer
{
private:
  ioa::handle_manager<automaton_A> m_self;
  ioa::automaton_manager<automaton_B>* m_b1;
  bool m_rebind_flag;

  void observe (ioa::observable* o) {
    if (o == m_b1) {
      if (-1 == m_b1->get_handle ()) {
	m_b1 = 0;
      }
    }
    schedule ();
  }

  void schedule () const {
    if (output_precondition ()) {
      ioa::schedule (&automaton_A::output);
    }
    if (rebind_precondition ()) {
      ioa::schedule (&automaton_A::rebind);
    }
  }
  
  bool output_precondition () const {
    return m_b1 != 0 && ioa::binding_count (&automaton_A::output) != 0;
  }
  void output_effect () { }
  UV_UP_OUTPUT (automaton_A, output);

  bool rebind_precondition () const {
    return m_b1 == 0 && !m_binding->is_bound () && !m_rebind_flag;
  }
  void rebind_effect () {
    ioa::automaton_manager<automaton_B>* b2 = new ioa::automaton_manager<automaton_B> (this, ioa::make_generator<automaton_B> ());
    m_binding->set_output (&m_self, &automaton_A::output);
    m_binding->set_input (b2, &automaton_B::input);
    m_rebind_flag = true;
  }
  UP_INTERNAL (automaton_A, rebind);

  static_binding<automaton_A, output_type, automaton_B, automaton_B::input_type>* m_binding;

public:
  automaton_A () :
    m_self (ioa::get_aid ()),
    m_rebind_flag (false) {
    m_binding = new static_binding<automaton_A, output_type, automaton_B, automaton_B::input_type> (this);
    m_b1 = new ioa::automaton_manager<automaton_B> (this, ioa::make_generator<automaton_B> ());
    add_observable (m_b1);
    m_binding->set_output (&m_self, &automaton_A::output);
    m_binding->set_input (m_b1, &automaton_B::input);
  }
};

static const char*
bind_key_exists ()
{
  std::cout << __func__ << std::endl;

  ioa::simple_scheduler ss;
  ioa::run (ss, ioa::make_generator<automaton_A> ());

  return 0;
}

const char*
all_tests ()
{
  mu_run_test (bind_key_exists);

  return 0;
}
