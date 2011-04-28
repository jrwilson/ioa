#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE action
#include <boost/test/unit_test.hpp>
#include <macro_action.hpp>
#include "automaton.hpp"
#include "scheduler.hpp"
#include "system.hpp"

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

BOOST_AUTO_TEST_CASE(untyped_macro_action)
{
  automaton* automaton_a = new automaton ();
  ioa::typed_automaton<automaton> a (0, ioa::null_sys, automaton_a);
  ioa::generic_automaton_handle h_a (&a);
  ioa::action<automaton::up_ut_output_action> output_action (h_a, automaton_a->up_ut_output);

  bool decomposed;
  callback cb (&decomposed);

  automaton* automaton_b = new automaton ();
  ioa::typed_automaton<automaton> b (0, ioa::null_sys, automaton_b);
  ioa::generic_automaton_handle h_b (&b);
  ioa::action<automaton::up_ut_input_action, callback> input_action (h_b, automaton_b->up_ut_input, h_b, cb);

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
  ioa::typed_automaton<automaton> a (0, ioa::null_sys, automaton_a);
  ioa::generic_automaton_handle h_a (&a);
  ioa::action<automaton::up_t_output_action> output_action (h_a, automaton_a->up_t_output);

  bool decomposed;
  callback cb (&decomposed);

  automaton* automaton_b = new automaton ();
  ioa::typed_automaton<automaton> b (0, ioa::null_sys, automaton_b);
  ioa::generic_automaton_handle h_b (&b);
  ioa::action<automaton::up_t_input_action, callback> input_action (h_b, automaton_b->up_t_input, h_b, cb);

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
