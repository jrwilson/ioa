#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE action
#include <boost/test/unit_test.hpp>
#include <macro_action.hpp>
#include "automaton.hpp"
#include "scheduler.hpp"

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

BOOST_AUTO_TEST_CASE(unparameterized_untyped_output_action)
{
  automaton* z = new automaton ();
  ioa::typed_automaton<automaton> a (z);
  ioa::action<automaton::up_ut_output_action> action (&a, z->up_ut_output);

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
  ioa::typed_automaton<automaton> a (z);
  ioa::action<automaton::p_ut_output_action> action (&a, &parameter, z->p_ut_output);
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
  ioa::typed_automaton<automaton> a (z);
  ioa::action<automaton::up_t_output_action> action (&a, z->up_t_output);
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
  ioa::typed_automaton<automaton> a (z);
  ioa::action<automaton::p_t_output_action> action (&a, &parameter, z->p_t_output);
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
  ioa::typed_automaton<automaton> a (z);
  bool decomposed;
  callback cb (&decomposed);
  ioa::action<automaton::up_ut_input_action, callback> action (&a, z->up_ut_input, &a, cb);
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
  ioa::typed_automaton<automaton> a (z);
  bool decomposed;
  callback cb (&decomposed);
  ioa::action<automaton::p_ut_input_action, callback> action (&a, &parameter, z->p_ut_input, &a, cb);
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
  ioa::typed_automaton<automaton> a (z);
  bool decomposed;
  callback cb (&decomposed);
  ioa::action<automaton::up_t_input_action, callback> action (&a, z->up_t_input, &a, cb);
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
  ioa::typed_automaton<automaton> a (z);
  bool decomposed;
  callback cb (&decomposed);
  ioa::action<automaton::p_t_input_action, callback> action (&a, &parameter, z->p_t_input, &a, cb);
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
  ioa::typed_automaton<automaton> a (automaton_a);
  ioa::action<automaton::up_ut_output_action> output_action (&a, automaton_a->up_ut_output);

  bool decomposed;
  callback cb (&decomposed);

  automaton* automaton_b = new automaton ();
  ioa::typed_automaton<automaton> b (automaton_b);
  ioa::action<automaton::up_ut_input_action, callback> input_action (&b, automaton_b->up_ut_input, &b, cb);

  ioa::untyped_macro_action macro_action (output_action);
  macro_action.add_input (input_action);

  BOOST_CHECK (macro_action.involves_output (output_action));
  BOOST_CHECK (macro_action.involves_input_check_owner (input_action));

  BOOST_CHECK (!automaton_b->up_ut_input.state);
  macro_action.execute ();
  BOOST_CHECK (automaton_b->up_ut_input.state);

  BOOST_CHECK (!decomposed);
  BOOST_CHECK (!macro_action.empty ());
  macro_action.decompose (input_action);
  BOOST_CHECK (decomposed);
  BOOST_CHECK (macro_action.empty ());
}

BOOST_AUTO_TEST_CASE(typed_macro_action)
{
  automaton* automaton_a = new automaton ();
  ioa::typed_automaton<automaton> a (automaton_a);
  ioa::action<automaton::up_t_output_action> output_action (&a, automaton_a->up_t_output);

  bool decomposed;
  callback cb (&decomposed);

  automaton* automaton_b = new automaton ();
  ioa::typed_automaton<automaton> b (automaton_b);
  ioa::action<automaton::up_t_input_action, callback> input_action (&b, automaton_b->up_t_input, &b, cb);

  ioa::typed_macro_action<int> macro_action (output_action);
  macro_action.add_input (input_action);

  BOOST_CHECK (macro_action.involves_output (output_action));
  BOOST_CHECK (macro_action.involves_input_check_owner (input_action));

  macro_action.execute ();
  BOOST_CHECK_EQUAL (automaton_b->up_t_input.value, 9845);

  BOOST_CHECK (!decomposed);
  BOOST_CHECK (!macro_action.empty ());
  macro_action.decompose (input_action);
  BOOST_CHECK (decomposed);
  BOOST_CHECK (macro_action.empty ());
}

BOOST_AUTO_TEST_SUITE_END()
