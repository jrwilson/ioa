#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE action
#include <boost/test/unit_test.hpp>

#include "automaton1.hpp"

BOOST_AUTO_TEST_SUITE(action_suite)

BOOST_AUTO_TEST_CASE(unparameterized_unvalued_input_action)
{
  ioa::automaton_handle<automaton1> h;
  ioa::action<automaton1, automaton1::up_uv_input_action> action (h, &automaton1::up_uv_input);
  BOOST_CHECK_EQUAL (h.aid (), action.get_aid ());
  automaton1* i = 0;
  BOOST_CHECK_EQUAL (&((*i).*(&automaton1::up_uv_input)), action.get_member_ptr ());
  BOOST_CHECK_EQUAL (0U, action.get_pid ());
  BOOST_CHECK (action == action);

  automaton1 z;
  action (z);
  BOOST_CHECK (z.up_uv_input.state);
  action.bound (z);
  BOOST_CHECK (z.up_uv_input.bound_);
  action.unbound (z);
  BOOST_CHECK (z.up_uv_input.unbound_);
}

BOOST_AUTO_TEST_CASE(parameterized_unvalued_input_action)
{
  int parameter = 345;
  ioa::automaton_handle<automaton1> h;
  ioa::action<automaton1, automaton1::p_uv_input_action> action (h, &automaton1::p_uv_input, parameter, ioa::parameterized ());
  BOOST_CHECK_EQUAL (h.aid (), action.get_aid ());
  automaton1* i = 0;
  BOOST_CHECK_EQUAL (&((*i).*(&automaton1::p_uv_input)), action.get_member_ptr ());
  BOOST_CHECK_EQUAL (size_t (parameter), action.get_pid ());
  BOOST_CHECK (action == action);

  automaton1 z;
  action (z);
  BOOST_CHECK (z.p_uv_input.state);
  BOOST_CHECK_EQUAL (parameter, z.p_uv_input.last_parameter);
  action.bound (z);
  BOOST_CHECK (z.p_uv_input.bound_);
  BOOST_CHECK_EQUAL (parameter, z.p_uv_input.bound_parameter);
  action.unbound (z);
  BOOST_CHECK (z.p_uv_input.unbound_);
  BOOST_CHECK_EQUAL (parameter, z.p_uv_input.unbound_parameter);
}

BOOST_AUTO_TEST_CASE(unparameterized_valued_input_action)
{
  ioa::automaton_handle<automaton1> h;
  ioa::action<automaton1, automaton1::up_v_input_action> action (h, &automaton1::up_v_input);
  BOOST_CHECK_EQUAL (h.aid (), action.get_aid ());
  automaton1* i = 0;
  BOOST_CHECK_EQUAL (&((*i).*(&automaton1::up_v_input)), action.get_member_ptr ());
  BOOST_CHECK_EQUAL (0U, action.get_pid ());
  BOOST_CHECK (action == action);

  automaton1 z;
  action (z, 9845);
  BOOST_CHECK_EQUAL (9845, z.up_v_input.value);
  action.bound (z);
  BOOST_CHECK (z.up_v_input.bound_);
  action.unbound (z);
  BOOST_CHECK (z.up_v_input.unbound_);
}

BOOST_AUTO_TEST_CASE(parameterized_valued_input_action)
{
  int parameter = 345;
  ioa::automaton_handle<automaton1> h;
  ioa::action<automaton1, automaton1::p_v_input_action> action (h, &automaton1::p_v_input, parameter, ioa::parameterized ());
  BOOST_CHECK_EQUAL (h.aid (), action.get_aid ());
  automaton1* i = 0;
  BOOST_CHECK_EQUAL (&((*i).*(&automaton1::p_v_input)), action.get_member_ptr ());
  BOOST_CHECK_EQUAL (size_t (parameter), action.get_pid ());
  BOOST_CHECK (action == action);

  automaton1 z;
  action (z, 9845);
  BOOST_CHECK_EQUAL (9845, z.p_v_input.value);
  BOOST_CHECK_EQUAL (parameter, z.p_v_input.last_parameter);
  action.bound (z);
  BOOST_CHECK (z.p_v_input.bound_);
  BOOST_CHECK_EQUAL (parameter, z.p_v_input.bound_parameter);
  action.unbound (z);
  BOOST_CHECK (z.p_v_input.unbound_);
  BOOST_CHECK_EQUAL (parameter, z.p_v_input.unbound_parameter);
}

BOOST_AUTO_TEST_CASE(unparameterized_unvalued_output_action)
{
  ioa::automaton_handle<automaton1> h;
  ioa::action<automaton1, automaton1::up_uv_output_action> action (h, &automaton1::up_uv_output);
  BOOST_CHECK_EQUAL (h.aid (), action.get_aid ());
  automaton1* i = 0;
  BOOST_CHECK_EQUAL (&((*i).*(&automaton1::up_uv_output)), action.get_member_ptr ());
  BOOST_CHECK_EQUAL (0U, action.get_pid ());
  BOOST_CHECK (action == action);

  automaton1 z;
  bool r = action (z);
  BOOST_CHECK (r);
  BOOST_CHECK (z.up_uv_output.state);
  action.bound (z);
  BOOST_CHECK (z.up_uv_output.bound_);
  action.unbound (z);
  BOOST_CHECK (z.up_uv_output.unbound_);
}

BOOST_AUTO_TEST_CASE(parameterized_unvalued_output_action)
{
  int parameter = 345;
  ioa::automaton_handle<automaton1> h;
  ioa::action<automaton1, automaton1::p_uv_output_action> action (h, &automaton1::p_uv_output, parameter, ioa::parameterized ());
  BOOST_CHECK_EQUAL (h.aid (), action.get_aid ());
  automaton1* i = 0;
  BOOST_CHECK_EQUAL (&((*i).*(&automaton1::p_uv_output)), action.get_member_ptr ());
  BOOST_CHECK_EQUAL (size_t (parameter), action.get_pid ());
  BOOST_CHECK (action == action);

  automaton1 z;
  bool r = action (z);
  BOOST_CHECK (r);
  BOOST_CHECK (z.p_uv_output.state);
  BOOST_CHECK_EQUAL (parameter, z.p_uv_output.last_parameter);
  action.bound (z);
  BOOST_CHECK (z.p_uv_output.bound_);
  BOOST_CHECK_EQUAL (parameter, z.p_uv_output.bound_parameter);
  action.unbound (z);
  BOOST_CHECK (z.p_uv_output.unbound_);
  BOOST_CHECK_EQUAL (parameter, z.p_uv_output.unbound_parameter);
}

BOOST_AUTO_TEST_CASE(unparameterized_valued_output_action)
{
  ioa::automaton_handle<automaton1> h;
  ioa::action<automaton1, automaton1::up_v_output_action> action (h, &automaton1::up_v_output);
  BOOST_CHECK_EQUAL (h.aid (), action.get_aid ());
  automaton1* i = 0;
  BOOST_CHECK_EQUAL (&((*i).*(&automaton1::up_v_output)), action.get_member_ptr ());
  BOOST_CHECK_EQUAL (0U, action.get_pid ());
  BOOST_CHECK (action == action);

  automaton1 z;
  std::pair<bool, int> r = action (z);
  BOOST_CHECK (r.first);
  BOOST_CHECK_EQUAL (r.second, 9845);
  BOOST_CHECK (z.up_v_output.state);
  action.bound (z);
  BOOST_CHECK (z.up_v_output.bound_);
  action.unbound (z);
  BOOST_CHECK (z.up_v_output.unbound_);
}

BOOST_AUTO_TEST_CASE(parameterized_valued_output_action)
{
  int parameter = 345;
  ioa::automaton_handle<automaton1> h;
  ioa::action<automaton1, automaton1::p_v_output_action> action (h, &automaton1::p_v_output, parameter, ioa::parameterized ());
  BOOST_CHECK_EQUAL (h.aid (), action.get_aid ());
  automaton1* i = 0;
  BOOST_CHECK_EQUAL (&((*i).*(&automaton1::p_v_output)), action.get_member_ptr ());
  BOOST_CHECK_EQUAL (size_t (parameter), action.get_pid ());
  BOOST_CHECK (action == action);

  automaton1 z;
  std::pair<bool, int> r = action (z);
  BOOST_CHECK (r.first);
  BOOST_CHECK_EQUAL (r.second, 9845);
  BOOST_CHECK (z.p_v_output.state);
  BOOST_CHECK_EQUAL (parameter, z.p_v_output.last_parameter);
  action.bound (z);
  BOOST_CHECK (z.p_v_output.bound_);
  BOOST_CHECK_EQUAL (parameter, z.p_v_output.bound_parameter);
  action.unbound (z);
  BOOST_CHECK (z.p_v_output.unbound_);
  BOOST_CHECK_EQUAL (parameter, z.p_v_output.unbound_parameter);
}

BOOST_AUTO_TEST_CASE(unparameterized_internal_action)
{
  ioa::automaton_handle<automaton1> h;
  ioa::action<automaton1, automaton1::up_internal_action> action (h, &automaton1::up_internal);
  BOOST_CHECK_EQUAL (h.aid (), action.get_aid ());
  automaton1* i = 0;
  BOOST_CHECK_EQUAL (&((*i).*(&automaton1::up_internal)), action.get_member_ptr ());
  BOOST_CHECK_EQUAL (0U, action.get_pid ());
  BOOST_CHECK (action == action);

  automaton1 z;
  action.execute (z);
  BOOST_CHECK (z.up_internal.state);
}

BOOST_AUTO_TEST_CASE(parameterized_internal_action)
{
  int parameter = 345;
  ioa::automaton_handle<automaton1> h;
  ioa::action<automaton1, automaton1::p_internal_action> action (h, &automaton1::p_internal, parameter, ioa::parameterized ());
  BOOST_CHECK_EQUAL (h.aid (), action.get_aid ());
  automaton1* i = 0;
  BOOST_CHECK_EQUAL (&((*i).*(&automaton1::p_internal)), action.get_member_ptr ());
  BOOST_CHECK_EQUAL (size_t (parameter), action.get_pid ());
  BOOST_CHECK (action == action);

  automaton1 z;
  action.execute (z);
  BOOST_CHECK (z.p_internal.state);
  BOOST_CHECK_EQUAL (parameter, z.p_internal.last_parameter);
}

BOOST_AUTO_TEST_CASE(unvalued_event_action)
{
  ioa::automaton_handle<automaton1> h;
  ioa::action<automaton1, automaton1::uv_event_action> action (h, &automaton1::uv_event);
  BOOST_CHECK_EQUAL (h.aid (), action.get_aid ());
  automaton1* i = 0;
  BOOST_CHECK_EQUAL (&((*i).*(&automaton1::uv_event)), action.get_member_ptr ());
  BOOST_CHECK_EQUAL (0U, action.get_pid ());
  BOOST_CHECK (action != action);

  automaton1 z;
  action.execute (z);
  BOOST_CHECK (z.uv_event.state);
}

BOOST_AUTO_TEST_CASE(valued_event_action)
{
  ioa::automaton_handle<automaton1> h;
  ioa::action<automaton1, automaton1::v_event_action> action (h, &automaton1::v_event, 345, ioa::unparameterized ());
  BOOST_CHECK_EQUAL (h.aid (), action.get_aid ());
  automaton1* i = 0;
  BOOST_CHECK_EQUAL (&((*i).*(&automaton1::v_event)), action.get_member_ptr ());
  BOOST_CHECK_EQUAL (0U, action.get_pid ());
  BOOST_CHECK (action != action);

  automaton1 z;
  action.execute (z);
  BOOST_CHECK (z.v_event.state);
  BOOST_CHECK_EQUAL (345, z.v_event.last_value);
}

BOOST_AUTO_TEST_SUITE_END()
