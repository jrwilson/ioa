#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE action
#include <boost/test/unit_test.hpp>
#include <action.hpp>
#include "automaton.hpp"

struct callback {
  bool* ptr;

  callback (bool* ptr) :
    ptr (ptr)
  { }

  void operator() () {
    *ptr = true;
  }
};

BOOST_AUTO_TEST_SUITE(action_suite)

BOOST_AUTO_TEST_CASE(unparameterized_untyped_input_action)
{
  automaton*z = new automaton();
  ioa::typed_automaton<automaton> a (z);
  ioa::generic_automaton_handle h_a (&a);
  bool decomposed;
  callback cb (&decomposed);
  ioa::action<automaton::up_ut_input_action, callback> action (h_a, z->up_ut_input, h_a, cb);
  BOOST_CHECK_EQUAL (h_a, action.get_automaton_handle ()); 
  BOOST_CHECK_EQUAL (action, action);
  BOOST_CHECK_EQUAL (action.get_owner_handle (), h_a);
  action ();
  BOOST_CHECK (z->up_ut_input.state);
  action.decompose ();
  BOOST_CHECK (decomposed);
}

BOOST_AUTO_TEST_CASE(parameterized_untyped_input_action)
{
  automaton* z = new automaton ();
  ioa::typed_automaton<automaton> a (z);
  ioa::generic_automaton_handle h_a (&a);

  int parameter;
  a.declare (&parameter);
  ioa::generic_parameter_handle<int> h_p (&a, &parameter);

  bool decomposed;
  callback cb (&decomposed);
  ioa::action<automaton::p_ut_input_action, callback> action (h_p, z->p_ut_input, h_p, cb);

  BOOST_CHECK_EQUAL (h_a, action.get_automaton_handle ()); 
  BOOST_CHECK_EQUAL (action, action);
  BOOST_CHECK_EQUAL (action.get_owner_handle (), h_a);
  action ();
  BOOST_CHECK (z->p_ut_input.state);
  BOOST_CHECK_EQUAL (&parameter, z->p_ut_input.last_parameter);
  action.decompose ();
  BOOST_CHECK (decomposed);
}

BOOST_AUTO_TEST_CASE(unparameterized_typed_input_action)
{
  automaton*z = new automaton();
  ioa::typed_automaton<automaton> a (z);
  ioa::generic_automaton_handle h_a (&a);
  bool decomposed;
  callback cb (&decomposed);
  ioa::action<automaton::up_t_input_action, callback> action (h_a, z->up_t_input, h_a, cb);
  BOOST_CHECK_EQUAL (h_a, action.get_automaton_handle ()); 
  BOOST_CHECK_EQUAL (action, action);
  BOOST_CHECK_EQUAL (action.get_owner_handle (), h_a);
  action (9845);
  BOOST_CHECK_EQUAL (9845, z->up_t_input.value);
  action.decompose ();
  BOOST_CHECK (decomposed);
}

BOOST_AUTO_TEST_CASE(parameterized_typed_input_action)
{
  automaton* z = new automaton ();
  ioa::typed_automaton<automaton> a (z);
  ioa::generic_automaton_handle h_a (&a);

  int parameter;
  a.declare (&parameter);
  ioa::generic_parameter_handle<int> h_p (&a, &parameter);

  bool decomposed;
  callback cb (&decomposed);
  ioa::action<automaton::p_t_input_action, callback> action (h_p, z->p_t_input, h_p, cb);

  BOOST_CHECK_EQUAL (h_a, action.get_automaton_handle ()); 
  BOOST_CHECK_EQUAL (action, action);
  BOOST_CHECK_EQUAL (action.get_owner_handle (), h_a);
  action (9845);
  BOOST_CHECK_EQUAL (9845, z->p_t_input.value);
  BOOST_CHECK_EQUAL (&parameter, z->p_t_input.last_parameter);
  action.decompose ();
  BOOST_CHECK (decomposed);
}

BOOST_AUTO_TEST_CASE(unparameterized_untyped_output_action)
{
  automaton* z = new automaton ();
  ioa::typed_automaton<automaton> a (z);
  ioa::generic_automaton_handle h_a (&a);
  ioa::action<automaton::up_ut_output_action> action (h_a, z->up_ut_output);

  BOOST_CHECK_EQUAL (h_a, action.get_automaton_handle ());
  BOOST_CHECK_EQUAL (action, action);
  bool r = action ();
  BOOST_CHECK (r);
  BOOST_CHECK (z->up_ut_output.state);
}

BOOST_AUTO_TEST_CASE(parameterized_untyped_output_action)
{
  automaton* z = new automaton ();
  ioa::typed_automaton<automaton> a (z);
  ioa::generic_automaton_handle h_a (&a);

  int parameter;
  a.declare (&parameter);
  ioa::generic_parameter_handle<int> h_p (&a, &parameter);

  ioa::action<automaton::p_ut_output_action> action (h_p, z->p_ut_output);

  BOOST_CHECK_EQUAL (h_a, action.get_automaton_handle ()); 
  BOOST_CHECK_EQUAL (action, action);
  bool r = action ();
  BOOST_CHECK (r);
  BOOST_CHECK (z->p_ut_output.state);
  BOOST_CHECK_EQUAL (&parameter, z->p_ut_output.last_parameter);
}

BOOST_AUTO_TEST_CASE(unparameterized_typed_output_action)
{
  automaton* z = new automaton ();
  ioa::typed_automaton<automaton> a (z);
  ioa::generic_automaton_handle h_a (&a);
  ioa::action<automaton::up_t_output_action> action (h_a, z->up_t_output);
  BOOST_CHECK_EQUAL (h_a, action.get_automaton_handle ()); 
  BOOST_CHECK_EQUAL (action, action);
  std::pair<bool, int> r = action ();
  BOOST_CHECK (r.first);
  BOOST_CHECK_EQUAL (r.second, 9845);
  BOOST_CHECK (z->up_t_output.state);
}

BOOST_AUTO_TEST_CASE(parameterized_typed_output_action)
{
  automaton* z = new automaton ();
  ioa::typed_automaton<automaton> a (z);
  ioa::generic_automaton_handle h_a (&a);

  int parameter;
  a.declare (&parameter);
  ioa::generic_parameter_handle<int> h_p (&a, &parameter);

  ioa::action<automaton::p_t_output_action> action (h_p, z->p_t_output);

  BOOST_CHECK_EQUAL (h_a, action.get_automaton_handle ()); 
  BOOST_CHECK_EQUAL (action, action);
  std::pair<bool, int> r = action ();
  BOOST_CHECK (r.first);
  BOOST_CHECK_EQUAL (r.second, 9845);
  BOOST_CHECK (z->p_t_output.state);
  BOOST_CHECK_EQUAL (&parameter, z->p_t_output.last_parameter);
}

BOOST_AUTO_TEST_SUITE_END()
