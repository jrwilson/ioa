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

  void decomposed () {
    *ptr = true;
  }
};

BOOST_AUTO_TEST_SUITE(action_suite)

BOOST_AUTO_TEST_CASE(unparameterized_untyped_output_action)
{
  automaton* z = new automaton ();
  ioa::automaton<automaton> a (z);
  ioa::unparameterized_untyped_output_action<automaton, automaton::up_ut_output_action> action (&a, &automaton::up_ut_output);
  BOOST_CHECK_EQUAL (&a, action.get_automaton ());
  BOOST_CHECK_EQUAL (action, action);
  bool r = action ();
  BOOST_CHECK (r);
  BOOST_CHECK (z->up_ut_output.state);
}

BOOST_AUTO_TEST_CASE(parameterized_untyped_output_action)
{
  int parameter;
  automaton* z = new automaton ();
  ioa::automaton<automaton> a (z);
  ioa::parameterized_untyped_output_action<automaton, automaton::p_ut_output_action, int> action (&a, &automaton::p_ut_output, &parameter);
  BOOST_CHECK_EQUAL (&a, action.get_automaton ()); 
  BOOST_CHECK_EQUAL (action, action);
  bool r = action ();
  BOOST_CHECK (r);
  BOOST_CHECK (z->p_ut_output.state);
  BOOST_CHECK_EQUAL (&parameter, z->p_ut_output.last_parameter);
}

BOOST_AUTO_TEST_CASE(unparameterized_typed_output_action)
{
  automaton* z = new automaton ();
  ioa::automaton<automaton> a (z);
  ioa::unparameterized_typed_output_action<automaton, automaton::up_t_output_action> action (&a, &automaton::up_t_output);
  BOOST_CHECK_EQUAL (&a, action.get_automaton ()); 
  BOOST_CHECK_EQUAL (action, action);
  std::pair<bool, int> r = action ();
  BOOST_CHECK (r.first);
  BOOST_CHECK_EQUAL (r.second, 9845);
  BOOST_CHECK (z->up_t_output.state);
}

BOOST_AUTO_TEST_CASE(parameterized_typed_output_action)
{
  int parameter;
  automaton* z = new automaton ();
  ioa::automaton<automaton> a (z);
  ioa::parameterized_typed_output_action<automaton, automaton::p_t_output_action, int> action (&a, &automaton::p_t_output, &parameter);
  BOOST_CHECK_EQUAL (&a, action.get_automaton ()); 
  BOOST_CHECK_EQUAL (action, action);
  std::pair<bool, int> r = action ();
  BOOST_CHECK (r.first);
  BOOST_CHECK_EQUAL (r.second, 9845);
  BOOST_CHECK (z->p_t_output.state);
  BOOST_CHECK_EQUAL (&parameter, z->p_t_output.last_parameter);
}

BOOST_AUTO_TEST_CASE(unparameterized_untyped_input_action)
{
  automaton*z = new automaton();
  ioa::automaton<automaton> a (z);
  bool decomposed;
  callback cb (&decomposed);
  ioa::unparameterized_untyped_input_action<automaton, automaton::up_ut_input_action, callback> action (&a, &automaton::up_ut_input, &a, cb);
  BOOST_CHECK_EQUAL (&a, action.get_automaton ()); 
  BOOST_CHECK_EQUAL (action, action);
  BOOST_CHECK_EQUAL (action.get_owner (), &a);
  action ();
  BOOST_CHECK (z->up_ut_input.state);
  action.decompose ();
  BOOST_CHECK (decomposed);
}

BOOST_AUTO_TEST_CASE(parameterized_untyped_input_action)
{
  int parameter;
  automaton*z = new automaton();
  ioa::automaton<automaton> a (z);
  bool decomposed;
  callback cb (&decomposed);
  ioa::parameterized_untyped_input_action<automaton, automaton::p_ut_input_action, callback, int> action (&a, &automaton::p_ut_input, &a, cb, &parameter);
  BOOST_CHECK_EQUAL (&a, action.get_automaton ()); 
  BOOST_CHECK_EQUAL (action, action);
  BOOST_CHECK_EQUAL (action.get_owner (), &a);
  action ();
  BOOST_CHECK (z->p_ut_input.state);
  BOOST_CHECK_EQUAL (&parameter, z->p_ut_input.last_parameter);
  action.decompose ();
  BOOST_CHECK (decomposed);
}

BOOST_AUTO_TEST_CASE(unparameterized_typed_input_action)
{
  automaton*z = new automaton();
  ioa::automaton<automaton> a (z);
  bool decomposed;
  callback cb (&decomposed);
  ioa::unparameterized_typed_input_action<automaton, automaton::up_t_input_action, callback> action (&a, &automaton::up_t_input, &a, cb);
  BOOST_CHECK_EQUAL (&a, action.get_automaton ()); 
  BOOST_CHECK_EQUAL (action, action);
  BOOST_CHECK_EQUAL (action.get_owner (), &a);
  action (9845);
  BOOST_CHECK_EQUAL (9845, z->up_t_input.value);
  action.decompose ();
  BOOST_CHECK (decomposed);
}

BOOST_AUTO_TEST_CASE(parameterized_typed_input_action)
{
  int parameter;
  automaton*z = new automaton();
  ioa::automaton<automaton> a (z);
  bool decomposed;
  callback cb (&decomposed);
  ioa::parameterized_typed_input_action<automaton, automaton::p_t_input_action, callback, int> action (&a, &automaton::p_t_input, &a, cb, &parameter);
  BOOST_CHECK_EQUAL (&a, action.get_automaton ()); 
  BOOST_CHECK_EQUAL (action, action);
  BOOST_CHECK_EQUAL (action.get_owner (), &a);
  action (9845);
  BOOST_CHECK_EQUAL (9845, z->p_t_input.value);
  BOOST_CHECK_EQUAL (&parameter, z->p_t_input.last_parameter);
  action.decompose ();
  BOOST_CHECK (decomposed);
}

BOOST_AUTO_TEST_CASE(untyped_macro_action)
{
  automaton* automaton_a = new automaton ();
  ioa::automaton<automaton> a (automaton_a);
  ioa::unparameterized_untyped_output_action<automaton, automaton::up_ut_output_action> output_action (&a, &automaton::up_ut_output);

  bool decomposed;
  callback cb (&decomposed);

  automaton* automaton_b = new automaton ();
  ioa::automaton<automaton> b (automaton_b);
  ioa::unparameterized_untyped_input_action<automaton, automaton::up_ut_input_action, callback> input_action (&b, &automaton::up_ut_input, &b, cb);

  null_scheduler scheduler;
  ioa::untyped_macro_action<null_scheduler> macro_action (output_action, scheduler);
  macro_action.add_input (input_action);

  BOOST_CHECK (macro_action.involves_output (output_action));
  BOOST_CHECK (macro_action.involves_input_check_owner (input_action));

  BOOST_CHECK (!automaton_b->up_ut_input.state);
  macro_action.execute ();
  BOOST_CHECK (automaton_b->up_ut_input.state);
}

BOOST_AUTO_TEST_CASE(typed_macro_action)
{
  automaton* automaton_a = new automaton ();
  ioa::automaton<automaton> a (automaton_a);
  ioa::unparameterized_typed_output_action<automaton, automaton::up_t_output_action> output_action (&a, &automaton::up_t_output);

  bool decomposed;
  callback cb (&decomposed);

  automaton* automaton_b = new automaton ();
  ioa::automaton<automaton> b (automaton_b);
  ioa::unparameterized_typed_input_action<automaton, automaton::up_t_input_action, callback> input_action (&b, &automaton::up_t_input, &b, cb);

  null_scheduler scheduler;
  ioa::typed_macro_action<int, null_scheduler> macro_action (output_action, scheduler);
  macro_action.add_input (input_action);

  BOOST_CHECK (macro_action.involves_output (output_action));
  BOOST_CHECK (macro_action.involves_input_check_owner (input_action));

  macro_action.execute ();
  BOOST_CHECK_EQUAL (automaton_b->up_t_input.value, 9845);
}

BOOST_AUTO_TEST_SUITE_END()
