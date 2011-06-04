#include "minunit.h"

#include <ioa/binding.hpp>
#include "automaton1.hpp"

class empty_class { };

namespace ioa {
  void system_scheduler::set_current_aid (aid_t, const automaton_interface&) { }
  void system_scheduler::clear_current_aid () { }

  void automaton_locker::lock_automaton (aid_t) { }
  void automaton_locker::unlock_automaton (aid_t) { }
}

static const char*
unbind_unvalued_unparameterized_output_action ()
{
  automaton1 binder_instance;
  automaton1 output_instance;
  automaton1 input1_instance;
  automaton1 input2_instance;
  ioa::automaton_handle<automaton1> binder_handle (0);
  ioa::automaton_handle<automaton1> output_handle (1);
  ioa::automaton_handle<automaton1> input1_handle (2);
  ioa::automaton_handle<automaton1> input2_handle (3);

  int input_parameter = 345;

  ioa::action<automaton1, automaton1::up_uv_output_action> output (output_handle, &automaton1::up_uv_output);
  ioa::action<automaton1, automaton1::up_uv_input_action> input1 (input1_handle, &automaton1::up_uv_input);
  ioa::action<automaton1, automaton1::p_uv_input_action> input2 (input2_handle, &automaton1::p_uv_input, input_parameter, ioa::parameterized ());

  ioa::binding<automaton1::up_uv_output_action> binding;
  empty_class d;
  binding.bind (1, output_instance, output, input1_instance, input1, binder_instance, binder_handle.aid (), d);
  binding.bind (2, output_instance, output, input2_instance, input2, binder_instance, binder_handle.aid (), d);

  mu_assert (!binding.empty ());
  mu_assert (binding.involves_output (output));
  mu_assert (binding.involves_binding (output, input1, binder_handle.aid ()));
  mu_assert (binding.involves_binding (output, input2, binder_handle.aid ()));
  mu_assert (binding.involves_input (input1));
  mu_assert (binding.involves_input (input2));
  mu_assert (binding.involves_input_automaton (input1_handle.aid ()));
  mu_assert (binding.involves_input_automaton (input2_handle.aid ()));

  binding.execute ();

  mu_assert (output_instance.up_uv_output.state);
  mu_assert (input1_instance.up_uv_input.state);
  mu_assert (input2_instance.p_uv_input.state);
  mu_assert (input2_instance.p_uv_input.last_parameter == input_parameter);

  binding.unbind (binder_handle.aid (), 1);

  mu_assert (!binding.empty ());
  mu_assert (binding.involves_output (output));
  mu_assert (!binding.involves_binding (output, input1, binder_handle.aid ()));
  mu_assert (binding.involves_binding (output, input2, binder_handle.aid ()));
  mu_assert (!binding.involves_input (input1));
  mu_assert (binding.involves_input (input2));
  mu_assert (!binding.involves_input_automaton (input1_handle.aid ()));
  mu_assert (binding.involves_input_automaton (input2_handle.aid ()));

  binding.unbind (binder_handle.aid (), 2);

  mu_assert (binding.empty ());
  mu_assert (!binding.involves_output (output));
  mu_assert (!binding.involves_binding (output, input1, binder_handle.aid ()));
  mu_assert (!binding.involves_binding (output, input2, binder_handle.aid ()));
  mu_assert (!binding.involves_input (input1));
  mu_assert (!binding.involves_input (input2));
  mu_assert (!binding.involves_input_automaton (input1_handle.aid ()));
  mu_assert (!binding.involves_input_automaton (input2_handle.aid ()));
  
  return 0;
}

static const char*
unbind_unvalued_parameterized_output_action ()
{
  automaton1 binder_instance;
  automaton1 output_instance;
  automaton1 input1_instance;
  automaton1 input2_instance;
  ioa::automaton_handle<automaton1> binder_handle (0);
  ioa::automaton_handle<automaton1> output_handle (1);
  ioa::automaton_handle<automaton1> input1_handle (2);
  ioa::automaton_handle<automaton1> input2_handle (3);

  int output_parameter = 123;
  int input_parameter = 456;

  ioa::action<automaton1, automaton1::p_uv_output_action> output (output_handle, &automaton1::p_uv_output, output_parameter, ioa::parameterized ());
  ioa::action<automaton1, automaton1::up_uv_input_action> input1 (input1_handle, &automaton1::up_uv_input);
  ioa::action<automaton1, automaton1::p_uv_input_action> input2 (input2_handle, &automaton1::p_uv_input, input_parameter, ioa::parameterized ());

  ioa::binding<automaton1::p_uv_output_action> binding;
  empty_class d;
  binding.bind (1, output_instance, output, input1_instance, input1, binder_instance, binder_handle.aid (), d);
  binding.bind (2, output_instance, output, input2_instance, input2, binder_instance, binder_handle.aid (), d);

  mu_assert (!binding.empty ());
  mu_assert (binding.involves_output (output));
  mu_assert (binding.involves_binding (output, input1, binder_handle.aid ()));
  mu_assert (binding.involves_binding (output, input2, binder_handle.aid ()));
  mu_assert (binding.involves_input (input1));
  mu_assert (binding.involves_input (input2));
  mu_assert (binding.involves_input_automaton (input1_handle.aid ()));
  mu_assert (binding.involves_input_automaton (input2_handle.aid ()));

  binding.execute ();

  mu_assert (output_instance.p_uv_output.state);
  mu_assert (output_instance.p_uv_output.last_parameter == output_parameter);
  mu_assert (input1_instance.up_uv_input.state);
  mu_assert (input2_instance.p_uv_input.state);
  mu_assert (input2_instance.p_uv_input.last_parameter == input_parameter);

  binding.unbind (binder_handle.aid (), 1);

  mu_assert (!binding.empty ());
  mu_assert (binding.involves_output (output));
  mu_assert (!binding.involves_binding (output, input1, binder_handle.aid ()));
  mu_assert (binding.involves_binding (output, input2, binder_handle.aid ()));
  mu_assert (!binding.involves_input (input1));
  mu_assert (binding.involves_input (input2));
  mu_assert (!binding.involves_input_automaton (input1_handle.aid ()));
  mu_assert (binding.involves_input_automaton (input2_handle.aid ()));

  binding.unbind (binder_handle.aid (), 2);

  mu_assert (binding.empty ());
  mu_assert (!binding.involves_output (output));
  mu_assert (!binding.involves_binding (output, input1, binder_handle.aid ()));
  mu_assert (!binding.involves_binding (output, input2, binder_handle.aid ()));
  mu_assert (!binding.involves_input (input1));
  mu_assert (!binding.involves_input (input2));
  mu_assert (!binding.involves_input_automaton (input1_handle.aid ()));
  mu_assert (!binding.involves_input_automaton (input2_handle.aid ()));

  return 0;
}

static const char*
unbind_valued_unparameterized_output_action ()
{
  automaton1 binder_instance;
  automaton1 output_instance;
  automaton1 input1_instance;
  automaton1 input2_instance;
  ioa::automaton_handle<automaton1> binder_handle (0);
  ioa::automaton_handle<automaton1> output_handle (1);
  ioa::automaton_handle<automaton1> input1_handle (2);
  ioa::automaton_handle<automaton1> input2_handle (3);

  int input_parameter = 345;

  ioa::action<automaton1, automaton1::up_v_output_action> output (output_handle, &automaton1::up_v_output);
  ioa::action<automaton1, automaton1::up_v_input_action> input1 (input1_handle, &automaton1::up_v_input);
  ioa::action<automaton1, automaton1::p_v_input_action> input2 (input2_handle, &automaton1::p_v_input, input_parameter, ioa::parameterized ());

  ioa::binding<automaton1::up_v_output_action> binding;
  empty_class d;
  binding.bind (1, output_instance, output, input1_instance, input1, binder_instance, binder_handle.aid (), d);
  binding.bind (2, output_instance, output, input2_instance, input2, binder_instance, binder_handle.aid (), d);

  mu_assert (!binding.empty ());
  mu_assert (binding.involves_output (output));
  mu_assert (binding.involves_binding (output, input1, binder_handle.aid ()));
  mu_assert (binding.involves_binding (output, input2, binder_handle.aid ()));
  mu_assert (binding.involves_input (input1));
  mu_assert (binding.involves_input (input2));
  mu_assert (binding.involves_input_automaton (input1_handle.aid ()));
  mu_assert (binding.involves_input_automaton (input2_handle.aid ()));

  binding.execute ();

  mu_assert (output_instance.up_v_output.state);
  mu_assert (input1_instance.up_v_input.value == 9845);
  mu_assert (input2_instance.p_v_input.value == 9845);
  mu_assert (input2_instance.p_v_input.last_parameter == input_parameter);

  binding.unbind (binder_handle.aid (), 1);

  mu_assert (!binding.empty ());
  mu_assert (binding.involves_output (output));
  mu_assert (!binding.involves_binding (output, input1, binder_handle.aid ()));
  mu_assert (binding.involves_binding (output, input2, binder_handle.aid ()));
  mu_assert (!binding.involves_input (input1));
  mu_assert (binding.involves_input (input2));
  mu_assert (!binding.involves_input_automaton (input1_handle.aid ()));
  mu_assert (binding.involves_input_automaton (input2_handle.aid ()));

  binding.unbind (binder_handle.aid (), 2);

  mu_assert (binding.empty ());
  mu_assert (!binding.involves_output (output));
  mu_assert (!binding.involves_binding (output, input1, binder_handle.aid ()));
  mu_assert (!binding.involves_binding (output, input2, binder_handle.aid ()));
  mu_assert (!binding.involves_input (input1));
  mu_assert (!binding.involves_input (input2));
  mu_assert (!binding.involves_input_automaton (input1_handle.aid ()));
  mu_assert (!binding.involves_input_automaton (input2_handle.aid ()));

  return 0;
}

static const char*
unbind_valued_parameterized_output_action ()
{
  automaton1 binder_instance;
  automaton1 output_instance;
  automaton1 input1_instance;
  automaton1 input2_instance;
  ioa::automaton_handle<automaton1> binder_handle (0);
  ioa::automaton_handle<automaton1> output_handle (1);
  ioa::automaton_handle<automaton1> input1_handle (2);
  ioa::automaton_handle<automaton1> input2_handle (3);

  int output_parameter = 123;
  int input_parameter = 345;

  ioa::action<automaton1, automaton1::p_v_output_action> output (output_handle, &automaton1::p_v_output, output_parameter, ioa::parameterized ());
  ioa::action<automaton1, automaton1::up_v_input_action> input1 (input1_handle, &automaton1::up_v_input);
  ioa::action<automaton1, automaton1::p_v_input_action> input2 (input2_handle, &automaton1::p_v_input, input_parameter, ioa::parameterized ());

  ioa::binding<automaton1::p_v_output_action> binding;
  empty_class d;
  binding.bind (1, output_instance, output, input1_instance, input1, binder_instance, binder_handle.aid (), d);
  binding.bind (2, output_instance, output, input2_instance, input2, binder_instance, binder_handle.aid (), d);

  binding.execute ();

  mu_assert (output_instance.p_v_output.state);
  mu_assert (output_instance.p_v_output.last_parameter == output_parameter);
  mu_assert (input1_instance.up_v_input.value == 9845);
  mu_assert (input2_instance.p_v_input.value == 9845);
  mu_assert (input2_instance.p_v_input.last_parameter == input_parameter);

  binding.unbind (binder_handle.aid (), 1);

  mu_assert (!binding.empty ());
  mu_assert (binding.involves_output (output));
  mu_assert (!binding.involves_binding (output, input1, binder_handle.aid ()));
  mu_assert (binding.involves_binding (output, input2, binder_handle.aid ()));
  mu_assert (!binding.involves_input (input1));
  mu_assert (binding.involves_input (input2));
  mu_assert (!binding.involves_input_automaton (input1_handle.aid ()));
  mu_assert (binding.involves_input_automaton (input2_handle.aid ()));

  binding.unbind (binder_handle.aid (), 2);

  mu_assert (binding.empty ());
  mu_assert (!binding.involves_output (output));
  mu_assert (!binding.involves_binding (output, input1, binder_handle.aid ()));
  mu_assert (!binding.involves_binding (output, input2, binder_handle.aid ()));
  mu_assert (!binding.involves_input (input1));
  mu_assert (!binding.involves_input (input2));
  mu_assert (!binding.involves_input_automaton (input1_handle.aid ()));
  mu_assert (!binding.involves_input_automaton (input2_handle.aid ()));

  return 0;
}

const char*
all_tests ()
{
  mu_run_test (unbind_unvalued_unparameterized_output_action);
  mu_run_test (unbind_unvalued_parameterized_output_action);
  mu_run_test (unbind_valued_unparameterized_output_action);
  mu_run_test (unbind_valued_parameterized_output_action);

  return 0;
}
