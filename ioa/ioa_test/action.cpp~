#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE action
#include <boost/test/unit_test.hpp>

#include "automaton.hpp"

BOOST_AUTO_TEST_SUITE(action_suite)

BOOST_AUTO_TEST_CASE(unparameterized_unvalued_input_action)
{
  ioa::locker<ioa::automaton_interface*> automata;
  automaton* z = new automaton ();
  ioa::automaton<automaton> a (z);
  ioa::automaton_handle<automaton> a_h = automata.insert (&a);
  ioa::action<automaton::up_uv_input_action> action (a_h, z->up_uv_input, a_h);
  BOOST_CHECK (a_h == action.get_automaton_handle ());
  BOOST_CHECK (action == action);
  BOOST_CHECK (a_h == action.get_composer_handle ());
  action ();
  BOOST_CHECK (z->up_uv_input.state);
}

BOOST_AUTO_TEST_CASE(parameterized_unvalued_input_action)
{
  ioa::locker<ioa::automaton_interface*> automata;
  automaton* z = new automaton ();
  ioa::automaton<automaton> a (z);
  ioa::automaton_handle<automaton> a_h = automata.insert (&a);

  int parameter;
  ioa::parameter_handle<int> p_h = a.declare_parameter (&parameter);

  ioa::action<automaton::p_uv_input_action> action (a_h, z->p_uv_input, p_h, a_h);

  BOOST_CHECK (a_h == action.get_automaton_handle ()); 
  BOOST_CHECK (action == action);
  BOOST_CHECK (a_h == action.get_composer_handle ());
  action ();
  BOOST_CHECK (z->p_uv_input.state);
  BOOST_CHECK_EQUAL (&parameter, z->p_uv_input.last_parameter);
}

BOOST_AUTO_TEST_CASE(unparameterized_valued_input_action)
{
  ioa::locker<ioa::automaton_interface*> automata;
  automaton* z = new automaton ();
  ioa::automaton<automaton> a (z);
  ioa::automaton_handle<automaton> a_h = automata.insert (&a);
  ioa::action<automaton::up_v_input_action> action (a_h, z->up_v_input, a_h);
  BOOST_CHECK (a_h == action.get_automaton_handle ()); 
  BOOST_CHECK (action == action);
  BOOST_CHECK (a_h == action.get_composer_handle ());
  action (9845);
  BOOST_CHECK_EQUAL (9845, z->up_v_input.value);
}

BOOST_AUTO_TEST_CASE(parameterized_valued_input_action)
{
  ioa::locker<ioa::automaton_interface*> automata;
  automaton* z = new automaton ();
  ioa::automaton<automaton> a (z);
  ioa::automaton_handle<automaton> a_h = automata.insert (&a);

  int parameter;
  ioa::parameter_handle<int> p_h = a.declare_parameter (&parameter);

  ioa::action<automaton::p_v_input_action> action (a_h, z->p_v_input, p_h, a_h);

  BOOST_CHECK (a_h == action.get_automaton_handle ()); 
  BOOST_CHECK (action == action);
  BOOST_CHECK (a_h == action.get_composer_handle ());
  action (9845);
  BOOST_CHECK_EQUAL (9845, z->p_v_input.value);
  BOOST_CHECK_EQUAL (&parameter, z->p_v_input.last_parameter);
}

BOOST_AUTO_TEST_CASE(unparameterized_unvalued_output_action)
{
  ioa::locker<ioa::automaton_interface*> automata;
  automaton* z = new automaton ();
  ioa::automaton<automaton> a (z);
  ioa::automaton_handle<automaton> a_h = automata.insert (&a);
  ioa::action<automaton::up_uv_output_action> action (a_h, z->up_uv_output);

  BOOST_CHECK (a_h == action.get_automaton_handle ());
  BOOST_CHECK (action == action);
  bool r = action ();
  BOOST_CHECK (r);
  BOOST_CHECK (z->up_uv_output.state);
}

BOOST_AUTO_TEST_CASE(parameterized_unvalued_output_action)
{
  ioa::locker<ioa::automaton_interface*> automata;
  automaton* z = new automaton ();
  ioa::automaton<automaton> a (z);
  ioa::automaton_handle<automaton> a_h = automata.insert (&a);

  int parameter;
  ioa::parameter_handle<int> p_h = a.declare_parameter (&parameter);

  ioa::action<automaton::p_uv_output_action> action (a_h, z->p_uv_output, p_h);

  BOOST_CHECK (a_h == action.get_automaton_handle ()); 
  BOOST_CHECK (action == action);
  bool r = action ();
  BOOST_CHECK (r);
  BOOST_CHECK (z->p_uv_output.state);
  BOOST_CHECK_EQUAL (&parameter, z->p_uv_output.last_parameter);
}

BOOST_AUTO_TEST_CASE(unparameterized_valued_output_action)
{
  ioa::locker<ioa::automaton_interface*> automata;
  automaton* z = new automaton ();
  ioa::automaton<automaton> a (z);
  ioa::automaton_handle<automaton> a_h = automata.insert (&a);
  ioa::action<automaton::up_v_output_action> action (a_h, z->up_v_output);
  BOOST_CHECK (a_h == action.get_automaton_handle ()); 
  BOOST_CHECK (action == action);
  std::pair<bool, int> r = action ();
  BOOST_CHECK (r.first);
  BOOST_CHECK_EQUAL (r.second, 9845);
  BOOST_CHECK (z->up_v_output.state);
}

BOOST_AUTO_TEST_CASE(parameterized_valued_output_action)
{
  ioa::locker<ioa::automaton_interface*> automata;
  automaton* z = new automaton ();
  ioa::automaton<automaton> a (z);
  ioa::automaton_handle<automaton> a_h = automata.insert (&a);

  int parameter;
  ioa::parameter_handle<int> p_h = a.declare_parameter (&parameter);

  ioa::action<automaton::p_v_output_action> action (a_h, z->p_v_output, p_h);

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
  ioa::locker<ioa::automaton_interface*> automata;
  automaton* z = new automaton ();
  ioa::automaton<automaton> a (z);
  ioa::automaton_handle<automaton> a_h = automata.insert (&a);
  ioa::action<automaton::up_internal_action> action (a_h, z->up_internal);

  BOOST_CHECK (a_h == action.get_automaton_handle ());
  action.execute ();
  BOOST_CHECK (z->up_internal.state);
}

BOOST_AUTO_TEST_CASE(parameterized_internal_action)
{
  ioa::locker<ioa::automaton_interface*> automata;
  automaton* z = new automaton ();
  ioa::automaton<automaton> a (z);
  ioa::automaton_handle<automaton> a_h = automata.insert (&a);

  int parameter;
  ioa::parameter_handle<int> p_h = a.declare_parameter (&parameter);

  ioa::action<automaton::p_internal_action> action (a_h, z->p_internal, p_h);

  BOOST_CHECK (a_h == action.get_automaton_handle ());
  action.execute ();
  BOOST_CHECK (z->p_internal.state);
  BOOST_CHECK_EQUAL (&parameter, z->p_internal.last_parameter);
}

BOOST_AUTO_TEST_CASE(unvalued_event_action)
{
  ioa::locker<ioa::automaton_interface*> automata;
  automaton* z = new automaton ();
  ioa::automaton<automaton> a (z);
  ioa::automaton_handle<automaton> a_h = automata.insert (&a);
  ioa::action<automaton::uv_event_action> action (a_h, z->uv_event);

  BOOST_CHECK (a_h == action.get_automaton_handle ());
  action.execute ();
  BOOST_CHECK (z->uv_event.state);
}

BOOST_AUTO_TEST_CASE(valued_event_action)
{
  ioa::locker<ioa::automaton_interface*> automata;
  automaton* z = new automaton ();
  ioa::automaton<automaton> a (z);
  ioa::automaton_handle<automaton> a_h = automata.insert (&a);
  ioa::action<automaton::v_event_action> action (a_h, z->v_event, 9845);

  BOOST_CHECK (a_h == action.get_automaton_handle ());
  action.execute ();
  BOOST_CHECK (z->v_event.state);
  BOOST_CHECK_EQUAL (z->v_event.last_value, 9845);
}

BOOST_AUTO_TEST_SUITE_END()
