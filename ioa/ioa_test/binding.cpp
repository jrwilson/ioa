#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE binding
#include <boost/test/unit_test.hpp>

#include <binding.hpp>
#include "automaton.hpp"

class dummy_scheduler :
  public ioa::scheduler_interface
{
public:
  void set_current_handle (const ioa::generic_automaton_handle&) { }
  void clear_current_handle (void) { }
};

BOOST_AUTO_TEST_SUITE(binding_suite)

BOOST_AUTO_TEST_CASE(bind_unparameterized_unvalued_output_action)
{
  ioa::locker<ioa::automaton_interface*> automata;
  automaton* binder_instance = new automaton ();
  automaton* output_instance = new automaton ();
  automaton* input1_instance = new automaton ();
  automaton* input2_instance = new automaton ();
  ioa::automaton<automaton> binder_automaton (binder_instance);
  ioa::automaton<automaton> output_automaton (output_instance);
  ioa::automaton<automaton> input1_automaton (input1_instance);
  ioa::automaton<automaton> input2_automaton (input2_instance);
  ioa::automaton_handle<automaton> binder_handle = automata.insert (&binder_automaton);
  ioa::automaton_handle<automaton> output_handle = automata.insert (&output_automaton);
  ioa::automaton_handle<automaton> input1_handle = automata.insert (&input1_automaton);
  ioa::automaton_handle<automaton> input2_handle = automata.insert (&input2_automaton);

  int input_parameter;
  ioa::parameter_handle<int> input_parameter_handle = input2_automaton.declare_parameter (&input_parameter);

  ioa::action<automaton::up_uv_output_action> output (output_handle, &automaton::up_uv_output);
  ioa::action<automaton::up_uv_input_action> input1 (input1_handle, &automaton::up_uv_input);
  ioa::action<automaton::p_uv_input_action> input2 (input2_handle, &automaton::p_uv_input, input_parameter_handle);

  ioa::binding<automaton::up_uv_output_action> binding (output);
  binding.bind (input1, binder_handle);
  binding.bind (input2, binder_handle);

  BOOST_CHECK (!binding.empty ());
  BOOST_CHECK (binding.involves_output (output));
  BOOST_CHECK (binding.involves_input (input1, binder_handle));
  ioa::action<automaton::p_uv_input_action> input_test (input2_handle, &automaton::p_uv_input, input_parameter_handle);
  BOOST_CHECK (binding.involves_input (input_test));
  BOOST_CHECK (binding.involves_input_automaton (input1_handle));
  BOOST_CHECK (binding.involves_input_automaton (input2_handle));

  dummy_scheduler scheduler;
  binding.execute (scheduler);

  BOOST_CHECK (output_instance->up_uv_output.state);
  BOOST_CHECK (input1_instance->up_uv_input.state);
  BOOST_CHECK (input2_instance->p_uv_input.state);
  BOOST_CHECK_EQUAL (input2_instance->p_uv_input.last_parameter, &input_parameter);
}

BOOST_AUTO_TEST_CASE(bind_parameterized_unvalued_output_action)
{
  ioa::locker<ioa::automaton_interface*> automata;
  automaton* binder_instance = new automaton ();
  automaton* output_instance = new automaton ();
  automaton* input1_instance = new automaton ();
  automaton* input2_instance = new automaton ();
  ioa::automaton<automaton> binder_automaton (binder_instance);
  ioa::automaton<automaton> output_automaton (output_instance);
  ioa::automaton<automaton> input1_automaton (input1_instance);
  ioa::automaton<automaton> input2_automaton (input2_instance);
  ioa::automaton_handle<automaton> binder_handle = automata.insert (&binder_automaton);
  ioa::automaton_handle<automaton> output_handle = automata.insert (&output_automaton);
  ioa::automaton_handle<automaton> input1_handle = automata.insert (&input1_automaton);
  ioa::automaton_handle<automaton> input2_handle = automata.insert (&input2_automaton);

  int output_parameter;
  ioa::parameter_handle<int> output_parameter_handle = output_automaton.declare_parameter (&output_parameter);
  int input_parameter;
  ioa::parameter_handle<int> input_parameter_handle = input2_automaton.declare_parameter (&input_parameter);

  ioa::action<automaton::p_uv_output_action> output (output_handle, &automaton::p_uv_output, output_parameter_handle);
  ioa::action<automaton::up_uv_input_action> input1 (input1_handle, &automaton::up_uv_input);
  ioa::action<automaton::p_uv_input_action> input2 (input2_handle, &automaton::p_uv_input, input_parameter_handle);

  ioa::binding<automaton::p_uv_output_action> binding (output);
  binding.bind (input1, binder_handle);
  binding.bind (input2, binder_handle);

  BOOST_CHECK (!binding.empty ());
  BOOST_CHECK (binding.involves_output (output));
  BOOST_CHECK (binding.involves_input (input1, binder_handle));
  ioa::action<automaton::p_uv_input_action> input_test (input2_handle, &automaton::p_uv_input, input_parameter_handle);
  BOOST_CHECK (binding.involves_input (input_test));
  BOOST_CHECK (binding.involves_input_automaton (input1_handle));
  BOOST_CHECK (binding.involves_input_automaton (input2_handle));

  dummy_scheduler scheduler;
  binding.execute (scheduler);

  BOOST_CHECK (output_instance->p_uv_output.state);
  BOOST_CHECK_EQUAL (output_instance->p_uv_output.last_parameter, &output_parameter);
  BOOST_CHECK (input1_instance->up_uv_input.state);
  BOOST_CHECK (input2_instance->p_uv_input.state);
  BOOST_CHECK_EQUAL (input2_instance->p_uv_input.last_parameter, &input_parameter);
}

BOOST_AUTO_TEST_CASE(bind_unparameterized_valued_output_action)
{
  ioa::locker<ioa::automaton_interface*> automata;
  automaton* binder_instance = new automaton ();
  automaton* output_instance = new automaton ();
  automaton* input1_instance = new automaton ();
  automaton* input2_instance = new automaton ();
  ioa::automaton<automaton> binder_automaton (binder_instance);
  ioa::automaton<automaton> output_automaton (output_instance);
  ioa::automaton<automaton> input1_automaton (input1_instance);
  ioa::automaton<automaton> input2_automaton (input2_instance);
  ioa::automaton_handle<automaton> binder_handle = automata.insert (&binder_automaton);
  ioa::automaton_handle<automaton> output_handle = automata.insert (&output_automaton);
  ioa::automaton_handle<automaton> input1_handle = automata.insert (&input1_automaton);
  ioa::automaton_handle<automaton> input2_handle = automata.insert (&input2_automaton);

  int input_parameter;
  ioa::parameter_handle<int> input_parameter_handle = input2_automaton.declare_parameter (&input_parameter);

  ioa::action<automaton::up_v_output_action> output (output_handle, &automaton::up_v_output);
  ioa::action<automaton::up_v_input_action> input1 (input1_handle, &automaton::up_v_input);
  ioa::action<automaton::p_v_input_action> input2 (input2_handle, &automaton::p_v_input, input_parameter_handle);

  ioa::binding<automaton::up_v_output_action> binding (output);
  binding.bind (input1, binder_handle);
  binding.bind (input2, binder_handle);

  BOOST_CHECK (!binding.empty ());
  BOOST_CHECK (binding.involves_output (output));
  BOOST_CHECK (binding.involves_input (input1, binder_handle));
  ioa::action<automaton::p_v_input_action> input_test (input2_handle, &automaton::p_v_input, input_parameter_handle);
  BOOST_CHECK (binding.involves_input (input_test));
  BOOST_CHECK (binding.involves_input_automaton (input1_handle));
  BOOST_CHECK (binding.involves_input_automaton (input2_handle));

  dummy_scheduler scheduler;
  binding.execute (scheduler);

  BOOST_CHECK (output_instance->up_v_output.state);
  BOOST_CHECK_EQUAL (input1_instance->up_v_input.value, 9845);
  BOOST_CHECK_EQUAL (input2_instance->p_v_input.value, 9845);
  BOOST_CHECK_EQUAL (input2_instance->p_v_input.last_parameter, &input_parameter);
}

BOOST_AUTO_TEST_CASE(bind_parameterized_valued_output_action)
{
  ioa::locker<ioa::automaton_interface*> automata;
  automaton* binder_instance = new automaton ();
  automaton* output_instance = new automaton ();
  automaton* input1_instance = new automaton ();
  automaton* input2_instance = new automaton ();
  ioa::automaton<automaton> binder_automaton (binder_instance);
  ioa::automaton<automaton> output_automaton (output_instance);
  ioa::automaton<automaton> input1_automaton (input1_instance);
  ioa::automaton<automaton> input2_automaton (input2_instance);
  ioa::automaton_handle<automaton> binder_handle = automata.insert (&binder_automaton);
  ioa::automaton_handle<automaton> output_handle = automata.insert (&output_automaton);
  ioa::automaton_handle<automaton> input1_handle = automata.insert (&input1_automaton);
  ioa::automaton_handle<automaton> input2_handle = automata.insert (&input2_automaton);

  int output_parameter;
  ioa::parameter_handle<int> output_parameter_handle = output_automaton.declare_parameter (&output_parameter);
  int input_parameter;
  ioa::parameter_handle<int> input_parameter_handle = input2_automaton.declare_parameter (&input_parameter);

  ioa::action<automaton::p_v_output_action> output (output_handle, &automaton::p_v_output, output_parameter_handle);
  ioa::action<automaton::up_v_input_action> input1 (input1_handle, &automaton::up_v_input);
  ioa::action<automaton::p_v_input_action> input2 (input2_handle, &automaton::p_v_input, input_parameter_handle);

  ioa::binding<automaton::p_v_output_action> binding (output);
  binding.bind (input1, binder_handle);
  binding.bind (input2, binder_handle);

  BOOST_CHECK (!binding.empty ());
  BOOST_CHECK (binding.involves_output (output));
  BOOST_CHECK (binding.involves_input (input1, binder_handle));
  ioa::action<automaton::p_v_input_action> input_test (input2_handle, &automaton::p_v_input, input_parameter_handle);
  BOOST_CHECK (binding.involves_input (input_test));
  BOOST_CHECK (binding.involves_input_automaton (input1_handle));
  BOOST_CHECK (binding.involves_input_automaton (input2_handle));

  dummy_scheduler scheduler;
  binding.execute (scheduler);

  BOOST_CHECK (output_instance->p_v_output.state);
  BOOST_CHECK_EQUAL (output_instance->p_v_output.last_parameter, &output_parameter);
  BOOST_CHECK_EQUAL (input1_instance->up_v_input.value, 9845);
  BOOST_CHECK_EQUAL (input2_instance->p_v_input.value, 9845);
  BOOST_CHECK_EQUAL (input2_instance->p_v_input.last_parameter, &input_parameter);
}

BOOST_AUTO_TEST_CASE(unbind_unparameterized_unvalued_output_action)
{
  ioa::locker<ioa::automaton_interface*> automata;
  automaton* binder_instance = new automaton ();
  automaton* output_instance = new automaton ();
  automaton* input1_instance = new automaton ();
  automaton* input2_instance = new automaton ();
  ioa::automaton<automaton> binder_automaton (binder_instance);
  ioa::automaton<automaton> output_automaton (output_instance);
  ioa::automaton<automaton> input1_automaton (input1_instance);
  ioa::automaton<automaton> input2_automaton (input2_instance);
  ioa::automaton_handle<automaton> binder_handle = automata.insert (&binder_automaton);
  ioa::automaton_handle<automaton> output_handle = automata.insert (&output_automaton);
  ioa::automaton_handle<automaton> input1_handle = automata.insert (&input1_automaton);
  ioa::automaton_handle<automaton> input2_handle = automata.insert (&input2_automaton);

  int input_parameter;
  ioa::parameter_handle<int> input_parameter_handle = input2_automaton.declare_parameter (&input_parameter);

  ioa::action<automaton::up_uv_output_action> output (output_handle, &automaton::up_uv_output);
  ioa::action<automaton::up_uv_input_action> input1 (input1_handle, &automaton::up_uv_input);
  ioa::action<automaton::p_uv_input_action> input2 (input2_handle, &automaton::p_uv_input, input_parameter_handle);

  ioa::binding<automaton::up_uv_output_action> binding (output);
  binding.bind (input1, binder_handle);
  binding.bind (input2, binder_handle);
  binding.unbind (input1, binder_handle);
  binding.unbind (input2, binder_handle);

  BOOST_CHECK (binding.empty ());
  BOOST_CHECK (binding.involves_output (output));
  BOOST_CHECK (!binding.involves_input (input1, binder_handle));
  ioa::action<automaton::p_uv_input_action> input_test (input2_handle, &automaton::p_uv_input, input_parameter_handle);
  BOOST_CHECK (!binding.involves_input (input_test));
  BOOST_CHECK (!binding.involves_input_automaton (input1_handle));
  BOOST_CHECK (!binding.involves_input_automaton (input2_handle));
}

BOOST_AUTO_TEST_CASE(unbind_parameterized_unvalued_output_action)
{
  ioa::locker<ioa::automaton_interface*> automata;
  automaton* binder_instance = new automaton ();
  automaton* output_instance = new automaton ();
  automaton* input1_instance = new automaton ();
  automaton* input2_instance = new automaton ();
  ioa::automaton<automaton> binder_automaton (binder_instance);
  ioa::automaton<automaton> output_automaton (output_instance);
  ioa::automaton<automaton> input1_automaton (input1_instance);
  ioa::automaton<automaton> input2_automaton (input2_instance);
  ioa::automaton_handle<automaton> binder_handle = automata.insert (&binder_automaton);
  ioa::automaton_handle<automaton> output_handle = automata.insert (&output_automaton);
  ioa::automaton_handle<automaton> input1_handle = automata.insert (&input1_automaton);
  ioa::automaton_handle<automaton> input2_handle = automata.insert (&input2_automaton);

  int output_parameter;
  ioa::parameter_handle<int> output_parameter_handle = output_automaton.declare_parameter (&output_parameter);
  int input_parameter;
  ioa::parameter_handle<int> input_parameter_handle = input2_automaton.declare_parameter (&input_parameter);

  ioa::action<automaton::p_uv_output_action> output (output_handle, &automaton::p_uv_output, output_parameter_handle);
  ioa::action<automaton::up_uv_input_action> input1 (input1_handle, &automaton::up_uv_input);
  ioa::action<automaton::p_uv_input_action> input2 (input2_handle, &automaton::p_uv_input, input_parameter_handle);

  ioa::binding<automaton::p_uv_output_action> binding (output);
  binding.bind (input1, binder_handle);
  binding.bind (input2, binder_handle);
  binding.unbind (input1, binder_handle);
  binding.unbind (input2, binder_handle);

  BOOST_CHECK (binding.empty ());
  BOOST_CHECK (binding.involves_output (output));
  BOOST_CHECK (!binding.involves_input (input1, binder_handle));
  ioa::action<automaton::p_uv_input_action> input_test (input2_handle, &automaton::p_uv_input, input_parameter_handle);
  BOOST_CHECK (!binding.involves_input (input_test));
  BOOST_CHECK (!binding.involves_input_automaton (input1_handle));
  BOOST_CHECK (!binding.involves_input_automaton (input2_handle));
}

BOOST_AUTO_TEST_CASE(unbind_unparameterized_valued_output_action)
{
  ioa::locker<ioa::automaton_interface*> automata;
  automaton* binder_instance = new automaton ();
  automaton* output_instance = new automaton ();
  automaton* input1_instance = new automaton ();
  automaton* input2_instance = new automaton ();
  ioa::automaton<automaton> binder_automaton (binder_instance);
  ioa::automaton<automaton> output_automaton (output_instance);
  ioa::automaton<automaton> input1_automaton (input1_instance);
  ioa::automaton<automaton> input2_automaton (input2_instance);
  ioa::automaton_handle<automaton> binder_handle = automata.insert (&binder_automaton);
  ioa::automaton_handle<automaton> output_handle = automata.insert (&output_automaton);
  ioa::automaton_handle<automaton> input1_handle = automata.insert (&input1_automaton);
  ioa::automaton_handle<automaton> input2_handle = automata.insert (&input2_automaton);

  int input_parameter;
  ioa::parameter_handle<int> input_parameter_handle = input2_automaton.declare_parameter (&input_parameter);

  ioa::action<automaton::up_v_output_action> output (output_handle, &automaton::up_v_output);
  ioa::action<automaton::up_v_input_action> input1 (input1_handle, &automaton::up_v_input);
  ioa::action<automaton::p_v_input_action> input2 (input2_handle, &automaton::p_v_input, input_parameter_handle);

  ioa::binding<automaton::up_v_output_action> binding (output);
  binding.bind (input1, binder_handle);
  binding.bind (input2, binder_handle);
  binding.unbind (input1, binder_handle);
  binding.unbind (input2, binder_handle);

  BOOST_CHECK (binding.empty ());
  BOOST_CHECK (binding.involves_output (output));
  BOOST_CHECK (!binding.involves_input (input1, binder_handle));
  ioa::action<automaton::p_v_input_action> input_test (input2_handle, &automaton::p_v_input, input_parameter_handle);
  BOOST_CHECK (!binding.involves_input (input_test));
  BOOST_CHECK (!binding.involves_input_automaton (input1_handle));
  BOOST_CHECK (!binding.involves_input_automaton (input2_handle));
}

BOOST_AUTO_TEST_CASE(unbind_parameterized_valued_output_action)
{
  ioa::locker<ioa::automaton_interface*> automata;
  automaton* binder_instance = new automaton ();
  automaton* output_instance = new automaton ();
  automaton* input1_instance = new automaton ();
  automaton* input2_instance = new automaton ();
  ioa::automaton<automaton> binder_automaton (binder_instance);
  ioa::automaton<automaton> output_automaton (output_instance);
  ioa::automaton<automaton> input1_automaton (input1_instance);
  ioa::automaton<automaton> input2_automaton (input2_instance);
  ioa::automaton_handle<automaton> binder_handle = automata.insert (&binder_automaton);
  ioa::automaton_handle<automaton> output_handle = automata.insert (&output_automaton);
  ioa::automaton_handle<automaton> input1_handle = automata.insert (&input1_automaton);
  ioa::automaton_handle<automaton> input2_handle = automata.insert (&input2_automaton);

  int output_parameter;
  ioa::parameter_handle<int> output_parameter_handle = output_automaton.declare_parameter (&output_parameter);
  int input_parameter;
  ioa::parameter_handle<int> input_parameter_handle = input2_automaton.declare_parameter (&input_parameter);

  ioa::action<automaton::p_v_output_action> output (output_handle, &automaton::p_v_output, output_parameter_handle);
  ioa::action<automaton::up_v_input_action> input1 (input1_handle, &automaton::up_v_input);
  ioa::action<automaton::p_v_input_action> input2 (input2_handle, &automaton::p_v_input, input_parameter_handle);

  ioa::binding<automaton::p_v_output_action> binding (output);
  binding.bind (input1, binder_handle);
  binding.bind (input2, binder_handle);
  binding.unbind (input1, binder_handle);
  binding.unbind (input2, binder_handle);

  BOOST_CHECK (binding.empty ());
  BOOST_CHECK (binding.involves_output (output));
  BOOST_CHECK (!binding.involves_input (input1, binder_handle));
  ioa::action<automaton::p_v_input_action> input_test (input2_handle, &automaton::p_v_input, input_parameter_handle);
  BOOST_CHECK (!binding.involves_input (input_test));
  BOOST_CHECK (!binding.involves_input_automaton (input1_handle));
  BOOST_CHECK (!binding.involves_input_automaton (input2_handle));
}

BOOST_AUTO_TEST_SUITE_END()
