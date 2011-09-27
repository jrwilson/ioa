#include "minunit.h"

#include <ioa/action_runnable.hpp>
#include <ioa/shared_lock.hpp>
#include <ioa/unique_lock.hpp>
#include "automaton1.hpp"

#include <iostream>
#include <vector>

class test_scheduler :
  public ioa::scheduler_interface
{
public:
  void set_current_aid (const ioa::aid_t aid) { }
  void clear_current_aid () { }
};

static const char*
unvalued_unparameterized_output_action ()
{
  std::cout << __func__ << std::endl;

  test_scheduler tss;
  ioa::shared_mutex shared_mutex;
  ioa::automaton_set automaton_set;
  ioa::binding_set binding_set;

  automaton1 instance;
  ioa::mutex mutex;
  ioa::automaton_handle<automaton1> h (automaton_set.create (&instance));

  automaton1 instance1;
  ioa::mutex mutex1;
  ioa::automaton_handle<automaton1> input1_handle (automaton_set.create (&instance1));

  automaton1 instance2;
  ioa::mutex mutex2;
  ioa::automaton_handle<automaton1> input2_handle (automaton_set.create (&instance2));

  automaton1 instance3;
  ioa::mutex mutex3;
  ioa::automaton_handle<automaton1> input3_handle (automaton_set.create (&instance3));

  ioa::action_executor<automaton1, automaton1::uv_up_output_action> executor (instance, mutex, h, &automaton1::uv_up_output);
  ioa::action_executor<automaton1, automaton1::uv_up_input_action> input_action1 (instance1, mutex1, input1_handle, &automaton1::uv_up_input);
  ioa::action_executor<automaton1, automaton1::uv_p_input_action> input_action2 (instance2, mutex2, input2_handle, &automaton1::uv_p_input, 890);
  ioa::action_executor<automaton1, automaton1::uv_ap_input_action> input_action3 (instance3, mutex3, input3_handle, &automaton1::uv_ap_input);
  input_action3.set_auto_parameter (executor.get_aid ());

  ioa::action_runnable<automaton1, automaton1::uv_up_output_action, ioa::shared_lock> runnable (tss, shared_mutex, automaton_set, binding_set, instance, mutex, h, &automaton1::uv_up_output);
  mu_assert (executor == runnable.get_action ());

  runnable ();
  mu_assert (instance.uv_up_output.state);

  int key1;
  int key2;
  int key3;

  binding_set.bind (executor.get_aid (), &key3, executor, input_action3);
  binding_set.bind (executor.get_aid (), &key1, executor, input_action1);
  binding_set.bind (executor.get_aid (), &key2, executor, input_action2);

  runnable ();
  mu_assert (instance.uv_up_output.state);
  mu_assert (instance1.uv_up_input.state);
  mu_assert (instance2.uv_p_input.state);
  mu_assert (instance3.uv_ap_input.state);

  return 0;
}

static const char*
unvalued_parameterized_output_action ()
{
  std::cout << __func__ << std::endl;

  test_scheduler tss;
  ioa::shared_mutex shared_mutex;
  ioa::automaton_set automaton_set;
  ioa::binding_set binding_set;

  automaton1 instance;
  ioa::mutex mutex;
  ioa::automaton_handle<automaton1> h (automaton_set.create (&instance));
  int parameter = 345;

  automaton1 instance1;
  ioa::mutex mutex1;
  ioa::automaton_handle<automaton1> input1_handle (automaton_set.create (&instance1));

  automaton1 instance2;
  ioa::mutex mutex2;
  ioa::automaton_handle<automaton1> input2_handle (automaton_set.create (&instance2));

  automaton1 instance3;
  ioa::mutex mutex3;
  ioa::automaton_handle<automaton1> input3_handle (automaton_set.create (&instance3));

  ioa::action_executor<automaton1, automaton1::uv_p_output_action> executor (instance, mutex, h, &automaton1::uv_p_output, parameter);
  ioa::action_executor<automaton1, automaton1::uv_up_input_action> input_action1 (instance1, mutex1, input1_handle, &automaton1::uv_up_input);
  ioa::action_executor<automaton1, automaton1::uv_p_input_action> input_action2 (instance2, mutex2, input2_handle, &automaton1::uv_p_input, 890);
  ioa::action_executor<automaton1, automaton1::uv_ap_input_action> input_action3 (instance3, mutex3, input3_handle, &automaton1::uv_ap_input);
  input_action3.set_auto_parameter (executor.get_aid ());

  ioa::action_runnable<automaton1, automaton1::uv_p_output_action, ioa::shared_lock> runnable (tss, shared_mutex, automaton_set, binding_set, instance, mutex, h, &automaton1::uv_p_output, parameter);
  mu_assert (executor == runnable.get_action ());
  
  runnable ();
  mu_assert (instance.uv_p_output.state);
  mu_assert (instance.uv_p_output.last_parameter == parameter);

  int key1;
  int key2;
  int key3;

  binding_set.bind (executor.get_aid (), &key3, executor, input_action3);
  binding_set.bind (executor.get_aid (), &key1, executor, input_action1);
  binding_set.bind (executor.get_aid (), &key2, executor, input_action2);

  runnable ();
  mu_assert (instance.uv_p_output.state);
  mu_assert (parameter == instance.uv_p_output.last_parameter);
  mu_assert (instance1.uv_up_input.state);
  mu_assert (instance2.uv_p_input.state);
  mu_assert (instance3.uv_ap_input.state);

  return 0;
}

static const char*
unvalued_auto_parameterized_output_action ()
{
  std::cout << __func__ << std::endl;

  test_scheduler tss;
  ioa::shared_mutex shared_mutex;
  ioa::automaton_set automaton_set;
  ioa::binding_set binding_set;

  automaton1 instance;
  ioa::mutex mutex;
  ioa::automaton_handle<automaton1> h (automaton_set.create (&instance));

  automaton1 instance1;
  ioa::mutex mutex1;
  ioa::automaton_handle<automaton1> input1_handle (automaton_set.create (&instance1));

  ioa::action_executor<automaton1, automaton1::uv_ap_output_action> executor (instance, mutex, h, &automaton1::uv_ap_output);
  executor.set_auto_parameter (input1_handle);
  ioa::action_executor<automaton1, automaton1::uv_up_input_action> input_action1 (instance1, mutex1, input1_handle, &automaton1::uv_up_input);

  ioa::action_runnable<automaton1, automaton1::uv_ap_output_action, ioa::shared_lock> runnable (tss, shared_mutex, automaton_set, binding_set, instance, mutex, h, &automaton1::uv_ap_output, input_action1.get_aid ());
  mu_assert (executor == runnable.get_action ());

  runnable ();
  mu_assert (instance.uv_ap_output.state);
  mu_assert (instance.uv_ap_output.last_parameter == input1_handle);

  int key1;

  binding_set.bind (executor.get_aid (), &key1, executor, input_action1);

  runnable ();
  mu_assert (instance.uv_ap_output.state);
  mu_assert (instance.uv_ap_output.last_parameter == input1_handle);
  mu_assert (instance1.uv_up_input.state);

  return 0;
}

static const char*
valued_unparameterized_output_action ()
{
  std::cout << __func__ << std::endl;

  test_scheduler tss;
  ioa::shared_mutex shared_mutex;
  ioa::automaton_set automaton_set;
  ioa::binding_set binding_set;

  automaton1 instance;
  ioa::mutex mutex;
  ioa::automaton_handle<automaton1> h (automaton_set.create (&instance));

  automaton1 instance1;
  ioa::mutex mutex1;
  ioa::automaton_handle<automaton1> input1_handle (automaton_set.create (&instance1));

  automaton1 instance2;
  ioa::mutex mutex2;
  ioa::automaton_handle<automaton1> input2_handle (automaton_set.create (&instance2));

  automaton1 instance3;
  ioa::mutex mutex3;
  ioa::automaton_handle<automaton1> input3_handle (automaton_set.create (&instance3));

  ioa::action_executor<automaton1, automaton1::v_up_output_action> executor (instance, mutex, h, &automaton1::v_up_output);
  ioa::action_executor<automaton1, automaton1::v_up_input_action> input_action1 (instance1, mutex1, input1_handle, &automaton1::v_up_input);
  ioa::action_executor<automaton1, automaton1::v_p_input_action> input_action2 (instance2, mutex2, input2_handle, &automaton1::v_p_input, 890);
  ioa::action_executor<automaton1, automaton1::v_ap_input_action> input_action3 (instance3, mutex3, input3_handle, &automaton1::v_ap_input);
  input_action3.set_auto_parameter (executor.get_aid ());

  ioa::action_runnable<automaton1, automaton1::v_up_output_action, ioa::shared_lock> runnable (tss, shared_mutex, automaton_set, binding_set, instance, mutex, h, &automaton1::v_up_output);
  mu_assert (executor == runnable.get_action ());

  runnable ();
  mu_assert (instance.v_up_output.state);

  int key1;
  int key2;
  int key3;

  binding_set.bind (executor.get_aid (), &key3, executor, input_action3);
  binding_set.bind (executor.get_aid (), &key1, executor, input_action1);
  binding_set.bind (executor.get_aid (), &key2, executor, input_action2);

  runnable ();
  mu_assert (instance.v_up_output.state);
  mu_assert (instance1.v_up_input.value == 9845);
  mu_assert (instance2.v_p_input.value == 9845);
  mu_assert (instance3.v_ap_input.value == 9845);

  return 0;
}

static const char*
valued_parameterized_output_action ()
{
  std::cout << __func__ << std::endl;

  test_scheduler tss;
  ioa::shared_mutex shared_mutex;
  ioa::automaton_set automaton_set;
  ioa::binding_set binding_set;

  automaton1 instance;
  ioa::mutex mutex;
  ioa::automaton_handle<automaton1> h (automaton_set.create (&instance));
  int parameter = 345;

  automaton1 instance1;
  ioa::mutex mutex1;
  ioa::automaton_handle<automaton1> input1_handle (automaton_set.create (&instance1));

  automaton1 instance2;
  ioa::mutex mutex2;
  ioa::automaton_handle<automaton1> input2_handle (automaton_set.create (&instance2));

  automaton1 instance3;
  ioa::mutex mutex3;
  ioa::automaton_handle<automaton1> input3_handle (automaton_set.create (&instance3));

  ioa::action_executor<automaton1, automaton1::v_p_output_action> executor (instance, mutex, h, &automaton1::v_p_output, parameter);
  ioa::action_executor<automaton1, automaton1::v_up_input_action> input_action1 (instance1, mutex1, input1_handle, &automaton1::v_up_input);
  ioa::action_executor<automaton1, automaton1::v_p_input_action> input_action2 (instance2, mutex2, input2_handle, &automaton1::v_p_input, 890);
  ioa::action_executor<automaton1, automaton1::v_ap_input_action> input_action3 (instance3, mutex3, input3_handle, &automaton1::v_ap_input);
  input_action3.set_auto_parameter (executor.get_aid ());

  ioa::action_runnable<automaton1, automaton1::v_p_output_action, ioa::shared_lock> runnable (tss, shared_mutex, automaton_set, binding_set, instance, mutex, h, &automaton1::v_p_output, parameter);
  mu_assert (executor == runnable.get_action ());

  runnable ();
  mu_assert (instance.v_p_output.state);
  mu_assert (parameter == instance.v_p_output.last_parameter);

  int key1;
  int key2;
  int key3;

  binding_set.bind (executor.get_aid (), &key3, executor, input_action3);
  binding_set.bind (executor.get_aid (), &key1, executor, input_action1);
  binding_set.bind (executor.get_aid (), &key2, executor, input_action2);

  runnable ();
  mu_assert (instance.v_p_output.state);
  mu_assert (parameter == instance.v_p_output.last_parameter);
  mu_assert (instance1.v_up_input.value == 9845);
  mu_assert (instance2.v_p_input.value == 9845);
  mu_assert (instance3.v_ap_input.value == 9845);

  return 0;
}

static const char*
valued_auto_parameterized_output_action ()
{
  std::cout << __func__ << std::endl;

  test_scheduler tss;
  ioa::shared_mutex shared_mutex;
  ioa::automaton_set automaton_set;
  ioa::binding_set binding_set;

  automaton1 instance;
  ioa::mutex mutex;
  ioa::automaton_handle<automaton1> h (automaton_set.create (&instance));

  automaton1 instance1;
  ioa::mutex mutex1;
  ioa::automaton_handle<automaton1> input1_handle (automaton_set.create (&instance1));

  ioa::action_executor<automaton1, automaton1::v_ap_output_action> executor (instance, mutex, h, &automaton1::v_ap_output);
  executor.set_auto_parameter (input1_handle);
  ioa::action_executor<automaton1, automaton1::v_up_input_action> input_action1 (instance1, mutex1, input1_handle, &automaton1::v_up_input);

  ioa::action_runnable<automaton1, automaton1::v_ap_output_action, ioa::shared_lock> runnable (tss, shared_mutex, automaton_set, binding_set, instance, mutex, h, &automaton1::v_ap_output, input_action1.get_aid ());
  mu_assert (executor == runnable.get_action ());

  runnable ();
  mu_assert (instance.v_ap_output.state);
  mu_assert (instance.v_ap_output.last_parameter == input1_handle);

  int key1;

  binding_set.bind (executor.get_aid (), &key1, executor, input_action1);

  runnable ();
  mu_assert (instance.v_ap_output.state);
  mu_assert (instance.v_ap_output.last_parameter == input1_handle);
  mu_assert (instance1.v_up_input.value == 9845);

  return 0;
}

static const char*
unparameterized_internal_action ()
{
  std::cout << __func__ << std::endl;

  test_scheduler tss;
  ioa::shared_mutex shared_mutex;
  ioa::automaton_set automaton_set;

  automaton1 instance;
  ioa::mutex mutex;
  ioa::automaton_handle<automaton1> h (automaton_set.create (&instance));

  ioa::action_executor<automaton1, automaton1::up_internal_action> executor (instance, mutex, h, &automaton1::up_internal);

  ioa::action_runnable<automaton1, automaton1::up_internal_action, ioa::shared_lock> runnable (tss, shared_mutex, automaton_set, instance, mutex, h, &automaton1::up_internal);
  mu_assert (executor == runnable.get_action ());

  runnable ();
  mu_assert (instance.up_internal.state);

  return 0;
}

static const char*
unparameterized_internal_action_unique ()
{
  std::cout << __func__ << std::endl;

  test_scheduler tss;
  ioa::shared_mutex shared_mutex;
  ioa::automaton_set automaton_set;

  automaton1 instance;
  ioa::mutex mutex;
  ioa::automaton_handle<automaton1> h (automaton_set.create (&instance));

  ioa::action_executor<automaton1, automaton1::up_internal_action> executor (instance, mutex, h, &automaton1::up_internal);

  ioa::action_runnable<automaton1, automaton1::up_internal_action, ioa::unique_lock> runnable (tss, shared_mutex, automaton_set, instance, mutex, h, &automaton1::up_internal);
  mu_assert (executor == runnable.get_action ());

  runnable ();
  mu_assert (instance.up_internal.state);

  return 0;
}

static const char*
parameterized_internal_action ()
{
  std::cout << __func__ << std::endl;

  test_scheduler tss;
  ioa::shared_mutex shared_mutex;
  ioa::automaton_set automaton_set;

  automaton1 instance;
  ioa::mutex mutex;
  ioa::automaton_handle<automaton1> h (automaton_set.create (&instance));
  int parameter = 345;

  ioa::action_executor<automaton1, automaton1::p_internal_action> executor (instance, mutex, h, &automaton1::p_internal, parameter);

  ioa::action_runnable<automaton1, automaton1::p_internal_action, ioa::shared_lock> runnable (tss, shared_mutex, automaton_set, instance, mutex, h, &automaton1::p_internal, parameter);
  mu_assert (executor == runnable.get_action ());

  runnable ();
  mu_assert (instance.p_internal.state);
  mu_assert (parameter == instance.p_internal.last_parameter);

  return 0;
}

static const char*
parameterized_internal_action_unique ()
{
  std::cout << __func__ << std::endl;

  test_scheduler tss;
  ioa::shared_mutex shared_mutex;
  ioa::automaton_set automaton_set;

  automaton1 instance;
  ioa::mutex mutex;
  ioa::automaton_handle<automaton1> h (automaton_set.create (&instance));
  int parameter = 345;

  ioa::action_executor<automaton1, automaton1::p_internal_action> executor (instance, mutex, h, &automaton1::p_internal, parameter);

  ioa::action_runnable<automaton1, automaton1::p_internal_action, ioa::unique_lock> runnable (tss, shared_mutex, automaton_set, instance, mutex, h, &automaton1::p_internal, parameter);
  mu_assert (executor == runnable.get_action ());

  runnable ();
  mu_assert (instance.p_internal.state);
  mu_assert (parameter == instance.p_internal.last_parameter);

  return 0;
}

const char*
all_tests ()
{
  mu_run_test (unvalued_unparameterized_output_action);
  mu_run_test (unvalued_parameterized_output_action);
  mu_run_test (unvalued_auto_parameterized_output_action);
  mu_run_test (valued_unparameterized_output_action);
  mu_run_test (valued_parameterized_output_action);
  mu_run_test (valued_auto_parameterized_output_action);
  mu_run_test (unparameterized_internal_action);
  mu_run_test (unparameterized_internal_action_unique);
  mu_run_test (parameterized_internal_action);
  mu_run_test (parameterized_internal_action_unique);

  return 0;
}
