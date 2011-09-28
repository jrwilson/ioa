#include "minunit.h"

#include <ioa/action_executor.hpp>
#include "../lib/null_mutex.hpp"
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
unvalued_unparameterized_input_action ()
{
  std::cout << __func__ << std::endl;
  automaton1 instance;
  ioa::null_mutex mutex;
  ioa::automaton_handle<automaton1> h;
  ioa::action_executor<automaton1, automaton1::uv_up_input_action> action (instance, mutex, h, &automaton1::uv_up_input);
  action.set_auto_parameter (314);
  mu_assert (action.get_aid () == h);
  mu_assert (&instance.uv_up_input == action.get_member_ptr ());
  mu_assert (0U == action.get_pid ());
  mu_assert (action == action);

  test_scheduler tss;
  action (tss);
  mu_assert (instance.uv_up_input.state);

  std::auto_ptr<ioa::action_executor_interface> clone (action.clone ());
  mu_assert (action == *clone);

  return 0;
}

static const char*
unvalued_parameterized_input_action ()
{
  std::cout << __func__ << std::endl;
  automaton1 instance;
  ioa::null_mutex mutex;
  ioa::automaton_handle<automaton1> h;
  int parameter = 345;
  ioa::action_executor<automaton1, automaton1::uv_p_input_action> action (instance, mutex, h, &automaton1::uv_p_input, parameter);
  action.set_auto_parameter (314);
  mu_assert (action.get_aid () == h);
  mu_assert (&instance.uv_p_input == action.get_member_ptr ());
  mu_assert (reinterpret_cast<void*> (parameter) == action.get_pid ());
  mu_assert (action == action);

  test_scheduler tss;
  action (tss);
  mu_assert (instance.uv_p_input.state);
  mu_assert (parameter == instance.uv_p_input.last_parameter);

  std::auto_ptr<ioa::action_executor_interface> clone (action.clone ());
  mu_assert (action == *clone);

  return 0;
}

static const char*
unvalued_auto_parameterized_input_action ()
{
  std::cout << __func__ << std::endl;
  automaton1 instance;
  ioa::null_mutex mutex;
  ioa::automaton_handle<automaton1> h;
  ioa::aid_t parameter = 37;
  ioa::action_executor<automaton1, automaton1::uv_ap_input_action> action (instance, mutex, h, &automaton1::uv_ap_input);
  mu_assert (action.get_aid () == h);
  mu_assert (&instance.uv_ap_input == action.get_member_ptr ());
  mu_assert (reinterpret_cast<void*> (-1) == action.get_pid ());
  action.set_auto_parameter (parameter);
  mu_assert (reinterpret_cast<void*> (parameter) == action.get_pid ());
  mu_assert (action == action);

  test_scheduler tss;
  action (tss);
  mu_assert (instance.uv_ap_input.state);
  mu_assert (parameter == instance.uv_ap_input.last_parameter);

  std::auto_ptr<ioa::action_executor_interface> clone (action.clone ());
  mu_assert (action == *clone);

  return 0;
}

static const char*
valued_unparameterized_input_action ()
{
  std::cout << __func__ << std::endl;
  automaton1 instance;
  ioa::null_mutex mutex;
  ioa::automaton_handle<automaton1> h;
  ioa::action_executor<automaton1, automaton1::v_up_input_action> action (instance, mutex, h, &automaton1::v_up_input);
  action.set_auto_parameter (314);
  mu_assert (action.get_aid () == h);
  mu_assert (&instance.v_up_input == action.get_member_ptr ());
  mu_assert (0U == action.get_pid ());
  mu_assert (action == action);

  test_scheduler tss;
  action (tss, 9845);
  mu_assert (9845 == instance.v_up_input.value);

  std::auto_ptr<ioa::action_executor_interface> clone (action.clone ());
  mu_assert (action == *clone);

  return 0;
}

static const char*
valued_parameterized_input_action ()
{
  std::cout << __func__ << std::endl;
  automaton1 instance;
  ioa::null_mutex mutex;
  ioa::automaton_handle<automaton1> h;
  int parameter = 345;
  ioa::action_executor<automaton1, automaton1::v_p_input_action> action (instance, mutex, h, &automaton1::v_p_input, parameter);
  action.set_auto_parameter (314);
  mu_assert (action.get_aid () == h);
  mu_assert (&instance.v_p_input == action.get_member_ptr ());
  mu_assert (reinterpret_cast<void*> (parameter) == action.get_pid ());
  mu_assert (action == action);

  test_scheduler tss;
  action (tss, 9845);
  mu_assert (9845 == instance.v_p_input.value);
  mu_assert (parameter == instance.v_p_input.last_parameter);

  std::auto_ptr<ioa::action_executor_interface> clone (action.clone ());
  mu_assert (action == *clone);

  return 0;
}

static const char*
valued_auto_parameterized_input_action ()
{
  std::cout << __func__ << std::endl;
  automaton1 instance;
  ioa::null_mutex mutex;
  ioa::automaton_handle<automaton1> h;
  ioa::aid_t parameter = 345;
  ioa::action_executor<automaton1, automaton1::v_ap_input_action> action (instance, mutex, h, &automaton1::v_ap_input);
  mu_assert (action.get_aid () == h);
  mu_assert (&instance.v_ap_input == action.get_member_ptr ());
  mu_assert (reinterpret_cast<void*> (-1) == action.get_pid ());
  action.set_auto_parameter (parameter);
  mu_assert (reinterpret_cast<void*> (parameter) == action.get_pid ());
  mu_assert (action == action);

  test_scheduler tss;
  action (tss, 9845);
  mu_assert (9845 == instance.v_ap_input.value);
  mu_assert (parameter == instance.v_ap_input.last_parameter);

  std::auto_ptr<ioa::action_executor_interface> clone (action.clone ());
  mu_assert (action == *clone);

  return 0;
}

static const char*
unvalued_unparameterized_output_action ()
{
  std::cout << __func__ << std::endl;

  test_scheduler tss;

  automaton1 instance;
  ioa::null_mutex mutex;
  ioa::automaton_handle<automaton1> h (1);
  ioa::action_executor<automaton1, automaton1::uv_up_output_action> action (instance, mutex, h, &automaton1::uv_up_output);
  action.set_auto_parameter (314);
  mu_assert (action.get_aid () == h);
  mu_assert (&instance.uv_up_output == action.get_member_ptr ());
  mu_assert (0U == action.get_pid ());
  mu_assert (action == action);

  std::vector<ioa::unvalued_input_executor_interface*> actions;

  action (tss, actions.begin (), actions.end ());
  mu_assert (instance.uv_up_output.state);

  automaton1 instance1;
  ioa::null_mutex mutex1;
  ioa::automaton_handle<automaton1> input1_handle (2);
  automaton1 instance2;
  ioa::null_mutex mutex2;
  ioa::automaton_handle<automaton1> input2_handle (3);
  automaton1 instance3;
  ioa::null_mutex mutex3;
  ioa::automaton_handle<automaton1> input3_handle (4);

  ioa::action_executor<automaton1, automaton1::uv_up_input_action> input1 (instance1, mutex1, input1_handle, &automaton1::uv_up_input);
  ioa::action_executor<automaton1, automaton1::uv_p_input_action> input2 (instance2, mutex2, input2_handle, &automaton1::uv_p_input, 890);
  ioa::action_executor<automaton1, automaton1::uv_ap_input_action> input3 (instance3, mutex3, input3_handle, &automaton1::uv_ap_input);
  input3.set_auto_parameter (action.get_aid ());

  actions.push_back (&input1);
  actions.push_back (&input2);
  actions.push_back (&input3);

  action (tss, actions.begin (), actions.end ());
  mu_assert (instance.uv_up_output.state);
  mu_assert (instance1.uv_up_input.state);
  mu_assert (instance2.uv_p_input.state);
  mu_assert (instance3.uv_ap_input.state);

  std::auto_ptr<ioa::output_executor_interface> clone (action.clone ());
  mu_assert (action == *clone);

  return 0;
}

static const char*
unvalued_parameterized_output_action ()
{
  std::cout << __func__ << std::endl;

  test_scheduler tss;

  automaton1 instance;
  ioa::null_mutex mutex;
  ioa::automaton_handle<automaton1> h (1);
  int parameter = 345;
  ioa::action_executor<automaton1, automaton1::uv_p_output_action> action (instance, mutex, h, &automaton1::uv_p_output, parameter);
  action.set_auto_parameter (314);
  mu_assert (action.get_aid () == h);
  mu_assert (&instance.uv_p_output == action.get_member_ptr ());
  mu_assert (reinterpret_cast<void*> (parameter) == action.get_pid ());
  mu_assert (action == action);

  std::vector<ioa::unvalued_input_executor_interface*> actions;

  action (tss, actions.begin (), actions.end ());
  mu_assert (instance.uv_p_output.state);
  mu_assert (instance.uv_p_output.last_parameter == parameter);

  automaton1 instance1;
  ioa::null_mutex mutex1;
  ioa::automaton_handle<automaton1> input1_handle (2);
  automaton1 instance2;
  ioa::null_mutex mutex2;
  ioa::automaton_handle<automaton1> input2_handle (3);
  automaton1 instance3;
  ioa::null_mutex mutex3;
  ioa::automaton_handle<automaton1> input3_handle (4);

  ioa::action_executor<automaton1, automaton1::uv_up_input_action> input1 (instance1, mutex1, input1_handle, &automaton1::uv_up_input);
  ioa::action_executor<automaton1, automaton1::uv_p_input_action> input2 (instance2, mutex2, input2_handle, &automaton1::uv_p_input, 890);
  ioa::action_executor<automaton1, automaton1::uv_ap_input_action> input3 (instance3, mutex3, input3_handle, &automaton1::uv_ap_input);
  input3.set_auto_parameter (action.get_aid ());

  actions.push_back (&input1);
  actions.push_back (&input2);
  actions.push_back (&input3);

  action (tss, actions.begin (), actions.end ());
  mu_assert (instance.uv_p_output.state);
  mu_assert (parameter == instance.uv_p_output.last_parameter);
  mu_assert (instance1.uv_up_input.state);
  mu_assert (instance2.uv_p_input.state);
  mu_assert (instance3.uv_ap_input.state);

  std::auto_ptr<ioa::output_executor_interface> clone (action.clone ());
  mu_assert (action == *clone);

  return 0;
}

static const char*
unvalued_auto_parameterized_output_action ()
{
  std::cout << __func__ << std::endl;

  test_scheduler tss;

  automaton1 instance;
  ioa::null_mutex mutex;
  ioa::automaton_handle<automaton1> h (1);
  ioa::action_executor<automaton1, automaton1::uv_ap_output_action> action (instance, mutex, h, &automaton1::uv_ap_output);
  action.set_auto_parameter (2);
  mu_assert (action.get_aid () == h);
  mu_assert (&instance.uv_ap_output == action.get_member_ptr ());
  mu_assert (reinterpret_cast<void*> (2) == action.get_pid ());
  mu_assert (action == action);

  std::vector<ioa::unvalued_input_executor_interface*> actions;

  action (tss, actions.begin (), actions.end ());
  mu_assert (instance.uv_ap_output.state);
  mu_assert (instance.uv_ap_output.last_parameter == 2);

  automaton1 instance1;
  ioa::null_mutex mutex1;
  ioa::automaton_handle<automaton1> input1_handle (2);

  ioa::action_executor<automaton1, automaton1::uv_up_input_action> input1 (instance1, mutex1, input1_handle, &automaton1::uv_up_input);

  actions.push_back (&input1);

  action (tss, actions.begin (), actions.end ());
  mu_assert (instance.uv_ap_output.state);
  mu_assert (instance.uv_ap_output.last_parameter == 2);
  mu_assert (instance1.uv_up_input.state);

  return 0;
}

static const char*
valued_unparameterized_output_action ()
{
  std::cout << __func__ << std::endl;

  test_scheduler tss;

  automaton1 instance;
  ioa::null_mutex mutex;
  ioa::automaton_handle<automaton1> h (1);
  ioa::action_executor<automaton1, automaton1::v_up_output_action> action (instance, mutex, h, &automaton1::v_up_output);
  action.set_auto_parameter (314);
  mu_assert (action.get_aid () == h);
  mu_assert (&instance.v_up_output == action.get_member_ptr ());
  mu_assert (0U == action.get_pid ());
  mu_assert (action == action);

  std::vector<ioa::valued_input_executor_interface<int>*> actions;

  action (tss, actions.begin (), actions.end ());
  mu_assert (instance.v_up_output.state);

  automaton1 instance1;
  ioa::null_mutex mutex1;
  ioa::automaton_handle<automaton1> input1_handle (2);
  automaton1 instance2;
  ioa::null_mutex mutex2;
  ioa::automaton_handle<automaton1> input2_handle (3);
  automaton1 instance3;
  ioa::null_mutex mutex3;
  ioa::automaton_handle<automaton1> input3_handle (4);

  ioa::action_executor<automaton1, automaton1::v_up_input_action> input1 (instance1, mutex1, input1_handle, &automaton1::v_up_input);
  ioa::action_executor<automaton1, automaton1::v_p_input_action> input2 (instance2, mutex2, input2_handle, &automaton1::v_p_input, 890);
  ioa::action_executor<automaton1, automaton1::v_ap_input_action> input3 (instance3, mutex3, input3_handle, &automaton1::v_ap_input);
  input3.set_auto_parameter (action.get_aid ());

  actions.push_back (&input1);
  actions.push_back (&input2);
  actions.push_back (&input3);

  action (tss, actions.begin (), actions.end ());
  mu_assert (instance.v_up_output.state);
  mu_assert (instance1.v_up_input.value == 9845);
  mu_assert (instance2.v_p_input.value == 9845);
  mu_assert (instance3.v_ap_input.value == 9845);

  std::auto_ptr<ioa::output_executor_interface> clone (action.clone ());
  mu_assert (action == *clone);

  return 0;
}

static const char*
valued_parameterized_output_action ()
{
  std::cout << __func__ << std::endl;

  test_scheduler tss;

  automaton1 instance;
  ioa::null_mutex mutex;
  ioa::automaton_handle<automaton1> h (1);
  int parameter = 345;
  ioa::action_executor<automaton1, automaton1::v_p_output_action> action (instance, mutex, h, &automaton1::v_p_output, parameter);
  action.set_auto_parameter (314);
  mu_assert (action.get_aid () == h);
  mu_assert (&instance.v_p_output == action.get_member_ptr ());
  mu_assert (reinterpret_cast<void*> (parameter) == action.get_pid ());
  mu_assert (action == action);

  std::vector<ioa::valued_input_executor_interface<int>*> actions;

  action (tss, actions.begin (), actions.end ());
  mu_assert (instance.v_p_output.state);
  mu_assert (parameter == instance.v_p_output.last_parameter);

  automaton1 instance1;
  ioa::null_mutex mutex1;
  ioa::automaton_handle<automaton1> input1_handle (2);
  automaton1 instance2;
  ioa::null_mutex mutex2;
  ioa::automaton_handle<automaton1> input2_handle (3);
  automaton1 instance3;
  ioa::null_mutex mutex3;
  ioa::automaton_handle<automaton1> input3_handle (4);


  ioa::action_executor<automaton1, automaton1::v_up_input_action> input1 (instance1, mutex1, input1_handle, &automaton1::v_up_input);
  ioa::action_executor<automaton1, automaton1::v_p_input_action> input2 (instance2, mutex2, input2_handle, &automaton1::v_p_input, 890);
  ioa::action_executor<automaton1, automaton1::v_ap_input_action> input3 (instance3, mutex3, input3_handle, &automaton1::v_ap_input);
  input3.set_auto_parameter (action.get_aid ());

  actions.push_back (&input1);
  actions.push_back (&input2);
  actions.push_back (&input3);

  action (tss, actions.begin (), actions.end ());
  mu_assert (instance.v_p_output.state);
  mu_assert (parameter == instance.v_p_output.last_parameter);
  mu_assert (instance1.v_up_input.value == 9845);
  mu_assert (instance2.v_p_input.value == 9845);
  mu_assert (instance3.v_ap_input.value == 9845);

  std::auto_ptr<ioa::output_executor_interface> clone (action.clone ());
  mu_assert (action == *clone);

  return 0;
}

static const char*
valued_auto_parameterized_output_action ()
{
  std::cout << __func__ << std::endl;

  test_scheduler tss;

  automaton1 instance;
  ioa::null_mutex mutex;
  ioa::automaton_handle<automaton1> h (1);
  ioa::action_executor<automaton1, automaton1::v_ap_output_action> action (instance, mutex, h, &automaton1::v_ap_output);
  action.set_auto_parameter (2);
  mu_assert (action.get_aid () == h);
  mu_assert (&instance.v_ap_output == action.get_member_ptr ());
  mu_assert (reinterpret_cast<void*> (2) == action.get_pid ());
  mu_assert (action == action);

  std::vector<ioa::valued_input_executor_interface<int>*> actions;

  action (tss, actions.begin (), actions.end ());
  mu_assert (instance.v_ap_output.state);
  mu_assert (instance.v_ap_output.last_parameter == 2);

  automaton1 instance1;
  ioa::null_mutex mutex1;
  ioa::automaton_handle<automaton1> input1_handle (2);

  ioa::action_executor<automaton1, automaton1::v_up_input_action> input1 (instance1, mutex1, input1_handle, &automaton1::v_up_input);

  actions.push_back (&input1);

  action (tss, actions.begin (), actions.end ());
  mu_assert (instance.v_ap_output.state);
  mu_assert (instance.v_ap_output.last_parameter == input1_handle);
  mu_assert (instance1.v_up_input.value == 9845);

  return 0;
}

static const char*
unparameterized_internal_action ()
{
  std::cout << __func__ << std::endl;

  automaton1 instance;
  ioa::null_mutex mutex;
  ioa::automaton_handle<automaton1> h;
  ioa::action_executor<automaton1, automaton1::up_internal_action> action (instance, mutex, h, &automaton1::up_internal);
  mu_assert (action.get_aid () == h);
  mu_assert (&instance.up_internal == action.get_member_ptr ());
  mu_assert (0U == action.get_pid ());
  mu_assert (action == action);

  test_scheduler tss;
  action (tss);
  mu_assert (instance.up_internal.state);

  return 0;
}

static const char*
parameterized_internal_action ()
{
  std::cout << __func__ << std::endl;

  automaton1 instance;
  ioa::null_mutex mutex;
  ioa::automaton_handle<automaton1> h;
  int parameter = 345;
  ioa::action_executor<automaton1, automaton1::p_internal_action> action (instance, mutex, h, &automaton1::p_internal, parameter);
  mu_assert (action.get_aid () == h);
  mu_assert (&instance.p_internal == action.get_member_ptr ());
  mu_assert (reinterpret_cast<void*> (parameter) == action.get_pid ());
  mu_assert (action == action);

  test_scheduler tss;
  action (tss);
  mu_assert (instance.p_internal.state);
  mu_assert (parameter == instance.p_internal.last_parameter);

  return 0;
}

const char*
all_tests ()
{
  mu_run_test (unvalued_unparameterized_input_action);
  mu_run_test (unvalued_parameterized_input_action);
  mu_run_test (unvalued_auto_parameterized_input_action);
  mu_run_test (valued_unparameterized_input_action);
  mu_run_test (valued_parameterized_input_action);
  mu_run_test (valued_auto_parameterized_input_action);
  mu_run_test (unvalued_unparameterized_output_action);
  mu_run_test (unvalued_parameterized_output_action);
  mu_run_test (unvalued_auto_parameterized_output_action);
  mu_run_test (valued_unparameterized_output_action);
  mu_run_test (valued_parameterized_output_action);
  mu_run_test (valued_auto_parameterized_output_action);
  mu_run_test (unparameterized_internal_action);
  mu_run_test (parameterized_internal_action);

  return 0;
}
