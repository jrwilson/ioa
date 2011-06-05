#include "minunit.h"

#include "automaton1.hpp"

#include "test_system_scheduler.hpp"

static const char*
unvalued_unparameterized_input_action ()
{
  ioa::automaton_handle<automaton1> h;
  ioa::action<automaton1, automaton1::up_uv_input_action> action (h, &automaton1::up_uv_input);
  mu_assert (action.get_aid () == h);
  automaton1* i = 0;
  mu_assert (&((*i).*(&automaton1::up_uv_input)) == action.get_member_ptr ());
  mu_assert (0U == action.get_pid ());
  mu_assert (action == action);
  
  automaton1 z;
  action (z);
  mu_assert (z.up_uv_input.state);
  action.bound (z);
  mu_assert (z.up_uv_input.bound_);
  action.unbound (z);
  mu_assert (z.up_uv_input.unbound_);

  return 0;
}

static const char*
unvalued_parameterized_input_action ()
{
  int parameter = 345;
  ioa::automaton_handle<automaton1> h;
  ioa::action<automaton1, automaton1::p_uv_input_action> action (h, &automaton1::p_uv_input, parameter, ioa::parameterized ());
  mu_assert (action.get_aid () == h);
  automaton1* i = 0;
  mu_assert (&((*i).*(&automaton1::p_uv_input)) == action.get_member_ptr ());
  mu_assert (size_t (parameter) == action.get_pid ());
  mu_assert (action == action);

  automaton1 z;
  action (z);
  mu_assert (z.p_uv_input.state);
  mu_assert (parameter == z.p_uv_input.last_parameter);
  action.bound (z);
  mu_assert (z.p_uv_input.bound_);
  mu_assert (parameter == z.p_uv_input.bound_parameter);
  action.unbound (z);
  mu_assert (z.p_uv_input.unbound_);
  mu_assert (parameter == z.p_uv_input.unbound_parameter);

  return 0;
}

static const char*
valued_unparameterized_input_action ()
{
  ioa::automaton_handle<automaton1> h;
  ioa::action<automaton1, automaton1::up_v_input_action> action (h, &automaton1::up_v_input);
  mu_assert (action.get_aid () == h);
  automaton1* i = 0;
  mu_assert (&((*i).*(&automaton1::up_v_input)) == action.get_member_ptr ());
  mu_assert (0U == action.get_pid ());
  mu_assert (action == action);

  automaton1 z;
  action (z, 9845);
  mu_assert (9845 == z.up_v_input.value);
  action.bound (z);
  mu_assert (z.up_v_input.bound_);
  action.unbound (z);
  mu_assert (z.up_v_input.unbound_);

  return 0;
}

static const char*
valued_parameterized_input_action ()
{
  int parameter = 345;
  ioa::automaton_handle<automaton1> h;
  ioa::action<automaton1, automaton1::p_v_input_action> action (h, &automaton1::p_v_input, parameter, ioa::parameterized ());
  mu_assert (action.get_aid () == h);
  automaton1* i = 0;
  mu_assert (&((*i).*(&automaton1::p_v_input)) == action.get_member_ptr ());
  mu_assert (size_t (parameter) == action.get_pid ());
  mu_assert (action == action);

  automaton1 z;
  action (z, 9845);
  mu_assert (9845 == z.p_v_input.value);
  mu_assert (parameter == z.p_v_input.last_parameter);
  action.bound (z);
  mu_assert (z.p_v_input.bound_);
  mu_assert (parameter == z.p_v_input.bound_parameter);
  action.unbound (z);
  mu_assert (z.p_v_input.unbound_);
  mu_assert (parameter == z.p_v_input.unbound_parameter);

  return 0;
}

static const char*
unvalued_unparameterized_output_action ()
{
//   ioa::automaton_handle<automaton1> h;
//   ioa::action<automaton1, automaton1::up_uv_output_action> action (h, &automaton1::up_uv_output);
//   mu_assert (h.aid (), action.get_aid ());
//   automaton1* i = 0;
//   mu_assert (&((*i).*(&automaton1::up_uv_output)), action.get_member_ptr ());
//   mu_assert (0U, action.get_pid ());
//   mu_assert (action == action);

//   automaton1 z;
//   bool r = action (z);
//   mu_assert (r);
//   mu_assert (z.up_uv_output.state);
//   action.bound (z);
//   mu_assert (z.up_uv_output.bound_);
//   action.unbound (z);
//   mu_assert (z.up_uv_output.unbound_);

  return 0;
}

static const char*
unvalued_parameterized_output_action ()
{
//   int parameter = 345;
//   ioa::automaton_handle<automaton1> h;
//   ioa::action<automaton1, automaton1::p_uv_output_action> action (h, &automaton1::p_uv_output, parameter, ioa::parameterized ());
//   mu_assert (h.aid (), action.get_aid ());
//   automaton1* i = 0;
//   mu_assert (&((*i).*(&automaton1::p_uv_output)), action.get_member_ptr ());
//   mu_assert (size_t (parameter), action.get_pid ());
//   mu_assert (action == action);

//   automaton1 z;
//   bool r = action (z);
//   mu_assert (r);
//   mu_assert (z.p_uv_output.state);
//   mu_assert (parameter, z.p_uv_output.last_parameter);
//   action.bound (z);
//   mu_assert (z.p_uv_output.bound_);
//   mu_assert (parameter, z.p_uv_output.bound_parameter);
//   action.unbound (z);
//   mu_assert (z.p_uv_output.unbound_);
//   mu_assert (parameter, z.p_uv_output.unbound_parameter);

  return 0;
}

static const char*
valued_unparameterized_output_action ()
{
//   ioa::automaton_handle<automaton1> h;
//   ioa::action<automaton1, automaton1::up_v_output_action> action (h, &automaton1::up_v_output);
//   mu_assert (h.aid (), action.get_aid ());
//   automaton1* i = 0;
//   mu_assert (&((*i).*(&automaton1::up_v_output)), action.get_member_ptr ());
//   mu_assert (0U, action.get_pid ());
//   mu_assert (action == action);

//   automaton1 z;
//   std::pair<bool, int> r = action (z);
//   mu_assert (r.first);
//   mu_assert (r.second, 9845);
//   mu_assert (z.up_v_output.state);
//   action.bound (z);
//   mu_assert (z.up_v_output.bound_);
//   action.unbound (z);
//   mu_assert (z.up_v_output.unbound_);

  return 0;
}

static const char*
valued_parameterized_output_action ()
{
//   int parameter = 345;
//   ioa::automaton_handle<automaton1> h;
//   ioa::action<automaton1, automaton1::p_v_output_action> action (h, &automaton1::p_v_output, parameter, ioa::parameterized ());
//   mu_assert (h.aid (), action.get_aid ());
//   automaton1* i = 0;
//   mu_assert (&((*i).*(&automaton1::p_v_output)), action.get_member_ptr ());
//   mu_assert (size_t (parameter), action.get_pid ());
//   mu_assert (action == action);

//   automaton1 z;
//   std::pair<bool, int> r = action (z);
//   mu_assert (r.first);
//   mu_assert (r.second, 9845);
//   mu_assert (z.p_v_output.state);
//   mu_assert (parameter, z.p_v_output.last_parameter);
//   action.bound (z);
//   mu_assert (z.p_v_output.bound_);
//   mu_assert (parameter, z.p_v_output.bound_parameter);
//   action.unbound (z);
//   mu_assert (z.p_v_output.unbound_);
//   mu_assert (parameter, z.p_v_output.unbound_parameter);

  return 0;
}

static const char*
unparameterized_internal_action ()
{
//   ioa::automaton_handle<automaton1> h;
//   ioa::action<automaton1, automaton1::up_internal_action> action (h, &automaton1::up_internal);
//   mu_assert (h.aid (), action.get_aid ());
//   automaton1* i = 0;
//   mu_assert (&((*i).*(&automaton1::up_internal)), action.get_member_ptr ());
//   mu_assert (0U, action.get_pid ());
//   mu_assert (action == action);

//   automaton1 z;
//   action (z);
//   mu_assert (z.up_internal.state);

  return 0;
}

static const char*
parameterized_internal_action ()
{
//   int parameter = 345;
//   ioa::automaton_handle<automaton1> h;
//   ioa::action<automaton1, automaton1::p_internal_action> action (h, &automaton1::p_internal, parameter, ioa::parameterized ());
//   mu_assert (h.aid (), action.get_aid ());
//   automaton1* i = 0;
//   mu_assert (&((*i).*(&automaton1::p_internal)), action.get_member_ptr ());
//   mu_assert (size_t (parameter), action.get_pid ());
//   mu_assert (action == action);

//   automaton1 z;
//   action (z);
//   mu_assert (z.p_internal.state);
//   mu_assert (parameter, z.p_internal.last_parameter);

  return 0;
}

static const char*
unvalued_event_action ()
{
//   ioa::automaton_handle<automaton1> h;
//   ioa::action<automaton1, automaton1::uv_event_action> action (h, &automaton1::uv_event);
//   mu_assert (h.aid (), action.get_aid ());
//   automaton1* i = 0;
//   mu_assert (&((*i).*(&automaton1::uv_event)), action.get_member_ptr ());
//   mu_assert (0U, action.get_pid ());
//   mu_assert (action != action);

//   automaton1 z;
//   action (z);
//   mu_assert (z.uv_event.state);

  return 0;
}

static const char*
valued_event_action ()
{
//   ioa::automaton_handle<automaton1> h;
//   ioa::action<automaton1, automaton1::v_event_action> action (h, &automaton1::v_event, 345, ioa::unparameterized ());
//   mu_assert (h.aid (), action.get_aid ());
//   automaton1* i = 0;
//   mu_assert (&((*i).*(&automaton1::v_event)), action.get_member_ptr ());
//   mu_assert (0U, action.get_pid ());
//   mu_assert (action != action);

//   automaton1 z;
//   action (z);
//   mu_assert (z.v_event.state);
//   mu_assert (345, z.v_event.last_value);

  return 0;
}

const char*
all_tests ()
{
  mu_run_test (unvalued_unparameterized_input_action);
  mu_run_test (unvalued_parameterized_input_action);
  mu_run_test (valued_unparameterized_input_action);
  mu_run_test (valued_parameterized_input_action);
  mu_run_test (unvalued_unparameterized_output_action);
  mu_run_test (unvalued_parameterized_output_action);
  mu_run_test (valued_unparameterized_output_action);
  mu_run_test (valued_parameterized_output_action);
  mu_run_test (unparameterized_internal_action);
  mu_run_test (parameterized_internal_action);
  mu_run_test (unvalued_event_action);
  mu_run_test (valued_event_action);

  return 0;
}
