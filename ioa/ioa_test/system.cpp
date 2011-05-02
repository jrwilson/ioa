#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE system
#include <boost/test/unit_test.hpp>

#include <system.hpp>
#include "automaton.hpp"

BOOST_AUTO_TEST_SUITE(system_suite)

BOOST_AUTO_TEST_CASE (system_create_creator_dne)
{
  // ioa::system system;
  // automaton* a = new automaton ();
  // ioa::timestamp<ioa::typed_automaton<blah> > a (&y);
  // x = new blah ();
  // ioa::system::create_result<blah> r1 = system.create (a, x);
  // BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_CREATOR_DNE);
  // delete a;
  // 1. Create an automaton.
  // 2. Destroy the automaton.
  // 3. Create another automaton using handle of first automaton.
  BOOST_CHECK (false);
}

BOOST_AUTO_TEST_CASE (system_create_exists)
{
  ioa::system system;
  automaton* a = new automaton ();
  ioa::system::create_result<automaton> r1 = system.create (a);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  r1 = system.create (a);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_EXISTS);
}

BOOST_AUTO_TEST_CASE (system_create_success)
{
  ioa::system system;
  automaton* a = new automaton ();
  ioa::system::create_result<automaton> r1 = system.create (a);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
}

BOOST_AUTO_TEST_CASE (system_declare_automaton_dne)
{
//   ioa::system system;
//   int parameter;
  
//   blah* x = new blah ();
//   ioa::typed_automaton<blah> y (x);
//   ioa::timestamp<ioa::typed_automaton<blah> > a (&y);
//   ioa::system::declare_result d1 = system.declare (a, &parameter);
//   BOOST_CHECK_EQUAL (d1.type, ioa::system::DECLARE_AUTOMATON_DNE);
  BOOST_CHECK (false);
}

BOOST_AUTO_TEST_CASE (system_declare_exists)
{
  ioa::system system;
  int parameter;
  
  ioa::system::create_result<automaton> r1 = system.create (new automaton ());
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::system::declare_result<int> d1 = system.declare (r1.automaton, &parameter);
  BOOST_CHECK_EQUAL (d1.type, ioa::system::DECLARE_SUCCESS);
  d1 = system.declare (r1.automaton, &parameter);
  BOOST_CHECK_EQUAL (d1.type, ioa::system::DECLARE_EXISTS);
}

BOOST_AUTO_TEST_CASE (system_declare_success)
{
  ioa::system system;
  int parameter;
  
  ioa::system::create_result<automaton> r1 = system.create (new automaton ());
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::system::declare_result<int> d1 = system.declare (r1.automaton, &parameter);
  BOOST_CHECK_EQUAL (d1.type, ioa::system::DECLARE_SUCCESS);
}

BOOST_AUTO_TEST_CASE (system_compose_compose_automaton_dne)
{
//   ioa::system system;

//   blah* x = new blah ();
//   ioa::typed_automaton<blah> y (x);
//   ioa::timestamp<ioa::typed_automaton<blah> > a (&y);

//   x = new blah ();
//   ioa::system::create_result<blah> r1 = system.create (x);
//   BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
//   ioa::timestamp<ioa::typed_automaton<blah> > output = r1.automaton;

//   r1 = system.create (new blah ());
//   BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
//   ioa::timestamp<ioa::typed_automaton<blah> > input = r1.automaton;

//   ioa::system::compose_result c1 = system.compose (output, &blah::output, input, &blah::input, a);
//   BOOST_CHECK_EQUAL (c1.type, ioa::system::COMPOSE_COMPOSER_AUTOMATON_DNE);
  BOOST_CHECK (false);
}

BOOST_AUTO_TEST_CASE (system_compose_output_automaton_dne)
{
//   ioa::system system;

//   blah* x = new blah ();
//   ioa::typed_automaton<blah> y (x);
//   ioa::timestamp<ioa::typed_automaton<blah> > a (&y);

//   x = new blah ();
//   ioa::system::create_result<blah> r1 = system.create (x);
//   BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
//   ioa::timestamp<ioa::automaton> composer = r1.automaton;

//   ioa::system::compose_result c1 = system.compose (a, &blah::output, a, &blah::input, composer);
//   BOOST_CHECK_EQUAL (c1.type, ioa::system::COMPOSE_OUTPUT_AUTOMATON_DNE);
  BOOST_CHECK (false);
}

BOOST_AUTO_TEST_CASE (system_compose_input_automaton_dne)
{
//   ioa::system system;

//   blah* x = new blah ();
//   ioa::typed_automaton<blah> y (x);
//   ioa::timestamp<ioa::typed_automaton<blah> > a (&y);

//   x = new blah ();
//   ioa::system::create_result<blah> r1 = system.create (x);
//   BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
//   ioa::timestamp<ioa::automaton> composer = r1.automaton;

//   r1 = system.create (new blah ());
//   BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
//   ioa::timestamp<ioa::typed_automaton<blah> > output = r1.automaton;

//   ioa::system::compose_result c1 = system.compose (output, &blah::output, a, &blah::input, composer);
//   BOOST_CHECK_EQUAL (c1.type, ioa::system::COMPOSE_INPUT_AUTOMATON_DNE);
  BOOST_CHECK (false);
}

BOOST_AUTO_TEST_CASE (system_compose_output_parameter_dne)
{
  ioa::system system;
  int parameter;
  automaton* composer_instance = new automaton ();
  automaton* output_instance = new automaton ();
  automaton* input_instance = new automaton ();

  ioa::system::create_result<automaton> r1 = system.create (composer_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton> composer = r1.automaton;
  
  r1 = system.create (output_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton> output = r1.automaton;

  r1 = system.create (input_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton> input = r1.automaton;

  // ioa::timestamp<void> param (&parameter);
  // ioa::system::compose_result c1 = system.compose (output, &automaton::output, param, input, &automaton::input, composer);
  // BOOST_CHECK_EQUAL (c1.type, ioa::system::COMPOSE_OUTPUT_PARAMETER_DNE);
  BOOST_CHECK (false);
}

BOOST_AUTO_TEST_CASE (system_compose_input_parameter_dne)
{
  ioa::system system;
  int parameter;
  automaton* composer_instance = new automaton ();
  automaton* output_instance = new automaton ();
  automaton* input_instance = new automaton ();

  ioa::system::create_result<automaton> r1 = system.create (composer_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton> composer = r1.automaton;
  
  r1 = system.create (output_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton> output = r1.automaton;

  r1 = system.create (input_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton> input = r1.automaton;

  // ioa::timestamp<void> param (&parameter);
  // ioa::system::compose_result c1 = system.compose (output, &automaton::output, input, &automaton::input, param, composer);
  // BOOST_CHECK_EQUAL (c1.type, ioa::system::COMPOSE_INPUT_PARAMETER_DNE);
  BOOST_CHECK (false);
}

BOOST_AUTO_TEST_CASE (system_compose_exists)
{
  ioa::system system;
  automaton* composer_instance = new automaton ();
  automaton* output_instance = new automaton ();
  automaton* input_instance = new automaton ();
  
  ioa::system::create_result<automaton> r1 = system.create (composer_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton> composer = r1.automaton;
  
  r1 = system.create (output_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton> output = r1.automaton;
  
  r1 = system.create (input_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton> input = r1.automaton;
  
  ioa::system::compose_result c1 = system.compose (output, &automaton::up_uv_output, input, &automaton::up_uv_input, composer);
  BOOST_CHECK_EQUAL (c1.type, ioa::system::COMPOSE_SUCCESS);

  c1 = system.compose (output, &automaton::up_uv_output, input, &automaton::up_uv_input, composer);
  BOOST_CHECK_EQUAL (c1.type, ioa::system::COMPOSE_EXISTS);
}

BOOST_AUTO_TEST_CASE (system_compose_input_action_unavailable)
{
  ioa::system system;
  automaton* composer_instance = new automaton ();
  automaton* output1_instance = new automaton ();
  automaton* output2_instance = new automaton ();
  automaton* input_instance = new automaton ();
  
  ioa::system::create_result<automaton> r1 = system.create (composer_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton> composer = r1.automaton;
    
  r1 = system.create (output1_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton> output1 = r1.automaton;
  
  r1 = system.create (output2_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton> output2 = r1.automaton;
  
  r1 = system.create (input_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton> input = r1.automaton;
  
  ioa::system::compose_result c1 = system.compose (output1, &automaton::up_uv_output, input, &automaton::up_uv_input, composer);
  BOOST_CHECK_EQUAL (c1.type, ioa::system::COMPOSE_SUCCESS);

  c1 = system.compose (output1, &automaton::up_uv_output, input, &automaton::up_uv_input, composer);
  BOOST_CHECK_EQUAL (c1.type, ioa::system::COMPOSE_EXISTS);

  c1 = system.compose (output2, &automaton::up_uv_output, input, &automaton::up_uv_input, composer);
  BOOST_CHECK_EQUAL (c1.type, ioa::system::COMPOSE_INPUT_ACTION_UNAVAILABLE);
}

BOOST_AUTO_TEST_CASE (system_compose_output_action_unavailable)
{
  ioa::system system;
  int parameter;
  automaton* composer_instance = new automaton ();
  automaton* output_instance = new automaton ();
  automaton* input_instance = new automaton ();

  ioa::system::create_result<automaton> r1 = system.create (composer_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton> composer = r1.automaton;
  
  r1 = system.create (output_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton> output = r1.automaton;

  r1 = system.create (input_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton> input = r1.automaton;

  ioa::system::declare_result<int> d1 = system.declare (input, &parameter);
  BOOST_CHECK_EQUAL (d1.type, ioa::system::DECLARE_SUCCESS);
  ioa::parameter_handle<int> param = d1.parameter;

  ioa::system::compose_result c1 = system.compose (output, &automaton::up_uv_output, input, &automaton::up_uv_input, composer);
  BOOST_CHECK_EQUAL (c1.type, ioa::system::COMPOSE_SUCCESS);

  c1 = system.compose (output, &automaton::up_uv_output, input, &automaton::p_uv_input, param, composer);
  BOOST_CHECK_EQUAL (c1.type, ioa::system::COMPOSE_OUTPUT_ACTION_UNAVAILABLE);
}

BOOST_AUTO_TEST_CASE (system_compose_success)
{
  ioa::system system;
  automaton* composer_instance = new automaton ();
  automaton* output_instance = new automaton ();
  automaton* input_instance = new automaton ();

  ioa::system::create_result<automaton> r1 = system.create (composer_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton> composer = r1.automaton;
  
  r1 = system.create (output_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton> output = r1.automaton;

  r1 = system.create (input_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton> input = r1.automaton;

  ioa::system::compose_result c1 = system.compose (output, &automaton::up_uv_output, input, &automaton::up_uv_input, composer);
  BOOST_CHECK_EQUAL (c1.type, ioa::system::COMPOSE_SUCCESS);

  system.execute_output (output, &automaton::up_uv_output);
  BOOST_CHECK (input_instance->up_uv_input.state);
}

BOOST_AUTO_TEST_SUITE_END()
