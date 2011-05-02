#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE composition
#include <boost/test/unit_test.hpp>

#include <composition.hpp>
#include "automaton.hpp"

BOOST_AUTO_TEST_SUITE(composition_suite)

BOOST_AUTO_TEST_CASE(compose_unparameterized_unvalued_output_action)
{
  ioa::locker<ioa::automaton_interface*> automata;
  automaton* composer_instance = new automaton ();
  automaton* output_instance = new automaton ();
  automaton* input1_instance = new automaton ();
  automaton* input2_instance = new automaton ();
  ioa::automaton<automaton> composer_automaton (composer_instance);
  ioa::automaton<automaton> output_automaton (output_instance);
  ioa::automaton<automaton> input1_automaton (input1_instance);
  ioa::automaton<automaton> input2_automaton (input2_instance);
  ioa::automaton_handle<automaton> composer_handle = automata.insert (&composer_automaton);
  ioa::automaton_handle<automaton> output_handle = automata.insert (&output_automaton);
  ioa::automaton_handle<automaton> input1_handle = automata.insert (&input1_automaton);
  ioa::automaton_handle<automaton> input2_handle = automata.insert (&input2_automaton);

  int input_parameter;
  ioa::parameter_handle<int> input_parameter_handle = input2_automaton.declare_parameter (&input_parameter);

  ioa::action<automaton::up_uv_output_action> output (output_handle, output_instance->up_uv_output);
  ioa::action<automaton::up_uv_input_action> input1 (input1_handle, input1_instance->up_uv_input, composer_handle);
  ioa::action<automaton::p_uv_input_action> input2 (input2_handle, input2_instance->p_uv_input, input_parameter_handle, composer_handle);

  ioa::composition<automaton::up_uv_output_action> composition (output);
  composition.compose (input1);
  composition.compose (input2);

  BOOST_CHECK (!composition.empty ());
  BOOST_CHECK (composition.involves_output (output));
  BOOST_CHECK (composition.involves_input (input1, true));
  ioa::action<automaton::p_uv_input_action> input_test (input2_handle, input2_instance->p_uv_input, input_parameter_handle, input2_handle);
  BOOST_CHECK (composition.involves_input (input_test, false));
  BOOST_CHECK (composition.involves_input_automaton (input1_handle));
  BOOST_CHECK (composition.involves_input_automaton (input2_handle));

  composition.execute ();

  BOOST_CHECK (output_instance->up_uv_output.state);
  BOOST_CHECK (input1_instance->up_uv_input.state);
  BOOST_CHECK (input2_instance->p_uv_input.state);
  BOOST_CHECK_EQUAL (input2_instance->p_uv_input.last_parameter, &input_parameter);
}

BOOST_AUTO_TEST_CASE(compose_parameterized_unvalued_output_action)
{
  ioa::locker<ioa::automaton_interface*> automata;
  automaton* composer_instance = new automaton ();
  automaton* output_instance = new automaton ();
  automaton* input1_instance = new automaton ();
  automaton* input2_instance = new automaton ();
  ioa::automaton<automaton> composer_automaton (composer_instance);
  ioa::automaton<automaton> output_automaton (output_instance);
  ioa::automaton<automaton> input1_automaton (input1_instance);
  ioa::automaton<automaton> input2_automaton (input2_instance);
  ioa::automaton_handle<automaton> composer_handle = automata.insert (&composer_automaton);
  ioa::automaton_handle<automaton> output_handle = automata.insert (&output_automaton);
  ioa::automaton_handle<automaton> input1_handle = automata.insert (&input1_automaton);
  ioa::automaton_handle<automaton> input2_handle = automata.insert (&input2_automaton);

  int output_parameter;
  ioa::parameter_handle<int> output_parameter_handle = output_automaton.declare_parameter (&output_parameter);
  int input_parameter;
  ioa::parameter_handle<int> input_parameter_handle = input2_automaton.declare_parameter (&input_parameter);

  ioa::action<automaton::p_uv_output_action> output (output_handle, output_instance->p_uv_output, output_parameter_handle);
  ioa::action<automaton::up_uv_input_action> input1 (input1_handle, input1_instance->up_uv_input, composer_handle);
  ioa::action<automaton::p_uv_input_action> input2 (input2_handle, input2_instance->p_uv_input, input_parameter_handle, composer_handle);

  ioa::composition<automaton::p_uv_output_action> composition (output);
  composition.compose (input1);
  composition.compose (input2);

  BOOST_CHECK (!composition.empty ());
  BOOST_CHECK (composition.involves_output (output));
  BOOST_CHECK (composition.involves_input (input1, true));
  ioa::action<automaton::p_uv_input_action> input_test (input2_handle, input2_instance->p_uv_input, input_parameter_handle, input2_handle);
  BOOST_CHECK (composition.involves_input (input_test, false));
  BOOST_CHECK (composition.involves_input_automaton (input1_handle));
  BOOST_CHECK (composition.involves_input_automaton (input2_handle));

  composition.execute ();

  BOOST_CHECK (output_instance->p_uv_output.state);
  BOOST_CHECK_EQUAL (output_instance->p_uv_output.last_parameter, &output_parameter);
  BOOST_CHECK (input1_instance->up_uv_input.state);
  BOOST_CHECK (input2_instance->p_uv_input.state);
  BOOST_CHECK_EQUAL (input2_instance->p_uv_input.last_parameter, &input_parameter);
}

BOOST_AUTO_TEST_CASE(compose_unparameterized_valued_output_action)
{
  ioa::locker<ioa::automaton_interface*> automata;
  automaton* composer_instance = new automaton ();
  automaton* output_instance = new automaton ();
  automaton* input1_instance = new automaton ();
  automaton* input2_instance = new automaton ();
  ioa::automaton<automaton> composer_automaton (composer_instance);
  ioa::automaton<automaton> output_automaton (output_instance);
  ioa::automaton<automaton> input1_automaton (input1_instance);
  ioa::automaton<automaton> input2_automaton (input2_instance);
  ioa::automaton_handle<automaton> composer_handle = automata.insert (&composer_automaton);
  ioa::automaton_handle<automaton> output_handle = automata.insert (&output_automaton);
  ioa::automaton_handle<automaton> input1_handle = automata.insert (&input1_automaton);
  ioa::automaton_handle<automaton> input2_handle = automata.insert (&input2_automaton);

  int input_parameter;
  ioa::parameter_handle<int> input_parameter_handle = input2_automaton.declare_parameter (&input_parameter);

  ioa::action<automaton::up_v_output_action> output (output_handle, output_instance->up_v_output);
  ioa::action<automaton::up_v_input_action> input1 (input1_handle, input1_instance->up_v_input, composer_handle);
  ioa::action<automaton::p_v_input_action> input2 (input2_handle, input2_instance->p_v_input, input_parameter_handle, composer_handle);

  ioa::composition<automaton::up_v_output_action> composition (output);
  composition.compose (input1);
  composition.compose (input2);

  BOOST_CHECK (!composition.empty ());
  BOOST_CHECK (composition.involves_output (output));
  BOOST_CHECK (composition.involves_input (input1, true));
  ioa::action<automaton::p_v_input_action> input_test (input2_handle, input2_instance->p_v_input, input_parameter_handle, input2_handle);
  BOOST_CHECK (composition.involves_input (input_test, false));
  BOOST_CHECK (composition.involves_input_automaton (input1_handle));
  BOOST_CHECK (composition.involves_input_automaton (input2_handle));

  composition.execute ();

  BOOST_CHECK (output_instance->up_v_output.state);
  BOOST_CHECK_EQUAL (input1_instance->up_v_input.value, 9845);
  BOOST_CHECK_EQUAL (input2_instance->p_v_input.value, 9845);
  BOOST_CHECK_EQUAL (input2_instance->p_v_input.last_parameter, &input_parameter);
}

BOOST_AUTO_TEST_CASE(compose_parameterized_valued_output_action)
{
  ioa::locker<ioa::automaton_interface*> automata;
  automaton* composer_instance = new automaton ();
  automaton* output_instance = new automaton ();
  automaton* input1_instance = new automaton ();
  automaton* input2_instance = new automaton ();
  ioa::automaton<automaton> composer_automaton (composer_instance);
  ioa::automaton<automaton> output_automaton (output_instance);
  ioa::automaton<automaton> input1_automaton (input1_instance);
  ioa::automaton<automaton> input2_automaton (input2_instance);
  ioa::automaton_handle<automaton> composer_handle = automata.insert (&composer_automaton);
  ioa::automaton_handle<automaton> output_handle = automata.insert (&output_automaton);
  ioa::automaton_handle<automaton> input1_handle = automata.insert (&input1_automaton);
  ioa::automaton_handle<automaton> input2_handle = automata.insert (&input2_automaton);

  int output_parameter;
  ioa::parameter_handle<int> output_parameter_handle = output_automaton.declare_parameter (&output_parameter);
  int input_parameter;
  ioa::parameter_handle<int> input_parameter_handle = input2_automaton.declare_parameter (&input_parameter);

  ioa::action<automaton::p_v_output_action> output (output_handle, output_instance->p_v_output, output_parameter_handle);
  ioa::action<automaton::up_v_input_action> input1 (input1_handle, input1_instance->up_v_input, composer_handle);
  ioa::action<automaton::p_v_input_action> input2 (input2_handle, input2_instance->p_v_input, input_parameter_handle, composer_handle);

  ioa::composition<automaton::p_v_output_action> composition (output);
  composition.compose (input1);
  composition.compose (input2);

  BOOST_CHECK (!composition.empty ());
  BOOST_CHECK (composition.involves_output (output));
  BOOST_CHECK (composition.involves_input (input1, true));
  ioa::action<automaton::p_v_input_action> input_test (input2_handle, input2_instance->p_v_input, input_parameter_handle, input2_handle);
  BOOST_CHECK (composition.involves_input (input_test, false));
  BOOST_CHECK (composition.involves_input_automaton (input1_handle));
  BOOST_CHECK (composition.involves_input_automaton (input2_handle));

  composition.execute ();

  BOOST_CHECK (output_instance->p_v_output.state);
  BOOST_CHECK_EQUAL (output_instance->p_v_output.last_parameter, &output_parameter);
  BOOST_CHECK_EQUAL (input1_instance->up_v_input.value, 9845);
  BOOST_CHECK_EQUAL (input2_instance->p_v_input.value, 9845);
  BOOST_CHECK_EQUAL (input2_instance->p_v_input.last_parameter, &input_parameter);
}

BOOST_AUTO_TEST_CASE(decompose_unparameterized_unvalued_output_action)
{
  ioa::locker<ioa::automaton_interface*> automata;
  automaton* composer_instance = new automaton ();
  automaton* output_instance = new automaton ();
  automaton* input1_instance = new automaton ();
  automaton* input2_instance = new automaton ();
  ioa::automaton<automaton> composer_automaton (composer_instance);
  ioa::automaton<automaton> output_automaton (output_instance);
  ioa::automaton<automaton> input1_automaton (input1_instance);
  ioa::automaton<automaton> input2_automaton (input2_instance);
  ioa::automaton_handle<automaton> composer_handle = automata.insert (&composer_automaton);
  ioa::automaton_handle<automaton> output_handle = automata.insert (&output_automaton);
  ioa::automaton_handle<automaton> input1_handle = automata.insert (&input1_automaton);
  ioa::automaton_handle<automaton> input2_handle = automata.insert (&input2_automaton);

  int input_parameter;
  ioa::parameter_handle<int> input_parameter_handle = input2_automaton.declare_parameter (&input_parameter);

  ioa::action<automaton::up_uv_output_action> output (output_handle, output_instance->up_uv_output);
  ioa::action<automaton::up_uv_input_action> input1 (input1_handle, input1_instance->up_uv_input, composer_handle);
  ioa::action<automaton::p_uv_input_action> input2 (input2_handle, input2_instance->p_uv_input, input_parameter_handle, composer_handle);

  ioa::composition<automaton::up_uv_output_action> composition (output);
  composition.compose (input1);
  composition.compose (input2);
  composition.decompose (input1);
  composition.decompose (input2);

  BOOST_CHECK (composition.empty ());
  BOOST_CHECK (composition.involves_output (output));
  BOOST_CHECK (!composition.involves_input (input1, true));
  ioa::action<automaton::p_uv_input_action> input_test (input2_handle, input2_instance->p_uv_input, input_parameter_handle, input2_handle);
  BOOST_CHECK (!composition.involves_input (input_test, false));
  BOOST_CHECK (!composition.involves_input_automaton (input1_handle));
  BOOST_CHECK (!composition.involves_input_automaton (input2_handle));
}

BOOST_AUTO_TEST_CASE(decompose_parameterized_unvalued_output_action)
{
  ioa::locker<ioa::automaton_interface*> automata;
  automaton* composer_instance = new automaton ();
  automaton* output_instance = new automaton ();
  automaton* input1_instance = new automaton ();
  automaton* input2_instance = new automaton ();
  ioa::automaton<automaton> composer_automaton (composer_instance);
  ioa::automaton<automaton> output_automaton (output_instance);
  ioa::automaton<automaton> input1_automaton (input1_instance);
  ioa::automaton<automaton> input2_automaton (input2_instance);
  ioa::automaton_handle<automaton> composer_handle = automata.insert (&composer_automaton);
  ioa::automaton_handle<automaton> output_handle = automata.insert (&output_automaton);
  ioa::automaton_handle<automaton> input1_handle = automata.insert (&input1_automaton);
  ioa::automaton_handle<automaton> input2_handle = automata.insert (&input2_automaton);

  int output_parameter;
  ioa::parameter_handle<int> output_parameter_handle = output_automaton.declare_parameter (&output_parameter);
  int input_parameter;
  ioa::parameter_handle<int> input_parameter_handle = input2_automaton.declare_parameter (&input_parameter);

  ioa::action<automaton::p_uv_output_action> output (output_handle, output_instance->p_uv_output, output_parameter_handle);
  ioa::action<automaton::up_uv_input_action> input1 (input1_handle, input1_instance->up_uv_input, composer_handle);
  ioa::action<automaton::p_uv_input_action> input2 (input2_handle, input2_instance->p_uv_input, input_parameter_handle, composer_handle);

  ioa::composition<automaton::p_uv_output_action> composition (output);
  composition.compose (input1);
  composition.compose (input2);
  composition.decompose (input1);
  composition.decompose (input2);

  BOOST_CHECK (composition.empty ());
  BOOST_CHECK (composition.involves_output (output));
  BOOST_CHECK (!composition.involves_input (input1, true));
  ioa::action<automaton::p_uv_input_action> input_test (input2_handle, input2_instance->p_uv_input, input_parameter_handle, input2_handle);
  BOOST_CHECK (!composition.involves_input (input_test, false));
  BOOST_CHECK (!composition.involves_input_automaton (input1_handle));
  BOOST_CHECK (!composition.involves_input_automaton (input2_handle));
}

BOOST_AUTO_TEST_CASE(decompose_unparameterized_valued_output_action)
{
  ioa::locker<ioa::automaton_interface*> automata;
  automaton* composer_instance = new automaton ();
  automaton* output_instance = new automaton ();
  automaton* input1_instance = new automaton ();
  automaton* input2_instance = new automaton ();
  ioa::automaton<automaton> composer_automaton (composer_instance);
  ioa::automaton<automaton> output_automaton (output_instance);
  ioa::automaton<automaton> input1_automaton (input1_instance);
  ioa::automaton<automaton> input2_automaton (input2_instance);
  ioa::automaton_handle<automaton> composer_handle = automata.insert (&composer_automaton);
  ioa::automaton_handle<automaton> output_handle = automata.insert (&output_automaton);
  ioa::automaton_handle<automaton> input1_handle = automata.insert (&input1_automaton);
  ioa::automaton_handle<automaton> input2_handle = automata.insert (&input2_automaton);

  int input_parameter;
  ioa::parameter_handle<int> input_parameter_handle = input2_automaton.declare_parameter (&input_parameter);

  ioa::action<automaton::up_v_output_action> output (output_handle, output_instance->up_v_output);
  ioa::action<automaton::up_v_input_action> input1 (input1_handle, input1_instance->up_v_input, composer_handle);
  ioa::action<automaton::p_v_input_action> input2 (input2_handle, input2_instance->p_v_input, input_parameter_handle, composer_handle);

  ioa::composition<automaton::up_v_output_action> composition (output);
  composition.compose (input1);
  composition.compose (input2);
  composition.decompose (input1);
  composition.decompose (input2);

  BOOST_CHECK (composition.empty ());
  BOOST_CHECK (composition.involves_output (output));
  BOOST_CHECK (!composition.involves_input (input1, true));
  ioa::action<automaton::p_v_input_action> input_test (input2_handle, input2_instance->p_v_input, input_parameter_handle, input2_handle);
  BOOST_CHECK (!composition.involves_input (input_test, false));
  BOOST_CHECK (!composition.involves_input_automaton (input1_handle));
  BOOST_CHECK (!composition.involves_input_automaton (input2_handle));
}

BOOST_AUTO_TEST_CASE(decompose_parameterized_valued_output_action)
{
  ioa::locker<ioa::automaton_interface*> automata;
  automaton* composer_instance = new automaton ();
  automaton* output_instance = new automaton ();
  automaton* input1_instance = new automaton ();
  automaton* input2_instance = new automaton ();
  ioa::automaton<automaton> composer_automaton (composer_instance);
  ioa::automaton<automaton> output_automaton (output_instance);
  ioa::automaton<automaton> input1_automaton (input1_instance);
  ioa::automaton<automaton> input2_automaton (input2_instance);
  ioa::automaton_handle<automaton> composer_handle = automata.insert (&composer_automaton);
  ioa::automaton_handle<automaton> output_handle = automata.insert (&output_automaton);
  ioa::automaton_handle<automaton> input1_handle = automata.insert (&input1_automaton);
  ioa::automaton_handle<automaton> input2_handle = automata.insert (&input2_automaton);

  int output_parameter;
  ioa::parameter_handle<int> output_parameter_handle = output_automaton.declare_parameter (&output_parameter);
  int input_parameter;
  ioa::parameter_handle<int> input_parameter_handle = input2_automaton.declare_parameter (&input_parameter);

  ioa::action<automaton::p_v_output_action> output (output_handle, output_instance->p_v_output, output_parameter_handle);
  ioa::action<automaton::up_v_input_action> input1 (input1_handle, input1_instance->up_v_input, composer_handle);
  ioa::action<automaton::p_v_input_action> input2 (input2_handle, input2_instance->p_v_input, input_parameter_handle, composer_handle);

  ioa::composition<automaton::p_v_output_action> composition (output);
  composition.compose (input1);
  composition.compose (input2);
  composition.decompose (input1);
  composition.decompose (input2);

  BOOST_CHECK (composition.empty ());
  BOOST_CHECK (composition.involves_output (output));
  BOOST_CHECK (!composition.involves_input (input1, true));
  ioa::action<automaton::p_v_input_action> input_test (input2_handle, input2_instance->p_v_input, input_parameter_handle, input2_handle);
  BOOST_CHECK (!composition.involves_input (input_test, false));
  BOOST_CHECK (!composition.involves_input_automaton (input1_handle));
  BOOST_CHECK (!composition.involves_input_automaton (input2_handle));
}

BOOST_AUTO_TEST_SUITE_END()
