#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE CompositionManager
#include <boost/test/unit_test.hpp>
#include "../src/composition_manager.hpp"
#include "automaton.hpp"

struct callback {
  void decompose() { }
};

BOOST_AUTO_TEST_SUITE(CompositionManagerSuite)

BOOST_AUTO_TEST_CASE(Compose)
{
  automaton* z = new automaton();
  ioa::composition_manager cm;
  ioa::automaton<automaton> parent(new automaton());
  ioa::automaton<automaton> child1(new automaton());
  ioa::automaton<automaton> child2(z);

  BOOST_CHECK (!(cm.composed<automaton, automaton::output_action, automaton, automaton::input_action, int>(&parent,
													  &child1, &automaton::output,
													   &child2, &automaton::input)));
  BOOST_CHECK (cm.input_available (&child2, &automaton::input));
  BOOST_CHECK (cm.output_available (&child1, &automaton::output, &child2));
  BOOST_CHECK (!cm.output_available (&child1, &automaton::output, &child1));
  
  callback cb;
  cm.compose<callback, automaton, automaton::output_action, automaton, automaton::input_action, int>
    (cb, &parent, &child1, &automaton::output, &child2, &automaton::input);

  BOOST_CHECK ((cm.composed<automaton, automaton::output_action, automaton, automaton::input_action, int>(&parent,
													  &child1, &automaton::output,
													   &child2, &automaton::input)));

  BOOST_CHECK (!cm.input_available (&child2, &automaton::input));
  BOOST_CHECK (!cm.output_available (&child1, &automaton::output, &child2));

  cm.execute(&child1, &automaton::output);
  BOOST_CHECK_EQUAL (z->value, 9845);

  cm.execute(&child2, &automaton::output);
}

BOOST_AUTO_TEST_CASE(Decompose)
{
  callback cb;
  automaton* z = new automaton();
  ioa::composition_manager cm;
  ioa::automaton<automaton> parent(new automaton());
  ioa::automaton<automaton> child1(new automaton());
  ioa::automaton<automaton> child2(z);

  cm.compose<callback, automaton, automaton::output_action, automaton, automaton::input_action, int>
    (cb, &parent, &child1, &automaton::output, &child2, &automaton::input);

  cm.decompose<automaton, automaton::output_action, automaton, automaton::input_action, int>
    (&parent, &child1, &automaton::output, &child2, &automaton::input);
}


BOOST_AUTO_TEST_SUITE_END()
