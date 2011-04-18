#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE composition_manager
#include <boost/test/unit_test.hpp>
#include <composition_manager.hpp>
#include "automaton.hpp"
#include "scheduler.hpp"

struct callback {
  void operator() () { }
};

BOOST_AUTO_TEST_SUITE(composition_manager_suite)

BOOST_AUTO_TEST_CASE(untyped_compose)
{
  ioa::composition_manager cm;
  ioa::typed_automaton<automaton> parent (new automaton());
  automaton* y = new automaton ();
  ioa::typed_automaton<automaton> child1 (y);
  ioa::generic_automaton_handle child1_handle (&child1);
  ioa::action<automaton::up_ut_output_action> output_action (child1_handle, y->up_ut_output);

  automaton* z = new automaton ();
  ioa::typed_automaton<automaton> child2 (z);
  ioa::generic_automaton_handle child2_handle (&child2);
  callback cb;
  ioa::action<automaton::up_ut_input_action, callback> input_action (child2_handle, z->up_ut_input, &parent, cb);

  BOOST_CHECK (!(cm.composed_check_owner (output_action, input_action)));
  BOOST_CHECK (cm.input_available (input_action));
  BOOST_CHECK (cm.output_available (output_action, &child2));
  BOOST_CHECK (!cm.output_available (output_action, &child1));
  
  cm.compose (output_action, input_action);

  BOOST_CHECK ((cm.composed_check_owner (output_action, input_action)));
  BOOST_CHECK (!cm.input_available (input_action));
  BOOST_CHECK (!cm.output_available (output_action, &child2));

  cm.execute (output_action);
  BOOST_CHECK (z->up_ut_input.state);

  ioa::action<automaton::up_ut_output_action> output_action2 (child2_handle, z->up_ut_output);
  cm.execute (output_action2);
}

BOOST_AUTO_TEST_CASE(typed_compose)
{
  ioa::composition_manager cm;
  ioa::typed_automaton<automaton> parent (new automaton());
  automaton* y = new automaton ();
  ioa::typed_automaton<automaton> child1 (y);
  ioa::generic_automaton_handle child1_handle (&child1);
  ioa::action<automaton::up_t_output_action> output_action (child1_handle, y->up_t_output);

  automaton* z = new automaton ();
  ioa::typed_automaton<automaton> child2 (z);
  ioa::generic_automaton_handle child2_handle (&child2);
  callback cb;
  ioa::action<automaton::up_t_input_action, callback> input_action (child2_handle, z->up_t_input, &parent, cb);

  BOOST_CHECK (!(cm.composed_check_owner (output_action, input_action)));
  BOOST_CHECK (cm.input_available (input_action));
  BOOST_CHECK (cm.output_available (output_action, &child2));
  BOOST_CHECK (!cm.output_available (output_action, &child1));
  
  cm.compose (output_action, input_action);

  BOOST_CHECK ((cm.composed_check_owner (output_action, input_action)));
  BOOST_CHECK (!cm.input_available (input_action));
  BOOST_CHECK (!cm.output_available (output_action, &child2));

  cm.execute (output_action);
  BOOST_CHECK_EQUAL (z->up_t_input.value, 9845);

  ioa::action<automaton::up_t_output_action> output_action2 (child2_handle, z->up_t_output);
  cm.execute (output_action2);
}

BOOST_AUTO_TEST_CASE(untyped_decompose)
{
  ioa::composition_manager cm;
  ioa::typed_automaton<automaton> parent (new automaton());
  automaton* y = new automaton ();
  ioa::typed_automaton<automaton> child1 (y);
  ioa::generic_automaton_handle child1_handle (&child1);
  ioa::action<automaton::up_ut_output_action> output_action (child1_handle, y->up_ut_output);

  automaton* z = new automaton ();
  ioa::typed_automaton<automaton> child2 (z);
  ioa::generic_automaton_handle child2_handle (&child2);
  callback cb;
  ioa::action<automaton::up_ut_input_action, callback> input_action (child2_handle, z->up_ut_input, &parent, cb);

  BOOST_CHECK (!(cm.composed_check_owner (output_action, input_action)));
  BOOST_CHECK (cm.input_available (input_action));
  BOOST_CHECK (cm.output_available (output_action, &child2));
  BOOST_CHECK (!cm.output_available (output_action, &child1));
  
  cm.compose (output_action, input_action);

  BOOST_CHECK ((cm.composed_check_owner (output_action, input_action)));
  BOOST_CHECK (!cm.input_available (input_action));
  BOOST_CHECK (!cm.output_available (output_action, &child2));

  cm.decompose (output_action, input_action);

  BOOST_CHECK (!(cm.composed_check_owner (output_action, input_action)));
  BOOST_CHECK (cm.input_available (input_action));
  BOOST_CHECK (cm.output_available (output_action, &child2));
}

// BOOST_AUTO_TEST_CASE(Decompose)
// {
//   callback cb;
//   automaton* z = new automaton();
//   ioa::composition_manager cm;
//   ioa::automaton<automaton> parent(new automaton());
//   ioa::automaton<automaton> child1(new automaton());
//   ioa::automaton<automaton> child2(z);

//   cm.compose<callback, automaton, automaton::output_action, automaton, automaton::input_action, int>
//     (cb, &parent, &child1, &automaton::output, &child2, &automaton::input);

//   cm.decompose<automaton, automaton::output_action, automaton, automaton::input_action, int>
//     (&parent, &child1, &automaton::output, &child2, &automaton::input);
// }

BOOST_AUTO_TEST_SUITE_END()
