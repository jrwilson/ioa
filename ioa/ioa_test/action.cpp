#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE action
#include <boost/test/unit_test.hpp>

#include "automaton1.hpp"

BOOST_AUTO_TEST_SUITE(action_suite)

BOOST_AUTO_TEST_CASE(unparameterized_unvalued_input_action)
{
  std::auto_ptr<automaton1> z (new automaton1 ());
  ioa::automaton_handle<automaton1> a_h (z.get ());
  ioa::action<automaton1, automaton1::up_uv_input_action> action (a_h, &automaton1::up_uv_input);
  BOOST_CHECK (a_h == action.get_automaton_handle ());
  BOOST_CHECK (action == action);
  action ();
  BOOST_CHECK (z->up_uv_input.state);
}

BOOST_AUTO_TEST_CASE(parameterized_unvalued_input_action)
{
  std::auto_ptr<automaton1> z (new automaton1 ());
  ioa::automaton_handle<automaton1> a_h (z.get ());
  int parameter;
  ioa::parameter_handle<int> p_h (&parameter);
  ioa::action<automaton1, automaton1::p_uv_input_action> action (a_h, &automaton1::p_uv_input, p_h);
  BOOST_CHECK (a_h == action.get_automaton_handle ()); 
  BOOST_CHECK (action == action);
  action ();
  BOOST_CHECK (z->p_uv_input.state);
  BOOST_CHECK_EQUAL (&parameter, z->p_uv_input.last_parameter);
}

BOOST_AUTO_TEST_CASE(unparameterized_valued_input_action)
{
  std::auto_ptr<automaton1> z (new automaton1 ());
  ioa::automaton_handle<automaton1> a_h (z.get ());
  ioa::action<automaton1, automaton1::up_v_input_action> action (a_h, &automaton1::up_v_input);
  BOOST_CHECK (a_h == action.get_automaton_handle ()); 
  BOOST_CHECK (action == action);
  action (9845);
  BOOST_CHECK_EQUAL (9845, z->up_v_input.value);
}

BOOST_AUTO_TEST_CASE(parameterized_valued_input_action)
{
  std::auto_ptr<automaton1> z (new automaton1 ());
  ioa::automaton_handle<automaton1> a_h (z.get ());
  int parameter;
  ioa::parameter_handle<int> p_h (&parameter);
  ioa::action<automaton1, automaton1::p_v_input_action> action (a_h, &automaton1::p_v_input, p_h);
  BOOST_CHECK (a_h == action.get_automaton_handle ()); 
  BOOST_CHECK (action == action);
  action (9845);
  BOOST_CHECK_EQUAL (9845, z->p_v_input.value);
  BOOST_CHECK_EQUAL (&parameter, z->p_v_input.last_parameter);
}

BOOST_AUTO_TEST_CASE(unparameterized_unvalued_output_action)
{
  std::auto_ptr<automaton1> z (new automaton1 ());
  ioa::automaton_handle<automaton1> a_h (z.get ());
  ioa::action<automaton1, automaton1::up_uv_output_action> action (a_h, &automaton1::up_uv_output);
  BOOST_CHECK (a_h == action.get_automaton_handle ());
  BOOST_CHECK (action == action);
  bool r = action ();
  BOOST_CHECK (r);
  BOOST_CHECK (z->up_uv_output.state);
}

BOOST_AUTO_TEST_CASE(parameterized_unvalued_output_action)
{
  std::auto_ptr<automaton1> z (new automaton1 ());
  ioa::automaton_handle<automaton1> a_h (z.get ());
  int parameter;
  ioa::parameter_handle<int> p_h (&parameter);
  ioa::action<automaton1, automaton1::p_uv_output_action> action (a_h, &automaton1::p_uv_output, p_h);
  BOOST_CHECK (a_h == action.get_automaton_handle ()); 
  BOOST_CHECK (action == action);
  bool r = action ();
  BOOST_CHECK (r);
  BOOST_CHECK (z->p_uv_output.state);
  BOOST_CHECK_EQUAL (&parameter, z->p_uv_output.last_parameter);
}

BOOST_AUTO_TEST_CASE(unparameterized_valued_output_action)
{
  std::auto_ptr<automaton1> z (new automaton1 ());
  ioa::automaton_handle<automaton1> a_h (z.get ());
  ioa::action<automaton1, automaton1::up_v_output_action> action (a_h, &automaton1::up_v_output);
  BOOST_CHECK (a_h == action.get_automaton_handle ()); 
  BOOST_CHECK (action == action);
  std::pair<bool, int> r = action ();
  BOOST_CHECK (r.first);
  BOOST_CHECK_EQUAL (r.second, 9845);
  BOOST_CHECK (z->up_v_output.state);
}

BOOST_AUTO_TEST_CASE(parameterized_valued_output_action)
{
  std::auto_ptr<automaton1> z (new automaton1 ());
  ioa::automaton_handle<automaton1> a_h (z.get ());
  int parameter;
  ioa::parameter_handle<int> p_h (&parameter);
  ioa::action<automaton1, automaton1::p_v_output_action> action (a_h, &automaton1::p_v_output, p_h);
  BOOST_CHECK (a_h == action.get_automaton_handle ()); 
  BOOST_CHECK (action == action);
  std::pair<bool, int> r = action ();
  BOOST_CHECK (r.first);
  BOOST_CHECK_EQUAL (r.second, 9845);
  BOOST_CHECK (z->p_v_output.state);
  BOOST_CHECK_EQUAL (&parameter, z->p_v_output.last_parameter);
}

BOOST_AUTO_TEST_CASE(unparameterized_internal_action)
{
  std::auto_ptr<automaton1> z (new automaton1 ());
  ioa::automaton_handle<automaton1> a_h (z.get ());
  ioa::action<automaton1, automaton1::up_internal_action> action (a_h, &automaton1::up_internal);
  BOOST_CHECK (a_h == action.get_automaton_handle ());
  action.execute ();
  BOOST_CHECK (z->up_internal.state);
}

BOOST_AUTO_TEST_CASE(parameterized_internal_action)
{
  std::auto_ptr<automaton1> z (new automaton1 ());
  ioa::automaton_handle<automaton1> a_h (z.get ());
  int parameter;
  ioa::parameter_handle<int> p_h (&parameter);
  ioa::action<automaton1, automaton1::p_internal_action> action (a_h, &automaton1::p_internal, p_h);
  BOOST_CHECK (a_h == action.get_automaton_handle ());
  action.execute ();
  BOOST_CHECK (z->p_internal.state);
  BOOST_CHECK_EQUAL (&parameter, z->p_internal.last_parameter);
}

BOOST_AUTO_TEST_CASE(unvalued_event_action)
{
  std::auto_ptr<automaton1> z (new automaton1 ());
  ioa::automaton_handle<automaton1> a_h (z.get ());
  ioa::action<automaton1, automaton1::uv_event_action> action (a_h, &automaton1::uv_event);
  BOOST_CHECK (a_h == action.get_automaton_handle ());
  action.execute ();
  BOOST_CHECK (z->uv_event.state);
}

BOOST_AUTO_TEST_CASE(valued_event_action)
{
  std::auto_ptr<automaton1> z (new automaton1 ());
  ioa::automaton_handle<automaton1> a_h (z.get ());
  ioa::action<automaton1, automaton1::v_event_action> action (a_h, &automaton1::v_event, 9845);
  BOOST_CHECK (a_h == action.get_automaton_handle ());
  action.execute ();
  BOOST_CHECK (z->v_event.state);
  BOOST_CHECK_EQUAL (z->v_event.last_value, 9845);
}

BOOST_AUTO_TEST_SUITE_END()
