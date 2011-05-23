#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE binding
#include <boost/test/unit_test.hpp>

#define AUTOMATON_HANDLE_SETTER
#include <binding.hpp>
#include "automaton1.hpp"

class empty_class { };

class dummy_scheduler :
  public ioa::scheduler_interface
{
public:
  void set_current_aid (const ioa::aid_t) { }
  void clear_current_aid (void) { }
};

class dummy_system :
  public ioa::system_interface
{
public:
  void lock_automaton (const ioa::aid_t) { }
  void unlock_automaton (const ioa::aid_t) { }
};

struct dummy_unbind_success_listener
{
  template <class OI, class OM, class II, class IM, class I, class D>
  void unbound (const ioa::action<OI, OM>& output_action,
		const ioa::action<II, IM>& input_action,
		const ioa::automaton_handle<I>& binder,
		D&) { }
};

BOOST_AUTO_TEST_SUITE(binding_suite)

BOOST_AUTO_TEST_CASE(unbind_unparameterized_unvalued_output_action)
{
  std::auto_ptr<automaton1> binder_instance (new automaton1 ());
  std::auto_ptr<automaton1> output_instance (new automaton1 ());
  std::auto_ptr<automaton1> input1_instance (new automaton1 ());
  std::auto_ptr<automaton1> input2_instance (new automaton1 ());
  ioa::automaton_handle<automaton1> binder_handle;
  binder_handle.set_aid (0);
  ioa::automaton_handle<automaton1> output_handle;
  output_handle.set_aid (1);
  ioa::automaton_handle<automaton1> input1_handle;
  input1_handle.set_aid (2);
  ioa::automaton_handle<automaton1> input2_handle;
  input2_handle.set_aid (3);

  int input_parameter;
  ioa::parameter_handle<int> input_parameter_handle;

  ioa::concrete_action<automaton1, automaton1::up_uv_output_action> output (ioa::make_action (output_handle, &automaton1::up_uv_output), output_instance.get ());
  ioa::concrete_action<automaton1, automaton1::up_uv_input_action> input1 (ioa::make_action (input1_handle, &automaton1::up_uv_input), input1_instance.get ());
  ioa::concrete_action<automaton1, automaton1::p_uv_input_action> input2 (ioa::make_action (input2_handle, &automaton1::p_uv_input, input_parameter_handle), input2_instance.get (), &input_parameter);

  ioa::binding<automaton1::up_uv_output_action> binding;
  dummy_unbind_success_listener usl1;
  empty_class d;
  binding.bind (output, input1, binder_handle, usl1, d);
  dummy_unbind_success_listener usl2;
  binding.bind (output, input2, binder_handle, usl2, d);

  BOOST_CHECK (!binding.empty ());
  BOOST_CHECK (binding.involves_output (output));
  BOOST_CHECK (binding.involves_binding (output, input1, binder_handle.aid ()));
  BOOST_CHECK (binding.involves_binding (output, input2, binder_handle.aid ()));
  BOOST_CHECK (binding.involves_input (input1));
  BOOST_CHECK (binding.involves_input (input2));
  BOOST_CHECK (binding.involves_input_automaton (input1_handle.aid ()));
  BOOST_CHECK (binding.involves_input_automaton (input2_handle.aid ()));

  dummy_scheduler sched;
  dummy_system sys;
  binding.execute (sched, sys);

  BOOST_CHECK (output_instance->up_uv_output.state);
  BOOST_CHECK (input1_instance->up_uv_input.state);
  BOOST_CHECK (input2_instance->p_uv_input.state);
  BOOST_CHECK_EQUAL (input2_instance->p_uv_input.last_parameter, &input_parameter);

  binding.unbind (output, input1, binder_handle);

  BOOST_CHECK (!binding.empty ());
  BOOST_CHECK (binding.involves_output (output));
  BOOST_CHECK (!binding.involves_binding (output, input1, binder_handle.aid ()));
  BOOST_CHECK (binding.involves_binding (output, input2, binder_handle.aid ()));
  BOOST_CHECK (!binding.involves_input (input1));
  BOOST_CHECK (binding.involves_input (input2));
  BOOST_CHECK (!binding.involves_input_automaton (input1_handle.aid ()));
  BOOST_CHECK (binding.involves_input_automaton (input2_handle.aid ()));

  binding.unbind (output, input2, binder_handle);

  BOOST_CHECK (binding.empty ());
  BOOST_CHECK (!binding.involves_output (output));
  BOOST_CHECK (!binding.involves_binding (output, input1, binder_handle.aid ()));
  BOOST_CHECK (!binding.involves_binding (output, input2, binder_handle.aid ()));
  BOOST_CHECK (!binding.involves_input (input1));
  BOOST_CHECK (!binding.involves_input (input2));
  BOOST_CHECK (!binding.involves_input_automaton (input1_handle.aid ()));
  BOOST_CHECK (!binding.involves_input_automaton (input2_handle.aid ()));
}

BOOST_AUTO_TEST_CASE(unbind_parameterized_unvalued_output_action)
{
  std::auto_ptr<automaton1> binder_instance (new automaton1 ());
  std::auto_ptr<automaton1> output_instance (new automaton1 ());
  std::auto_ptr<automaton1> input1_instance (new automaton1 ());
  std::auto_ptr<automaton1> input2_instance (new automaton1 ());
  ioa::automaton_handle<automaton1> binder_handle;
  binder_handle.set_aid (0);
  ioa::automaton_handle<automaton1> output_handle;
  output_handle.set_aid (1);
  ioa::automaton_handle<automaton1> input1_handle;
  input1_handle.set_aid (2);
  ioa::automaton_handle<automaton1> input2_handle;
  input2_handle.set_aid (3);

  int output_parameter;
  ioa::parameter_handle<int> output_parameter_handle;
  int input_parameter;
  ioa::parameter_handle<int> input_parameter_handle;

  ioa::concrete_action<automaton1, automaton1::p_uv_output_action> output (ioa::make_action (output_handle, &automaton1::p_uv_output, output_parameter_handle), output_instance.get (), &output_parameter);
  ioa::concrete_action<automaton1, automaton1::up_uv_input_action> input1 (ioa::make_action (input1_handle, &automaton1::up_uv_input), input1_instance.get ());
  ioa::concrete_action<automaton1, automaton1::p_uv_input_action> input2 (ioa::make_action (input2_handle, &automaton1::p_uv_input, input_parameter_handle), input2_instance.get (), &input_parameter);

  ioa::binding<automaton1::p_uv_output_action> binding;
  dummy_unbind_success_listener usl1;
  empty_class d;
  binding.bind (output, input1, binder_handle, usl1, d);
  dummy_unbind_success_listener usl2;
  binding.bind (output, input2, binder_handle, usl2, d);

  BOOST_CHECK (!binding.empty ());
  BOOST_CHECK (binding.involves_output (output));
  BOOST_CHECK (binding.involves_binding (output, input1, binder_handle.aid ()));
  BOOST_CHECK (binding.involves_binding (output, input2, binder_handle.aid ()));
  BOOST_CHECK (binding.involves_input (input1));
  BOOST_CHECK (binding.involves_input (input2));
  BOOST_CHECK (binding.involves_input_automaton (input1_handle.aid ()));
  BOOST_CHECK (binding.involves_input_automaton (input2_handle.aid ()));

  dummy_scheduler sched;
  dummy_system sys;
  binding.execute (sched, sys);

  BOOST_CHECK (output_instance->p_uv_output.state);
  BOOST_CHECK_EQUAL (output_instance->p_uv_output.last_parameter, &output_parameter);
  BOOST_CHECK (input1_instance->up_uv_input.state);
  BOOST_CHECK (input2_instance->p_uv_input.state);
  BOOST_CHECK_EQUAL (input2_instance->p_uv_input.last_parameter, &input_parameter);

  binding.unbind (output, input1, binder_handle);

  BOOST_CHECK (!binding.empty ());
  BOOST_CHECK (binding.involves_output (output));
  BOOST_CHECK (!binding.involves_binding (output, input1, binder_handle.aid ()));
  BOOST_CHECK (binding.involves_binding (output, input2, binder_handle.aid ()));
  BOOST_CHECK (!binding.involves_input (input1));
  BOOST_CHECK (binding.involves_input (input2));
  BOOST_CHECK (!binding.involves_input_automaton (input1_handle.aid ()));
  BOOST_CHECK (binding.involves_input_automaton (input2_handle.aid ()));

  binding.unbind (output, input2, binder_handle);

  BOOST_CHECK (binding.empty ());
  BOOST_CHECK (!binding.involves_output (output));
  BOOST_CHECK (!binding.involves_binding (output, input1, binder_handle.aid ()));
  BOOST_CHECK (!binding.involves_binding (output, input2, binder_handle.aid ()));
  BOOST_CHECK (!binding.involves_input (input1));
  BOOST_CHECK (!binding.involves_input (input2));
  BOOST_CHECK (!binding.involves_input_automaton (input1_handle.aid ()));
  BOOST_CHECK (!binding.involves_input_automaton (input2_handle.aid ()));
}

BOOST_AUTO_TEST_CASE(unbind_unparameterized_valued_output_action)
{
  std::auto_ptr<automaton1> binder_instance (new automaton1 ());
  std::auto_ptr<automaton1> output_instance (new automaton1 ());
  std::auto_ptr<automaton1> input1_instance (new automaton1 ());
  std::auto_ptr<automaton1> input2_instance (new automaton1 ());
  ioa::automaton_handle<automaton1> binder_handle;
  binder_handle.set_aid (0);
  ioa::automaton_handle<automaton1> output_handle;
  output_handle.set_aid (1);
  ioa::automaton_handle<automaton1> input1_handle;
  input1_handle.set_aid (2);
  ioa::automaton_handle<automaton1> input2_handle;
  input2_handle.set_aid (3);

  int input_parameter;
  ioa::parameter_handle<int> input_parameter_handle;

  ioa::concrete_action<automaton1, automaton1::up_v_output_action> output (ioa::make_action (output_handle, &automaton1::up_v_output), output_instance.get ());
  ioa::concrete_action<automaton1, automaton1::up_v_input_action> input1 (ioa::make_action (input1_handle, &automaton1::up_v_input), input1_instance.get ());
  ioa::concrete_action<automaton1, automaton1::p_v_input_action> input2 (ioa::make_action (input2_handle, &automaton1::p_v_input, input_parameter_handle), input2_instance.get (), &input_parameter);

  ioa::binding<automaton1::up_v_output_action> binding;
  dummy_unbind_success_listener usl1;
  empty_class d;
  binding.bind (output, input1, binder_handle, usl1, d);
  dummy_unbind_success_listener usl2;
  binding.bind (output, input2, binder_handle, usl2, d);

  BOOST_CHECK (!binding.empty ());
  BOOST_CHECK (binding.involves_output (output));
  BOOST_CHECK (binding.involves_binding (output, input1, binder_handle.aid ()));
  BOOST_CHECK (binding.involves_binding (output, input2, binder_handle.aid ()));
  BOOST_CHECK (binding.involves_input (input1));
  BOOST_CHECK (binding.involves_input (input2));
  BOOST_CHECK (binding.involves_input_automaton (input1_handle.aid ()));
  BOOST_CHECK (binding.involves_input_automaton (input2_handle.aid ()));

  dummy_scheduler sched;
  dummy_system sys;
  binding.execute (sched, sys);

  BOOST_CHECK (output_instance->up_v_output.state);
  BOOST_CHECK_EQUAL (input1_instance->up_v_input.value, 9845);
  BOOST_CHECK_EQUAL (input2_instance->p_v_input.value, 9845);
  BOOST_CHECK_EQUAL (input2_instance->p_v_input.last_parameter, &input_parameter);

  binding.unbind (output, input1, binder_handle);

  BOOST_CHECK (!binding.empty ());
  BOOST_CHECK (binding.involves_output (output));
  BOOST_CHECK (!binding.involves_binding (output, input1, binder_handle.aid ()));
  BOOST_CHECK (binding.involves_binding (output, input2, binder_handle.aid ()));
  BOOST_CHECK (!binding.involves_input (input1));
  BOOST_CHECK (binding.involves_input (input2));
  BOOST_CHECK (!binding.involves_input_automaton (input1_handle.aid ()));
  BOOST_CHECK (binding.involves_input_automaton (input2_handle.aid ()));

  binding.unbind (output, input2, binder_handle);

  BOOST_CHECK (binding.empty ());
  BOOST_CHECK (!binding.involves_output (output));
  BOOST_CHECK (!binding.involves_binding (output, input1, binder_handle.aid ()));
  BOOST_CHECK (!binding.involves_binding (output, input2, binder_handle.aid ()));
  BOOST_CHECK (!binding.involves_input (input1));
  BOOST_CHECK (!binding.involves_input (input2));
  BOOST_CHECK (!binding.involves_input_automaton (input1_handle.aid ()));
  BOOST_CHECK (!binding.involves_input_automaton (input2_handle.aid ()));
}

BOOST_AUTO_TEST_CASE(unbind_parameterized_valued_output_action)
{
  std::auto_ptr<automaton1> binder_instance (new automaton1 ());
  std::auto_ptr<automaton1> output_instance (new automaton1 ());
  std::auto_ptr<automaton1> input1_instance (new automaton1 ());
  std::auto_ptr<automaton1> input2_instance (new automaton1 ());
  ioa::automaton_handle<automaton1> binder_handle;
  binder_handle.set_aid (0);
  ioa::automaton_handle<automaton1> output_handle;
  output_handle.set_aid (1);
  ioa::automaton_handle<automaton1> input1_handle;
  input1_handle.set_aid (2);
  ioa::automaton_handle<automaton1> input2_handle;
  input2_handle.set_aid (3);

  int output_parameter;
  ioa::parameter_handle<int> output_parameter_handle;
  int input_parameter;
  ioa::parameter_handle<int> input_parameter_handle;

  ioa::concrete_action<automaton1, automaton1::p_v_output_action> output (ioa::make_action (output_handle, &automaton1::p_v_output, output_parameter_handle), output_instance.get (), &output_parameter);
  ioa::concrete_action<automaton1, automaton1::up_v_input_action> input1 (ioa::make_action (input1_handle, &automaton1::up_v_input), input1_instance.get ());
  ioa::concrete_action<automaton1, automaton1::p_v_input_action> input2 (ioa::make_action (input2_handle, &automaton1::p_v_input, input_parameter_handle), input2_instance.get (), &input_parameter);

  ioa::binding<automaton1::p_v_output_action> binding;
  dummy_unbind_success_listener usl1;
  empty_class d;
  binding.bind (output, input1, binder_handle, usl1, d);
  dummy_unbind_success_listener usl2;
  binding.bind (output, input2, binder_handle, usl2, d);

  dummy_scheduler sched;
  dummy_system sys;
  binding.execute (sched, sys);

  BOOST_CHECK (output_instance->p_v_output.state);
  BOOST_CHECK_EQUAL (output_instance->p_v_output.last_parameter, &output_parameter);
  BOOST_CHECK_EQUAL (input1_instance->up_v_input.value, 9845);
  BOOST_CHECK_EQUAL (input2_instance->p_v_input.value, 9845);
  BOOST_CHECK_EQUAL (input2_instance->p_v_input.last_parameter, &input_parameter);

  binding.unbind (output, input1, binder_handle);

  BOOST_CHECK (!binding.empty ());
  BOOST_CHECK (binding.involves_output (output));
  BOOST_CHECK (!binding.involves_binding (output, input1, binder_handle.aid ()));
  BOOST_CHECK (binding.involves_binding (output, input2, binder_handle.aid ()));
  BOOST_CHECK (!binding.involves_input (input1));
  BOOST_CHECK (binding.involves_input (input2));
  BOOST_CHECK (!binding.involves_input_automaton (input1_handle.aid ()));
  BOOST_CHECK (binding.involves_input_automaton (input2_handle.aid ()));

  binding.unbind (output, input2, binder_handle);

  BOOST_CHECK (binding.empty ());
  BOOST_CHECK (!binding.involves_output (output));
  BOOST_CHECK (!binding.involves_binding (output, input1, binder_handle.aid ()));
  BOOST_CHECK (!binding.involves_binding (output, input2, binder_handle.aid ()));
  BOOST_CHECK (!binding.involves_input (input1));
  BOOST_CHECK (!binding.involves_input (input2));
  BOOST_CHECK (!binding.involves_input_automaton (input1_handle.aid ()));
  BOOST_CHECK (!binding.involves_input_automaton (input2_handle.aid ()));
}

BOOST_AUTO_TEST_SUITE_END()
