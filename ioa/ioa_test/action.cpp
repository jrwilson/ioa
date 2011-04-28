#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE action
#include <boost/test/unit_test.hpp>

#include "automaton.hpp"

BOOST_AUTO_TEST_SUITE(action_suite)

BOOST_AUTO_TEST_CASE(unparameterized_untyped_input_action)
{
  ioa::locker<ioa::automaton_interface*> automata;
  automaton* z = new automaton ();
  ioa::automaton<automaton> a (z);
  ioa::automaton_handle<automaton> a_h = automata.insert (&a);
  ioa::action<automaton::up_ut_input_action> action (a_h, z->up_ut_input, a_h);
  BOOST_CHECK (a_h == action.get_automaton_handle ());
  BOOST_CHECK (action == action);
  BOOST_CHECK (a_h == action.get_composer_handle ());
  action ();
  BOOST_CHECK (z->up_ut_input.state);
}

BOOST_AUTO_TEST_CASE(parameterized_untyped_input_action)
{
  ioa::locker<ioa::automaton_interface*> automata;
  automaton* z = new automaton ();
  ioa::automaton<automaton> a (z);
  ioa::automaton_handle<automaton> a_h = automata.insert (&a);

  int parameter;
  ioa::parameter_handle<int> p_h = a.declare_parameter (&parameter);

  ioa::action<automaton::p_ut_input_action> action (a_h, z->p_ut_input, p_h, a_h);

  BOOST_CHECK (a_h == action.get_automaton_handle ()); 
  BOOST_CHECK (action == action);
  BOOST_CHECK (a_h == action.get_composer_handle ());
  action ();
  BOOST_CHECK (z->p_ut_input.state);
  BOOST_CHECK_EQUAL (&parameter, z->p_ut_input.last_parameter);
}

BOOST_AUTO_TEST_CASE(unparameterized_typed_input_action)
{
  ioa::locker<ioa::automaton_interface*> automata;
  automaton* z = new automaton ();
  ioa::automaton<automaton> a (z);
  ioa::automaton_handle<automaton> a_h = automata.insert (&a);
  ioa::action<automaton::up_t_input_action> action (a_h, z->up_t_input, a_h);
  BOOST_CHECK (a_h == action.get_automaton_handle ()); 
  BOOST_CHECK (action == action);
  BOOST_CHECK (a_h == action.get_composer_handle ());
  action (9845);
  BOOST_CHECK_EQUAL (9845, z->up_t_input.value);
}

BOOST_AUTO_TEST_CASE(parameterized_typed_input_action)
{
  ioa::locker<ioa::automaton_interface*> automata;
  automaton* z = new automaton ();
  ioa::automaton<automaton> a (z);
  ioa::automaton_handle<automaton> a_h = automata.insert (&a);

  int parameter;
  ioa::parameter_handle<int> p_h = a.declare_parameter (&parameter);

  ioa::action<automaton::p_t_input_action> action (a_h, z->p_t_input, p_h, a_h);

  BOOST_CHECK (a_h == action.get_automaton_handle ()); 
  BOOST_CHECK (action == action);
  BOOST_CHECK (a_h == action.get_composer_handle ());
  action (9845);
  BOOST_CHECK_EQUAL (9845, z->p_t_input.value);
  BOOST_CHECK_EQUAL (&parameter, z->p_t_input.last_parameter);
}

BOOST_AUTO_TEST_CASE(unparameterized_untyped_output_action)
{
  ioa::locker<ioa::automaton_interface*> automata;
  automaton* z = new automaton ();
  ioa::automaton<automaton> a (z);
  ioa::automaton_handle<automaton> a_h = automata.insert (&a);
  ioa::action<automaton::up_ut_output_action> action (a_h, z->up_ut_output);

  BOOST_CHECK (a_h == action.get_automaton_handle ());
  BOOST_CHECK (action == action);
  bool r = action ();
  BOOST_CHECK (r);
  BOOST_CHECK (z->up_ut_output.state);
}

BOOST_AUTO_TEST_CASE(parameterized_untyped_output_action)
{
  ioa::locker<ioa::automaton_interface*> automata;
  automaton* z = new automaton ();
  ioa::automaton<automaton> a (z);
  ioa::automaton_handle<automaton> a_h = automata.insert (&a);

  int parameter;
  ioa::parameter_handle<int> p_h = a.declare_parameter (&parameter);

  ioa::action<automaton::p_ut_output_action> action (a_h, z->p_ut_output, p_h);

  BOOST_CHECK (a_h == action.get_automaton_handle ()); 
  BOOST_CHECK (action == action);
  bool r = action ();
  BOOST_CHECK (r);
  BOOST_CHECK (z->p_ut_output.state);
  BOOST_CHECK_EQUAL (&parameter, z->p_ut_output.last_parameter);
}

BOOST_AUTO_TEST_CASE(unparameterized_typed_output_action)
{
  ioa::locker<ioa::automaton_interface*> automata;
  automaton* z = new automaton ();
  ioa::automaton<automaton> a (z);
  ioa::automaton_handle<automaton> a_h = automata.insert (&a);
  ioa::action<automaton::up_t_output_action> action (a_h, z->up_t_output);
  BOOST_CHECK (a_h == action.get_automaton_handle ()); 
  BOOST_CHECK (action == action);
  std::pair<bool, int> r = action ();
  BOOST_CHECK (r.first);
  BOOST_CHECK_EQUAL (r.second, 9845);
  BOOST_CHECK (z->up_t_output.state);
}

BOOST_AUTO_TEST_CASE(parameterized_typed_output_action)
{
  ioa::locker<ioa::automaton_interface*> automata;
  automaton* z = new automaton ();
  ioa::automaton<automaton> a (z);
  ioa::automaton_handle<automaton> a_h = automata.insert (&a);

  int parameter;
  ioa::parameter_handle<int> p_h = a.declare_parameter (&parameter);

  ioa::action<automaton::p_t_output_action> action (a_h, z->p_t_output, p_h);

  BOOST_CHECK (a_h == action.get_automaton_handle ()); 
  BOOST_CHECK (action == action);
  std::pair<bool, int> r = action ();
  BOOST_CHECK (r.first);
  BOOST_CHECK_EQUAL (r.second, 9845);
  BOOST_CHECK (z->p_t_output.state);
  BOOST_CHECK_EQUAL (&parameter, z->p_t_output.last_parameter);
}

BOOST_AUTO_TEST_SUITE_END()
