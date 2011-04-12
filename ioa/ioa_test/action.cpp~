#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE Action
#include <boost/test/unit_test.hpp>
#include "../src/action.hpp"
#include "automaton.hpp"

struct callback {
  void decompose() { }
};

BOOST_AUTO_TEST_SUITE(ActionSuite)

BOOST_AUTO_TEST_CASE(OutputAction)
{
  automaton*z = new automaton();
  ioa::automaton<automaton> a(z);
  ioa::output_action<automaton, automaton::output_action, int> action(&a, &automaton::output);
  BOOST_CHECK (action.is_action(&a, &z->output));
  BOOST_CHECK_EQUAL (action(), 9845);
}

BOOST_AUTO_TEST_CASE(InputAction)
{
  automaton*z = new automaton();
  ioa::automaton<automaton> a(z);
  callback cb;
  ioa::input_action<automaton, automaton::input_action, int, callback> action(&a, &automaton::input, &a, cb);
  BOOST_CHECK (action.is_action(&a, &z->input));
  action(9845);
  BOOST_CHECK_EQUAL (z->value, 9845);
}

BOOST_AUTO_TEST_CASE(MacroAction)
{
  automaton* z = new automaton();
  callback cb;
  ioa::automaton<automaton> a(z);
  ioa::output_action<automaton, automaton::output_action, int>* output_action = new ioa::output_action<automaton, automaton::output_action, int>(&a, &automaton::output);
  ioa::input_action<automaton, automaton::input_action, int, callback>* input_action = new ioa::input_action<automaton, automaton::input_action, int,callback>(&a, &automaton::input, &a, cb);

  ioa::macro_action<int> macro_action(output_action);
  macro_action.add_input(input_action);

  BOOST_CHECK (macro_action.is_output(&a, &z->output));
  BOOST_CHECK (macro_action.is_input(&a, &z->input));

  macro_action();
  BOOST_CHECK_EQUAL (z->value, 9845);
}


BOOST_AUTO_TEST_SUITE_END()
