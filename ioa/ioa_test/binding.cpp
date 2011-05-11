#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE binding
#include <boost/test/unit_test.hpp>

#include <binding.hpp>
#include "automaton1.hpp"

class dummy_scheduler :
  public ioa::scheduler_interface
{
public:
  void set_current_handle (const ioa::generic_automaton_handle&) { }
  void clear_current_handle (void) { }
};

class dummy_system :
  public ioa::system_interface
{
public:
  void lock_automaton (const ioa::generic_automaton_handle&) { }
  void unlock_automaton (const ioa::generic_automaton_handle&) { }
};

struct dummy_unbind_success_listener
{
  template <class OM, class IM>
  void unbound (const ioa::action<OM>& output_action,
		const ioa::action<IM>& input_action,
		const ioa::generic_automaton_handle& binder) { }
};

BOOST_AUTO_TEST_SUITE(binding_suite)

BOOST_AUTO_TEST_CASE(bind_unparameterized_unvalued_output_action)
{
  std::auto_ptr<automaton1> binder_instance (new automaton1 ());
  std::auto_ptr<automaton1> output_instance (new automaton1 ());
  std::auto_ptr<automaton1> input1_instance (new automaton1 ());
  std::auto_ptr<automaton1> input2_instance (new automaton1 ());
  ioa::automaton_handle<automaton1> binder_handle (binder_instance.get ());
  ioa::automaton_handle<automaton1> output_handle (output_instance.get ());
  ioa::automaton_handle<automaton1> input1_handle (input1_instance.get ());
  ioa::automaton_handle<automaton1> input2_handle (input2_instance.get ());

  int input_parameter;
  ioa::parameter_handle<int> input_parameter_handle (&input_parameter);

  ioa::action<automaton1::up_uv_output_action> output (output_handle, &automaton1::up_uv_output);
  ioa::action<automaton1::up_uv_input_action> input1 (input1_handle, &automaton1::up_uv_input);
  ioa::action<automaton1::p_uv_input_action> input2 (input2_handle, &automaton1::p_uv_input, input_parameter_handle);


  ioa::binding<automaton1::up_uv_output_action> binding;
  dummy_unbind_success_listener usl1;
  binding.bind (output, input1, binder_handle, usl1);
  dummy_unbind_success_listener usl2;
  binding.bind (output, input2, binder_handle, usl2);

  BOOST_CHECK (!binding.empty ());
  BOOST_CHECK (binding.involves_output (output));
  BOOST_CHECK (binding.involves_input (input1, binder_handle));
  ioa::action<automaton1::p_uv_input_action> input_test (input2_handle, &automaton1::p_uv_input, input_parameter_handle);
  BOOST_CHECK (binding.involves_input (input_test));
  BOOST_CHECK (binding.involves_input_automaton (input1_handle));
  BOOST_CHECK (binding.involves_input_automaton (input2_handle));

  dummy_scheduler scheduler;
  dummy_system system;
  binding.execute (scheduler, system);

  BOOST_CHECK (output_instance->up_uv_output.state);
  BOOST_CHECK (input1_instance->up_uv_input.state);
  BOOST_CHECK (input2_instance->p_uv_input.state);
  BOOST_CHECK_EQUAL (input2_instance->p_uv_input.last_parameter, &input_parameter);
}

BOOST_AUTO_TEST_CASE(bind_parameterized_unvalued_output_action)
{
  std::auto_ptr<automaton1> binder_instance (new automaton1 ());
  std::auto_ptr<automaton1> output_instance (new automaton1 ());
  std::auto_ptr<automaton1> input1_instance (new automaton1 ());
  std::auto_ptr<automaton1> input2_instance (new automaton1 ());
  ioa::automaton_handle<automaton1> binder_handle (binder_instance.get ());
  ioa::automaton_handle<automaton1> output_handle (output_instance.get ());
  ioa::automaton_handle<automaton1> input1_handle (input1_instance.get ());
  ioa::automaton_handle<automaton1> input2_handle (input2_instance.get ());

  int output_parameter;
  ioa::parameter_handle<int> output_parameter_handle (&output_parameter);
  int input_parameter;
  ioa::parameter_handle<int> input_parameter_handle (&input_parameter);

  ioa::action<automaton1::p_uv_output_action> output (output_handle, &automaton1::p_uv_output, output_parameter_handle);
  ioa::action<automaton1::up_uv_input_action> input1 (input1_handle, &automaton1::up_uv_input);
  ioa::action<automaton1::p_uv_input_action> input2 (input2_handle, &automaton1::p_uv_input, input_parameter_handle);

  ioa::binding<automaton1::p_uv_output_action> binding;
  dummy_unbind_success_listener usl1;
  binding.bind (output, input1, binder_handle, usl1);
  dummy_unbind_success_listener usl2;
  binding.bind (output, input2, binder_handle, usl2);

  BOOST_CHECK (!binding.empty ());
  BOOST_CHECK (binding.involves_output (output));
  BOOST_CHECK (binding.involves_input (input1, binder_handle));
  ioa::action<automaton1::p_uv_input_action> input_test (input2_handle, &automaton1::p_uv_input, input_parameter_handle);
  BOOST_CHECK (binding.involves_input (input_test));
  BOOST_CHECK (binding.involves_input_automaton (input1_handle));
  BOOST_CHECK (binding.involves_input_automaton (input2_handle));

  dummy_scheduler scheduler;
  dummy_system system;
  binding.execute (scheduler, system);

  BOOST_CHECK (output_instance->p_uv_output.state);
  BOOST_CHECK_EQUAL (output_instance->p_uv_output.last_parameter, &output_parameter);
  BOOST_CHECK (input1_instance->up_uv_input.state);
  BOOST_CHECK (input2_instance->p_uv_input.state);
  BOOST_CHECK_EQUAL (input2_instance->p_uv_input.last_parameter, &input_parameter);
}

BOOST_AUTO_TEST_CASE(bind_unparameterized_valued_output_action)
{
  std::auto_ptr<automaton1> binder_instance (new automaton1 ());
  std::auto_ptr<automaton1> output_instance (new automaton1 ());
  std::auto_ptr<automaton1> input1_instance (new automaton1 ());
  std::auto_ptr<automaton1> input2_instance (new automaton1 ());
  ioa::automaton_handle<automaton1> binder_handle (binder_instance.get ());
  ioa::automaton_handle<automaton1> output_handle (output_instance.get ());
  ioa::automaton_handle<automaton1> input1_handle (input1_instance.get ());
  ioa::automaton_handle<automaton1> input2_handle (input2_instance.get ());

  int input_parameter;
  ioa::parameter_handle<int> input_parameter_handle (&input_parameter);

  ioa::action<automaton1::up_v_output_action> output (output_handle, &automaton1::up_v_output);
  ioa::action<automaton1::up_v_input_action> input1 (input1_handle, &automaton1::up_v_input);
  ioa::action<automaton1::p_v_input_action> input2 (input2_handle, &automaton1::p_v_input, input_parameter_handle);

  ioa::binding<automaton1::up_v_output_action> binding;
  dummy_unbind_success_listener usl1;
  binding.bind (output, input1, binder_handle, usl1);
  dummy_unbind_success_listener usl2;
  binding.bind (output, input2, binder_handle, usl2);

  BOOST_CHECK (!binding.empty ());
  BOOST_CHECK (binding.involves_output (output));
  BOOST_CHECK (binding.involves_input (input1, binder_handle));
  ioa::action<automaton1::p_v_input_action> input_test (input2_handle, &automaton1::p_v_input, input_parameter_handle);
  BOOST_CHECK (binding.involves_input (input_test));
  BOOST_CHECK (binding.involves_input_automaton (input1_handle));
  BOOST_CHECK (binding.involves_input_automaton (input2_handle));

  dummy_scheduler scheduler;
  dummy_system system;
  binding.execute (scheduler, system);

  BOOST_CHECK (output_instance->up_v_output.state);
  BOOST_CHECK_EQUAL (input1_instance->up_v_input.value, 9845);
  BOOST_CHECK_EQUAL (input2_instance->p_v_input.value, 9845);
  BOOST_CHECK_EQUAL (input2_instance->p_v_input.last_parameter, &input_parameter);
}

BOOST_AUTO_TEST_CASE(bind_parameterized_valued_output_action)
{
  std::auto_ptr<automaton1> binder_instance (new automaton1 ());
  std::auto_ptr<automaton1> output_instance (new automaton1 ());
  std::auto_ptr<automaton1> input1_instance (new automaton1 ());
  std::auto_ptr<automaton1> input2_instance (new automaton1 ());
  ioa::automaton_handle<automaton1> binder_handle (binder_instance.get ());
  ioa::automaton_handle<automaton1> output_handle (output_instance.get ());
  ioa::automaton_handle<automaton1> input1_handle (input1_instance.get ());
  ioa::automaton_handle<automaton1> input2_handle (input2_instance.get ());

  int output_parameter;
  ioa::parameter_handle<int> output_parameter_handle (&output_parameter);
  int input_parameter;
  ioa::parameter_handle<int> input_parameter_handle (&input_parameter);

  ioa::action<automaton1::p_v_output_action> output (output_handle, &automaton1::p_v_output, output_parameter_handle);
  ioa::action<automaton1::up_v_input_action> input1 (input1_handle, &automaton1::up_v_input);
  ioa::action<automaton1::p_v_input_action> input2 (input2_handle, &automaton1::p_v_input, input_parameter_handle);

  ioa::binding<automaton1::p_v_output_action> binding;
  dummy_unbind_success_listener usl1;
  binding.bind (output, input1, binder_handle, usl1);
  dummy_unbind_success_listener usl2;
  binding.bind (output, input2, binder_handle, usl2);

  BOOST_CHECK (!binding.empty ());
  BOOST_CHECK (binding.involves_output (output));
  BOOST_CHECK (binding.involves_input (input1, binder_handle));
  ioa::action<automaton1::p_v_input_action> input_test (input2_handle, &automaton1::p_v_input, input_parameter_handle);
  BOOST_CHECK (binding.involves_input (input_test));
  BOOST_CHECK (binding.involves_input_automaton (input1_handle));
  BOOST_CHECK (binding.involves_input_automaton (input2_handle));

  dummy_scheduler scheduler;
  dummy_system system;
  binding.execute (scheduler, system);

  BOOST_CHECK (output_instance->p_v_output.state);
  BOOST_CHECK_EQUAL (output_instance->p_v_output.last_parameter, &output_parameter);
  BOOST_CHECK_EQUAL (input1_instance->up_v_input.value, 9845);
  BOOST_CHECK_EQUAL (input2_instance->p_v_input.value, 9845);
  BOOST_CHECK_EQUAL (input2_instance->p_v_input.last_parameter, &input_parameter);
}

BOOST_AUTO_TEST_CASE(unbind_unparameterized_unvalued_output_action)
{
  std::auto_ptr<automaton1> binder_instance (new automaton1 ());
  std::auto_ptr<automaton1> output_instance (new automaton1 ());
  std::auto_ptr<automaton1> input1_instance (new automaton1 ());
  std::auto_ptr<automaton1> input2_instance (new automaton1 ());
  ioa::automaton_handle<automaton1> binder_handle (binder_instance.get ());
  ioa::automaton_handle<automaton1> output_handle (output_instance.get ());
  ioa::automaton_handle<automaton1> input1_handle (input1_instance.get ());
  ioa::automaton_handle<automaton1> input2_handle (input2_instance.get ());

  int input_parameter;
  ioa::parameter_handle<int> input_parameter_handle (&input_parameter);

  ioa::action<automaton1::up_uv_output_action> output (output_handle, &automaton1::up_uv_output);
  ioa::action<automaton1::up_uv_input_action> input1 (input1_handle, &automaton1::up_uv_input);
  ioa::action<automaton1::p_uv_input_action> input2 (input2_handle, &automaton1::p_uv_input, input_parameter_handle);

  ioa::binding<automaton1::up_uv_output_action> binding;
  dummy_unbind_success_listener usl1;
  binding.bind (output, input1, binder_handle, usl1);
  dummy_unbind_success_listener usl2;
  binding.bind (output, input2, binder_handle, usl2);
  binding.unbind (output, input1, binder_handle);
  binding.unbind (output, input2, binder_handle);

  BOOST_CHECK (binding.empty ());
  BOOST_CHECK (!binding.involves_output (output));
  BOOST_CHECK (!binding.involves_input (input1, binder_handle));
  ioa::action<automaton1::p_uv_input_action> input_test (input2_handle, &automaton1::p_uv_input, input_parameter_handle);
  BOOST_CHECK (!binding.involves_input (input_test));
  BOOST_CHECK (!binding.involves_input_automaton (input1_handle));
  BOOST_CHECK (!binding.involves_input_automaton (input2_handle));
}

BOOST_AUTO_TEST_CASE(unbind_parameterized_unvalued_output_action)
{
  std::auto_ptr<automaton1> binder_instance (new automaton1 ());
  std::auto_ptr<automaton1> output_instance (new automaton1 ());
  std::auto_ptr<automaton1> input1_instance (new automaton1 ());
  std::auto_ptr<automaton1> input2_instance (new automaton1 ());
  ioa::automaton_handle<automaton1> binder_handle (binder_instance.get ());
  ioa::automaton_handle<automaton1> output_handle (output_instance.get ());
  ioa::automaton_handle<automaton1> input1_handle (input1_instance.get ());
  ioa::automaton_handle<automaton1> input2_handle (input2_instance.get ());

  int output_parameter;
  ioa::parameter_handle<int> output_parameter_handle (&output_parameter);
  int input_parameter;
  ioa::parameter_handle<int> input_parameter_handle (&input_parameter);

  ioa::action<automaton1::p_uv_output_action> output (output_handle, &automaton1::p_uv_output, output_parameter_handle);
  ioa::action<automaton1::up_uv_input_action> input1 (input1_handle, &automaton1::up_uv_input);
  ioa::action<automaton1::p_uv_input_action> input2 (input2_handle, &automaton1::p_uv_input, input_parameter_handle);

  ioa::binding<automaton1::p_uv_output_action> binding;
  dummy_unbind_success_listener usl1;
  binding.bind (output, input1, binder_handle, usl1);
  dummy_unbind_success_listener usl2;
  binding.bind (output, input2, binder_handle, usl2);
  binding.unbind (output, input1, binder_handle);
  binding.unbind (output, input2, binder_handle);

  BOOST_CHECK (binding.empty ());
  BOOST_CHECK (!binding.involves_output (output));
  BOOST_CHECK (!binding.involves_input (input1, binder_handle));
  ioa::action<automaton1::p_uv_input_action> input_test (input2_handle, &automaton1::p_uv_input, input_parameter_handle);
  BOOST_CHECK (!binding.involves_input (input_test));
  BOOST_CHECK (!binding.involves_input_automaton (input1_handle));
  BOOST_CHECK (!binding.involves_input_automaton (input2_handle));
}

BOOST_AUTO_TEST_CASE(unbind_unparameterized_valued_output_action)
{
  std::auto_ptr<automaton1> binder_instance (new automaton1 ());
  std::auto_ptr<automaton1> output_instance (new automaton1 ());
  std::auto_ptr<automaton1> input1_instance (new automaton1 ());
  std::auto_ptr<automaton1> input2_instance (new automaton1 ());
  ioa::automaton_handle<automaton1> binder_handle (binder_instance.get ());
  ioa::automaton_handle<automaton1> output_handle (output_instance.get ());
  ioa::automaton_handle<automaton1> input1_handle (input1_instance.get ());
  ioa::automaton_handle<automaton1> input2_handle (input2_instance.get ());

  int input_parameter;
  ioa::parameter_handle<int> input_parameter_handle (&input_parameter);

  ioa::action<automaton1::up_v_output_action> output (output_handle, &automaton1::up_v_output);
  ioa::action<automaton1::up_v_input_action> input1 (input1_handle, &automaton1::up_v_input);
  ioa::action<automaton1::p_v_input_action> input2 (input2_handle, &automaton1::p_v_input, input_parameter_handle);

  ioa::binding<automaton1::up_v_output_action> binding;
  dummy_unbind_success_listener usl1;
  binding.bind (output, input1, binder_handle, usl1);
  dummy_unbind_success_listener usl2;
  binding.bind (output, input2, binder_handle, usl2);
  binding.unbind (output, input1, binder_handle);
  binding.unbind (output, input2, binder_handle);

  BOOST_CHECK (binding.empty ());
  BOOST_CHECK (!binding.involves_output (output));
  BOOST_CHECK (!binding.involves_input (input1, binder_handle));
  ioa::action<automaton1::p_v_input_action> input_test (input2_handle, &automaton1::p_v_input, input_parameter_handle);
  BOOST_CHECK (!binding.involves_input (input_test));
  BOOST_CHECK (!binding.involves_input_automaton (input1_handle));
  BOOST_CHECK (!binding.involves_input_automaton (input2_handle));
}

BOOST_AUTO_TEST_CASE(unbind_parameterized_valued_output_action)
{
  std::auto_ptr<automaton1> binder_instance (new automaton1 ());
  std::auto_ptr<automaton1> output_instance (new automaton1 ());
  std::auto_ptr<automaton1> input1_instance (new automaton1 ());
  std::auto_ptr<automaton1> input2_instance (new automaton1 ());
  ioa::automaton_handle<automaton1> binder_handle (binder_instance.get ());
  ioa::automaton_handle<automaton1> output_handle (output_instance.get ());
  ioa::automaton_handle<automaton1> input1_handle (input1_instance.get ());
  ioa::automaton_handle<automaton1> input2_handle (input2_instance.get ());

  int output_parameter;
  ioa::parameter_handle<int> output_parameter_handle (&output_parameter);
  int input_parameter;
  ioa::parameter_handle<int> input_parameter_handle (&input_parameter);

  ioa::action<automaton1::p_v_output_action> output (output_handle, &automaton1::p_v_output, output_parameter_handle);
  ioa::action<automaton1::up_v_input_action> input1 (input1_handle, &automaton1::up_v_input);
  ioa::action<automaton1::p_v_input_action> input2 (input2_handle, &automaton1::p_v_input, input_parameter_handle);

  ioa::binding<automaton1::p_v_output_action> binding;
  dummy_unbind_success_listener usl1;
  binding.bind (output, input1, binder_handle, usl1);
  dummy_unbind_success_listener usl2;
  binding.bind (output, input2, binder_handle, usl2);
  binding.unbind (output, input1, binder_handle);
  binding.unbind (output, input2, binder_handle);

  BOOST_CHECK (binding.empty ());
  BOOST_CHECK (!binding.involves_output (output));
  BOOST_CHECK (!binding.involves_input (input1, binder_handle));
  ioa::action<automaton1::p_v_input_action> input_test (input2_handle, &automaton1::p_v_input, input_parameter_handle);
  BOOST_CHECK (!binding.involves_input (input_test));
  BOOST_CHECK (!binding.involves_input_automaton (input1_handle));
  BOOST_CHECK (!binding.involves_input_automaton (input2_handle));
}

BOOST_AUTO_TEST_SUITE_END()
