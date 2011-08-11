#include "minunit.h"

#include <ioa/action_executor.hpp>
#include "automaton1.hpp"
#include <iostream>

struct test_model :
  public ioa::model_interface
{
  automaton1 instance;

  void add_bind_key (const ioa::aid_t aid,
		     void* const key) { }
  void remove_bind_key (const ioa::aid_t aid,
			void* const key) { }

  // Executing user actions.
  ioa::automaton* get_instance (const ioa::aid_t aid) { return &instance; }
  void lock_automaton (const ioa::aid_t aid) { }
  void unlock_automaton (const ioa::aid_t aid) { }
  int execute (ioa::output_executor_interface& exec) { return -1; }
  int execute (ioa::internal_executor_interface& exec) { return -1; }

  // Executing system outputs.
  int execute_sys_create (const ioa::aid_t automaton) { return -1; }
  int execute_sys_bind (const ioa::aid_t automaton) { return -1; }
  int execute_sys_unbind (const ioa::aid_t automaton) { return -1; }
  int execute_sys_destroy (const ioa::aid_t automaton) { return -1; }
  int execute_sys_self_destruct (const ioa::aid_t automaton) { return -1; }

  // Executing configuation actions.
  ioa::aid_t create (const ioa::aid_t automaton,
		     std::auto_ptr<ioa::generator_interface> generator,
		     void* const key) { return -1; }
  int bind (const ioa::aid_t automaton,
	    ioa::shared_ptr<ioa::bind_executor_interface> exec,
	    void* const key) { return -1; }
  int unbind (const ioa::aid_t automaton,
	      void* const key) { return -1; }
  int destroy (const ioa::aid_t automaton,
	       void* const key) { return -1; }
  int destroy (const ioa::aid_t automaton) { return -1; }

  // Executing system inputs.
  int execute (ioa::system_input_executor_interface& exec) { return -1; }
  int execute_output_bound (ioa::output_executor_interface& exec) { return -1; }
  int execute_input_bound (ioa::input_executor_interface& exec) { return -1; }
  int execute_output_unbound (ioa::output_executor_interface& exec) { return -1; }
  int execute_input_unbound (ioa::input_executor_interface& exec) { return -1; }
};

struct test_system_scheduler :
  public ioa::system_scheduler_interface
{
  void set_current_aid (const ioa::aid_t aid) { }
  void clear_current_aid () { }
    
  void create (const ioa::aid_t automaton,
	       std::auto_ptr<ioa::generator_interface> generator,
	       void* const key) { }
    
  void bind (const ioa::aid_t automaton,
	     ioa::shared_ptr<ioa::bind_executor_interface> exec,
	     void* const key) { }
    
  void unbind (const ioa::aid_t automaton,
	       void* const key) { }
    
  void destroy (const ioa::aid_t automaton,
		void* const key) { }

  void self_destruct (const ioa::aid_t automaton) { }

  void created (const ioa::aid_t automaton,
		const ioa::automaton::created_t,
		void* const key,
		const ioa::aid_t) { }
    
  void bound (const ioa::aid_t automaton,
	      const ioa::automaton::bound_t,
	      void* const key) { }
    
  void output_bound (const ioa::output_executor_interface&) { }
    
  void input_bound (const ioa::input_executor_interface&) { }
    
  void unbound (const ioa::aid_t automaton,
		const ioa::automaton::unbound_t,
		void* const key) { }
    
  void output_unbound (const ioa::output_executor_interface&) { }
    
  void input_unbound (const ioa::input_executor_interface&) { }
    
  void destroyed (const ioa::aid_t automaton,
		  const ioa::automaton::destroyed_t,
		  void* const key) { }
};

static const char*
unvalued_unparameterized_input_action ()
{
  std::cout << __func__ << std::endl;
  ioa::automaton_handle<automaton1> h;
  ioa::action_executor<automaton1, automaton1::uv_up_input_action> action (h, &automaton1::uv_up_input);
  mu_assert (action.get_aid () == h);
  automaton1* i = 0;
  mu_assert (&((*i).*(&automaton1::uv_up_input)) == action.get_member_ptr ());
  mu_assert (0U == action.get_pid ());
  mu_assert (action == action);

  test_model tm;
  test_system_scheduler tss;
  mu_assert (action.fetch_instance (tm));
  action (tss);
  mu_assert (tm.instance.uv_up_input.state);
  action.bound (tm, tss);
  mu_assert (tm.instance.uv_up_input.bound_);
  action.unbound (tm, tss);
  mu_assert (tm.instance.uv_up_input.unbound_);

  std::auto_ptr<ioa::action_executor_interface> clone (action.clone ());
  mu_assert (action == *clone);

  return 0;
}

static const char*
unvalued_parameterized_input_action ()
{
  std::cout << __func__ << std::endl;
  int parameter = 345;
  ioa::automaton_handle<automaton1> h;
  ioa::action_executor<automaton1, automaton1::uv_p_input_action> action (h, &automaton1::uv_p_input, parameter);
  mu_assert (action.get_aid () == h);
  automaton1* i = 0;
  mu_assert (&((*i).*(&automaton1::uv_p_input)) == action.get_member_ptr ());
  mu_assert (size_t (parameter) == action.get_pid ());
  mu_assert (action == action);

  test_model tm;
  test_system_scheduler tss;
  mu_assert (action.fetch_instance (tm));
  action (tss);
  mu_assert (tm.instance.uv_p_input.state);
  mu_assert (parameter == tm.instance.uv_p_input.last_parameter);
  action.bound (tm, tss);
  mu_assert (tm.instance.uv_p_input.bound_);
  mu_assert (parameter == tm.instance.uv_p_input.bound_parameter);
  action.unbound (tm, tss);
  mu_assert (tm.instance.uv_p_input.unbound_);
  mu_assert (parameter == tm.instance.uv_p_input.unbound_parameter);

  std::auto_ptr<ioa::action_executor_interface> clone (action.clone ());
  mu_assert (action == *clone);

  return 0;
}

static const char*
unvalued_auto_parameterized_input_action ()
{
  std::cout << __func__ << std::endl;
  ioa::aid_t parameter = 37;
  ioa::automaton_handle<automaton1> h;
  ioa::action_executor<automaton1, automaton1::uv_ap_input_action> action (h, &automaton1::uv_ap_input);
  mu_assert (action.get_aid () == h);
  automaton1* i = 0;
  mu_assert (&((*i).*(&automaton1::uv_ap_input)) == action.get_member_ptr ());
  mu_assert (size_t (-1) == action.get_pid ());
  action.set_parameter (parameter);
  mu_assert (size_t (parameter) == action.get_pid ());
  mu_assert (action == action);

  test_model tm;
  test_system_scheduler tss;
  mu_assert (action.fetch_instance (tm));
  action (tss);
  mu_assert (tm.instance.uv_ap_input.state);
  mu_assert (parameter == tm.instance.uv_ap_input.last_parameter);
  action.bound (tm, tss);
  mu_assert (tm.instance.uv_ap_input.bound_);
  mu_assert (parameter == tm.instance.uv_ap_input.bound_parameter);
  action.unbound (tm, tss);
  mu_assert (tm.instance.uv_ap_input.unbound_);
  mu_assert (parameter == tm.instance.uv_ap_input.unbound_parameter);

  std::auto_ptr<ioa::action_executor_interface> clone (action.clone ());
  mu_assert (action == *clone);

  return 0;
}

static const char*
valued_unparameterized_input_action ()
{
  std::cout << __func__ << std::endl;
  ioa::automaton_handle<automaton1> h;
  ioa::action_executor<automaton1, automaton1::v_up_input_action> action (h, &automaton1::v_up_input);
  mu_assert (action.get_aid () == h);
  automaton1* i = 0;
  mu_assert (&((*i).*(&automaton1::v_up_input)) == action.get_member_ptr ());
  mu_assert (0U == action.get_pid ());
  mu_assert (action == action);

  test_model tm;
  test_system_scheduler tss;
  mu_assert (action.fetch_instance (tm));
  action (tss, 9845);
  mu_assert (9845 == tm.instance.v_up_input.value);
  action.bound (tm, tss);
  mu_assert (tm.instance.v_up_input.bound_);
  action.unbound (tm, tss);
  mu_assert (tm.instance.v_up_input.unbound_);

  std::auto_ptr<ioa::action_executor_interface> clone (action.clone ());
  mu_assert (action == *clone);

  return 0;
}

static const char*
valued_parameterized_input_action ()
{
  std::cout << __func__ << std::endl;
  int parameter = 345;
  ioa::automaton_handle<automaton1> h;
  ioa::action_executor<automaton1, automaton1::v_p_input_action> action (h, &automaton1::v_p_input, parameter);
  mu_assert (action.get_aid () == h);
  automaton1* i = 0;
  mu_assert (&((*i).*(&automaton1::v_p_input)) == action.get_member_ptr ());
  mu_assert (size_t (parameter) == action.get_pid ());
  mu_assert (action == action);

  test_model tm;
  test_system_scheduler tss;
  mu_assert (action.fetch_instance (tm));
  action (tss, 9845);
  mu_assert (9845 == tm.instance.v_p_input.value);
  mu_assert (parameter == tm.instance.v_p_input.last_parameter);
  action.bound (tm, tss);
  mu_assert (tm.instance.v_p_input.bound_);
  mu_assert (parameter == tm.instance.v_p_input.bound_parameter);
  action.unbound (tm, tss);
  mu_assert (tm.instance.v_p_input.unbound_);
  mu_assert (parameter == tm.instance.v_p_input.unbound_parameter);

  std::auto_ptr<ioa::action_executor_interface> clone (action.clone ());
  mu_assert (action == *clone);

  return 0;
}

static const char*
valued_auto_parameterized_input_action ()
{
  std::cout << __func__ << std::endl;
  ioa::aid_t parameter = 345;
  ioa::automaton_handle<automaton1> h;
  ioa::action_executor<automaton1, automaton1::v_ap_input_action> action (h, &automaton1::v_ap_input);
  mu_assert (action.get_aid () == h);
  automaton1* i = 0;
  mu_assert (&((*i).*(&automaton1::v_ap_input)) == action.get_member_ptr ());
  mu_assert (size_t (-1) == action.get_pid ());
  action.set_parameter (parameter);
  mu_assert (size_t (parameter) == action.get_pid ());
  mu_assert (action == action);

  test_model tm;
  test_system_scheduler tss;
  mu_assert (action.fetch_instance (tm));
  action (tss, 9845);
  mu_assert (9845 == tm.instance.v_ap_input.value);
  mu_assert (parameter == tm.instance.v_ap_input.last_parameter);
  action.bound (tm, tss);
  mu_assert (tm.instance.v_ap_input.bound_);
  mu_assert (parameter == tm.instance.v_ap_input.bound_parameter);
  action.unbound (tm, tss);
  mu_assert (tm.instance.v_ap_input.unbound_);
  mu_assert (parameter == tm.instance.v_ap_input.unbound_parameter);

  std::auto_ptr<ioa::action_executor_interface> clone (action.clone ());
  mu_assert (action == *clone);

  return 0;
}

static const char*
unvalued_unparameterized_output_action ()
{
  std::cout << __func__ << std::endl;

  // Must exist whole time.
  test_model tm;
  test_system_scheduler tss;

  ioa::automaton_handle<automaton1> h (1);
  ioa::action_executor<automaton1, automaton1::uv_up_output_action> action (h, &automaton1::uv_up_output);
  mu_assert (action.get_aid () == h);
  automaton1* i = 0;
  mu_assert (&((*i).*(&automaton1::uv_up_output)) == action.get_member_ptr ());
  mu_assert (0U == action.get_pid ());
  mu_assert (action == action);

  mu_assert (action.involves_output (action));

  ioa::automaton_handle<automaton1> input1_handle (2);
  ioa::automaton_handle<automaton1> input2_handle (3);
  ioa::automaton_handle<automaton1> input3_handle (4);
  ioa::automaton_handle<automaton1> binder_handle (5);

  ioa::action_executor<automaton1, automaton1::uv_up_input_action> input1 (input1_handle, &automaton1::uv_up_input);
  mu_assert (input1.fetch_instance (tm));
  ioa::action_executor<automaton1, automaton1::uv_p_input_action> input2 (input2_handle, &automaton1::uv_p_input, 890);
  mu_assert (input2.fetch_instance (tm));
  ioa::action_executor<automaton1, automaton1::uv_ap_input_action> input3 (input3_handle, &automaton1::uv_ap_input);
  mu_assert (input3.fetch_instance (tm));

  mu_assert (action.empty ());
  mu_assert (action.size () == 0);
  mu_assert (!action.involves_input (input1));
  mu_assert (!action.involves_input_automaton (input1_handle));
  mu_assert (!action.involves_binding (action, input1, binder_handle));
  mu_assert (!action.involves_aid_key (binder_handle, &input1));

  // Always set parameter before binding.
  action.set_parameter (input1_handle);
  input1.set_parameter (h);
  action.bind (tss, tm, input1, binder_handle, &input1);
  mu_assert (!action.empty ());
  mu_assert (action.size () == 1);
  mu_assert (action.involves_input (input1));
  mu_assert (action.involves_input_automaton (input1_handle));
  mu_assert (action.involves_binding (action, input1, binder_handle));
  mu_assert (action.involves_aid_key (binder_handle, &input1));

  action.set_parameter (input2_handle);
  input2.set_parameter (h);
  action.bind (tss, tm, input2, binder_handle, &input2);
  mu_assert (!action.empty ());
  mu_assert (action.size () == 2);
  mu_assert (action.involves_input (input2));
  mu_assert (action.involves_input_automaton (input2_handle));
  mu_assert (action.involves_binding (action, input2, binder_handle));
  mu_assert (action.involves_aid_key (binder_handle, &input2));

  action.set_parameter (input3_handle);
  input3.set_parameter (h);
  action.bind (tss, tm, input3, binder_handle, &input3);
  mu_assert (!action.empty ());
  mu_assert (action.size () == 3);
  mu_assert (action.involves_input (input3));
  mu_assert (action.involves_input_automaton (input3_handle));
  mu_assert (action.involves_binding (action, input3, binder_handle));
  mu_assert (action.involves_aid_key (binder_handle, &input3));

  mu_assert (action.fetch_instance (tm));
  action (tm, tss);
  mu_assert (tm.instance.uv_up_output.state);
  mu_assert (tm.instance.uv_up_input.state);
  mu_assert (tm.instance.uv_p_input.state);
  mu_assert (tm.instance.uv_ap_input.state);

  action.bound (tm, tss);
  mu_assert (tm.instance.uv_up_output.bound_);
  action.unbound (tm, tss);
  mu_assert (tm.instance.uv_up_output.unbound_);

  action.unbind (binder_handle, &input3);
  mu_assert (!action.empty ());
  mu_assert (action.size () == 2);
  mu_assert (!action.involves_input (input3));
  mu_assert (!action.involves_input_automaton (input3_handle));
  mu_assert (!action.involves_binding (action, input3, binder_handle));
  mu_assert (!action.involves_aid_key (binder_handle, &input3));

  action.unbind_automaton (input2_handle);
  mu_assert (!action.empty ());
  mu_assert (action.size () == 1);
  mu_assert (!action.involves_input (input2));
  mu_assert (!action.involves_input_automaton (input2_handle));
  mu_assert (!action.involves_binding (action, input2, binder_handle));
  mu_assert (!action.involves_aid_key (binder_handle, &input2));

  action.unbind_automaton (binder_handle);
  mu_assert (action.empty ());
  mu_assert (action.size () == 0);
  mu_assert (!action.involves_input (input1));
  mu_assert (!action.involves_input_automaton (input1_handle));
  mu_assert (!action.involves_binding (action, input1, binder_handle));
  mu_assert (!action.involves_aid_key (binder_handle, &input1));

  action.set_parameter (input1_handle);
  input1.set_parameter (h);
  action.bind (tss, tm, input1, binder_handle, &input1);
  action.set_parameter (input2_handle);
  input2.set_parameter (h);
  action.bind (tss, tm, input2, binder_handle, &input2);
  action.set_parameter (input3_handle);
  input3.set_parameter (h);
  action.bind (tss, tm, input3, binder_handle, &input3);
  mu_assert (!action.empty ());
  mu_assert (action.size () == 3);

  std::auto_ptr<ioa::output_executor_interface> clone (action.clone ());
  mu_assert (action == *clone);
  mu_assert (clone->empty ());
  mu_assert (clone->size () == 0);

  action.unbind_automaton (h);
  mu_assert (action.empty ());
  mu_assert (action.size () == 0);

  // Parameter didn't actually change.
  mu_assert (0U == action.get_pid ());

  return 0;
}

static const char*
unvalued_parameterized_output_action ()
{
  std::cout << __func__ << std::endl;

  // Must exist whole time.
  test_model tm;
  test_system_scheduler tss;

  int parameter = 345;
  ioa::automaton_handle<automaton1> h (1);
  ioa::action_executor<automaton1, automaton1::uv_p_output_action> action (h, &automaton1::uv_p_output, parameter);
  mu_assert (action.get_aid () == h);
  automaton1* i = 0;
  mu_assert (&((*i).*(&automaton1::uv_p_output)) == action.get_member_ptr ());
  mu_assert (size_t (parameter) == action.get_pid ());
  mu_assert (action == action);

  mu_assert (action.involves_output (action));

  ioa::automaton_handle<automaton1> input1_handle (2);
  ioa::automaton_handle<automaton1> input2_handle (3);
  ioa::automaton_handle<automaton1> input3_handle (4);
  ioa::automaton_handle<automaton1> binder_handle (5);

  ioa::action_executor<automaton1, automaton1::uv_up_input_action> input1 (input1_handle, &automaton1::uv_up_input);
  mu_assert (input1.fetch_instance (tm));
  ioa::action_executor<automaton1, automaton1::uv_p_input_action> input2 (input2_handle, &automaton1::uv_p_input, 890);
  mu_assert (input2.fetch_instance (tm));
  ioa::action_executor<automaton1, automaton1::uv_ap_input_action> input3 (input3_handle, &automaton1::uv_ap_input);
  mu_assert (input3.fetch_instance (tm));

  mu_assert (action.empty ());
  mu_assert (action.size () == 0);
  mu_assert (!action.involves_input (input1));
  mu_assert (!action.involves_input_automaton (input1_handle));
  mu_assert (!action.involves_binding (action, input1, binder_handle));
  mu_assert (!action.involves_aid_key (binder_handle, &input1));

  action.set_parameter (input1_handle);
  input1.set_parameter (h);
  action.bind (tss, tm, input1, binder_handle, &input1);
  mu_assert (!action.empty ());
  mu_assert (action.size () == 1);
  mu_assert (action.involves_input (input1));
  mu_assert (action.involves_input_automaton (input1_handle));
  mu_assert (action.involves_binding (action, input1, binder_handle));
  mu_assert (action.involves_aid_key (binder_handle, &input1));

  action.set_parameter (input2_handle);
  input2.set_parameter (h);
  action.bind (tss, tm, input2, binder_handle, &input2);
  mu_assert (!action.empty ());
  mu_assert (action.size () == 2);
  mu_assert (action.involves_input (input2));
  mu_assert (action.involves_input_automaton (input2_handle));
  mu_assert (action.involves_binding (action, input2, binder_handle));
  mu_assert (action.involves_aid_key (binder_handle, &input2));

  action.set_parameter (input3_handle);
  input3.set_parameter (h);
  action.bind (tss, tm, input3, binder_handle, &input3);
  mu_assert (!action.empty ());
  mu_assert (action.size () == 3);
  mu_assert (action.involves_input (input3));
  mu_assert (action.involves_input_automaton (input3_handle));
  mu_assert (action.involves_binding (action, input3, binder_handle));
  mu_assert (action.involves_aid_key (binder_handle, &input3));

  mu_assert (action.fetch_instance (tm));
  action (tm, tss);
  mu_assert (tm.instance.uv_p_output.state);
  mu_assert (parameter == tm.instance.uv_p_output.last_parameter);
  mu_assert (tm.instance.uv_up_input.state);
  mu_assert (tm.instance.uv_p_input.state);
  mu_assert (tm.instance.uv_ap_input.state);

  action.bound (tm, tss);
  mu_assert (tm.instance.uv_p_output.bound_);
  mu_assert (parameter == tm.instance.uv_p_output.bound_parameter);
  action.unbound (tm, tss);
  mu_assert (tm.instance.uv_p_output.unbound_);
  mu_assert (parameter == tm.instance.uv_p_output.unbound_parameter);

  action.unbind (binder_handle, &input3);
  mu_assert (!action.empty ());
  mu_assert (action.size () == 2);
  mu_assert (!action.involves_input (input3));
  mu_assert (!action.involves_input_automaton (input3_handle));
  mu_assert (!action.involves_binding (action, input3, binder_handle));
  mu_assert (!action.involves_aid_key (binder_handle, &input3));

  action.unbind_automaton (input2_handle);
  mu_assert (!action.empty ());
  mu_assert (action.size () == 1);
  mu_assert (!action.involves_input (input2));
  mu_assert (!action.involves_input_automaton (input2_handle));
  mu_assert (!action.involves_binding (action, input2, binder_handle));
  mu_assert (!action.involves_aid_key (binder_handle, &input2));

  action.unbind_automaton (binder_handle);
  mu_assert (action.empty ());
  mu_assert (action.size () == 0);
  mu_assert (!action.involves_input (input1));
  mu_assert (!action.involves_input_automaton (input1_handle));
  mu_assert (!action.involves_binding (action, input1, binder_handle));
  mu_assert (!action.involves_aid_key (binder_handle, &input1));

  action.set_parameter (input1_handle);
  input1.set_parameter (h);
  action.bind (tss, tm, input1, binder_handle, &input1);
  action.set_parameter (input2_handle);
  input2.set_parameter (h);
  action.bind (tss, tm, input2, binder_handle, &input2);
  action.set_parameter (input3_handle);
  input3.set_parameter (h);
  action.bind (tss, tm, input3, binder_handle, &input3);
  mu_assert (!action.empty ());
  mu_assert (action.size () == 3);

  std::auto_ptr<ioa::output_executor_interface> clone (action.clone ());
  mu_assert (action == *clone);
  mu_assert (clone->empty ());
  mu_assert (clone->size () == 0);

  action.unbind_automaton (h);
  mu_assert (action.empty ());
  mu_assert (action.size () == 0);

  // Parameter didn't actually change.
  mu_assert (size_t (parameter) == action.get_pid ());

  return 0;
}

static const char*
unvalued_auto_parameterized_output_action ()
{
  std::cout << __func__ << std::endl;

  // Must exist whole time.
  test_model tm;
  test_system_scheduler tss;

  ioa::automaton_handle<automaton1> h (1);
  ioa::action_executor<automaton1, automaton1::uv_ap_output_action> action (h, &automaton1::uv_ap_output);
  mu_assert (action.get_aid () == h);
  automaton1* i = 0;
  mu_assert (&((*i).*(&automaton1::uv_ap_output)) == action.get_member_ptr ());
  mu_assert (size_t (-1) == action.get_pid ());
  mu_assert (action == action);

  mu_assert (action.involves_output (action));

  ioa::automaton_handle<automaton1> input1_handle (2);
  ioa::automaton_handle<automaton1> binder_handle (5);

  ioa::action_executor<automaton1, automaton1::uv_up_input_action> input1 (input1_handle, &automaton1::uv_up_input);
  mu_assert (input1.fetch_instance (tm));

  mu_assert (action.empty ());
  mu_assert (action.size () == 0);
  mu_assert (!action.involves_input (input1));
  mu_assert (!action.involves_input_automaton (input1_handle));
  mu_assert (!action.involves_binding (action, input1, binder_handle));
  mu_assert (!action.involves_aid_key (binder_handle, &input1));

  action.set_parameter (input1_handle);
  input1.set_parameter (h);
  action.bind (tss, tm, input1, binder_handle, &input1);
  mu_assert (!action.empty ());
  mu_assert (action.size () == 1);
  mu_assert (action.involves_input (input1));
  mu_assert (action.involves_input_automaton (input1_handle));
  mu_assert (action.involves_binding (action, input1, binder_handle));
  mu_assert (action.involves_aid_key (binder_handle, &input1));

  mu_assert (size_t (input1_handle) == action.get_pid ());

  mu_assert (action.fetch_instance (tm));
  action (tm, tss);
  mu_assert (tm.instance.uv_ap_output.state);
  mu_assert (tm.instance.uv_ap_output.last_parameter == input1_handle);
  mu_assert (tm.instance.uv_up_input.state);

  action.bound (tm, tss);
  mu_assert (tm.instance.uv_ap_output.bound_);
  mu_assert (tm.instance.uv_ap_output.bound_parameter == input1_handle);
  action.unbound (tm, tss);
  mu_assert (tm.instance.uv_ap_output.unbound_);
  mu_assert (tm.instance.uv_ap_output.unbound_parameter == input1_handle);

  action.unbind_automaton (h);
  mu_assert (action.empty ());
  mu_assert (action.size () == 0);

  return 0;
}

static const char*
valued_unparameterized_output_action ()
{
  std::cout << __func__ << std::endl;

  // Must exist whole time.
  test_model tm;
  test_system_scheduler tss;

  ioa::automaton_handle<automaton1> h (1);
  ioa::action_executor<automaton1, automaton1::v_up_output_action> action (h, &automaton1::v_up_output);
  mu_assert (action.get_aid () == h);
  automaton1* i = 0;
  mu_assert (&((*i).*(&automaton1::v_up_output)) == action.get_member_ptr ());
  mu_assert (0U == action.get_pid ());
  mu_assert (action == action);

  mu_assert (action.involves_output (action));

  ioa::automaton_handle<automaton1> input1_handle (2);
  ioa::automaton_handle<automaton1> input2_handle (3);
  ioa::automaton_handle<automaton1> input3_handle (4);
  ioa::automaton_handle<automaton1> binder_handle (5);

  ioa::action_executor<automaton1, automaton1::v_up_input_action> input1 (input1_handle, &automaton1::v_up_input);
  mu_assert (input1.fetch_instance (tm));
  ioa::action_executor<automaton1, automaton1::v_p_input_action> input2 (input2_handle, &automaton1::v_p_input, 890);
  mu_assert (input2.fetch_instance (tm));
  ioa::action_executor<automaton1, automaton1::v_ap_input_action> input3 (input3_handle, &automaton1::v_ap_input);
  mu_assert (input3.fetch_instance (tm));

  mu_assert (action.empty ());
  mu_assert (action.size () == 0);
  mu_assert (!action.involves_input (input1));
  mu_assert (!action.involves_input_automaton (input1_handle));
  mu_assert (!action.involves_binding (action, input1, binder_handle));
  mu_assert (!action.involves_aid_key (binder_handle, &input1));

  // Always set parameter before binding.
  action.set_parameter (input1_handle);
  input1.set_parameter (h);
  action.bind (tss, tm, input1, binder_handle, &input1);
  mu_assert (!action.empty ());
  mu_assert (action.size () == 1);
  mu_assert (action.involves_input (input1));
  mu_assert (action.involves_input_automaton (input1_handle));
  mu_assert (action.involves_binding (action, input1, binder_handle));
  mu_assert (action.involves_aid_key (binder_handle, &input1));

  action.set_parameter (input2_handle);
  input2.set_parameter (h);
  action.bind (tss, tm, input2, binder_handle, &input2);
  mu_assert (!action.empty ());
  mu_assert (action.size () == 2);
  mu_assert (action.involves_input (input2));
  mu_assert (action.involves_input_automaton (input2_handle));
  mu_assert (action.involves_binding (action, input2, binder_handle));
  mu_assert (action.involves_aid_key (binder_handle, &input2));

  action.set_parameter (input3_handle);
  input3.set_parameter (h);
  action.bind (tss, tm, input3, binder_handle, &input3);
  mu_assert (!action.empty ());
  mu_assert (action.size () == 3);
  mu_assert (action.involves_input (input3));
  mu_assert (action.involves_input_automaton (input3_handle));
  mu_assert (action.involves_binding (action, input3, binder_handle));
  mu_assert (action.involves_aid_key (binder_handle, &input3));

  mu_assert (action.fetch_instance (tm));
  action (tm, tss);
  mu_assert (tm.instance.v_up_output.state);
  mu_assert (tm.instance.v_up_input.value == 9845);
  mu_assert (tm.instance.v_p_input.value == 9845);
  mu_assert (tm.instance.v_ap_input.value == 9845);

  action.bound (tm, tss);
  mu_assert (tm.instance.v_up_output.bound_);
  action.unbound (tm, tss);
  mu_assert (tm.instance.v_up_output.unbound_);

  action.unbind (binder_handle, &input3);
  mu_assert (!action.empty ());
  mu_assert (action.size () == 2);
  mu_assert (!action.involves_input (input3));
  mu_assert (!action.involves_input_automaton (input3_handle));
  mu_assert (!action.involves_binding (action, input3, binder_handle));
  mu_assert (!action.involves_aid_key (binder_handle, &input3));

  action.unbind_automaton (input2_handle);
  mu_assert (!action.empty ());
  mu_assert (action.size () == 1);
  mu_assert (!action.involves_input (input2));
  mu_assert (!action.involves_input_automaton (input2_handle));
  mu_assert (!action.involves_binding (action, input2, binder_handle));
  mu_assert (!action.involves_aid_key (binder_handle, &input2));

  action.unbind_automaton (binder_handle);
  mu_assert (action.empty ());
  mu_assert (action.size () == 0);
  mu_assert (!action.involves_input (input1));
  mu_assert (!action.involves_input_automaton (input1_handle));
  mu_assert (!action.involves_binding (action, input1, binder_handle));
  mu_assert (!action.involves_aid_key (binder_handle, &input1));

  action.set_parameter (input1_handle);
  input1.set_parameter (h);
  action.bind (tss, tm, input1, binder_handle, &input1);
  action.set_parameter (input2_handle);
  input2.set_parameter (h);
  action.bind (tss, tm, input2, binder_handle, &input2);
  action.set_parameter (input3_handle);
  input3.set_parameter (h);
  action.bind (tss, tm, input3, binder_handle, &input3);
  mu_assert (!action.empty ());
  mu_assert (action.size () == 3);

  std::auto_ptr<ioa::output_executor_interface> clone (action.clone ());
  mu_assert (action == *clone);
  mu_assert (clone->empty ());
  mu_assert (clone->size () == 0);

  action.unbind_automaton (h);
  mu_assert (action.empty ());
  mu_assert (action.size () == 0);

  // Parameter didn't actually change.
  mu_assert (0U == action.get_pid ());

  return 0;
}

static const char*
valued_parameterized_output_action ()
{
  std::cout << __func__ << std::endl;

  // Must exist whole time.
  test_model tm;
  test_system_scheduler tss;

  int parameter = 345;
  ioa::automaton_handle<automaton1> h (1);
  ioa::action_executor<automaton1, automaton1::v_p_output_action> action (h, &automaton1::v_p_output, parameter);
  mu_assert (action.get_aid () == h);
  automaton1* i = 0;
  mu_assert (&((*i).*(&automaton1::v_p_output)) == action.get_member_ptr ());
  mu_assert (size_t (parameter) == action.get_pid ());
  mu_assert (action == action);

  mu_assert (action.involves_output (action));

  ioa::automaton_handle<automaton1> input1_handle (2);
  ioa::automaton_handle<automaton1> input2_handle (3);
  ioa::automaton_handle<automaton1> input3_handle (4);
  ioa::automaton_handle<automaton1> binder_handle (5);

  ioa::action_executor<automaton1, automaton1::v_up_input_action> input1 (input1_handle, &automaton1::v_up_input);
  mu_assert (input1.fetch_instance (tm));
  ioa::action_executor<automaton1, automaton1::v_p_input_action> input2 (input2_handle, &automaton1::v_p_input, 890);
  mu_assert (input2.fetch_instance (tm));
  ioa::action_executor<automaton1, automaton1::v_ap_input_action> input3 (input3_handle, &automaton1::v_ap_input);
  mu_assert (input3.fetch_instance (tm));

  mu_assert (action.empty ());
  mu_assert (action.size () == 0);
  mu_assert (!action.involves_input (input1));
  mu_assert (!action.involves_input_automaton (input1_handle));
  mu_assert (!action.involves_binding (action, input1, binder_handle));
  mu_assert (!action.involves_aid_key (binder_handle, &input1));

  action.set_parameter (input1_handle);
  input1.set_parameter (h);
  action.bind (tss, tm, input1, binder_handle, &input1);
  mu_assert (!action.empty ());
  mu_assert (action.size () == 1);
  mu_assert (action.involves_input (input1));
  mu_assert (action.involves_input_automaton (input1_handle));
  mu_assert (action.involves_binding (action, input1, binder_handle));
  mu_assert (action.involves_aid_key (binder_handle, &input1));

  action.set_parameter (input2_handle);
  input2.set_parameter (h);
  action.bind (tss, tm, input2, binder_handle, &input2);
  mu_assert (!action.empty ());
  mu_assert (action.size () == 2);
  mu_assert (action.involves_input (input2));
  mu_assert (action.involves_input_automaton (input2_handle));
  mu_assert (action.involves_binding (action, input2, binder_handle));
  mu_assert (action.involves_aid_key (binder_handle, &input2));

  action.set_parameter (input3_handle);
  input3.set_parameter (h);
  action.bind (tss, tm, input3, binder_handle, &input3);
  mu_assert (!action.empty ());
  mu_assert (action.size () == 3);
  mu_assert (action.involves_input (input3));
  mu_assert (action.involves_input_automaton (input3_handle));
  mu_assert (action.involves_binding (action, input3, binder_handle));
  mu_assert (action.involves_aid_key (binder_handle, &input3));

  mu_assert (action.fetch_instance (tm));
  action (tm, tss);
  mu_assert (tm.instance.v_p_output.state);
  mu_assert (parameter == tm.instance.v_p_output.last_parameter);
  mu_assert (tm.instance.v_up_input.value == 9845);
  mu_assert (tm.instance.v_p_input.value == 9845);
  mu_assert (tm.instance.v_ap_input.value == 9845);

  action.bound (tm, tss);
  mu_assert (tm.instance.v_p_output.bound_);
  mu_assert (parameter == tm.instance.v_p_output.bound_parameter);
  action.unbound (tm, tss);
  mu_assert (tm.instance.v_p_output.unbound_);
  mu_assert (parameter == tm.instance.v_p_output.unbound_parameter);

  action.unbind (binder_handle, &input3);
  mu_assert (!action.empty ());
  mu_assert (action.size () == 2);
  mu_assert (!action.involves_input (input3));
  mu_assert (!action.involves_input_automaton (input3_handle));
  mu_assert (!action.involves_binding (action, input3, binder_handle));
  mu_assert (!action.involves_aid_key (binder_handle, &input3));

  action.unbind_automaton (input2_handle);
  mu_assert (!action.empty ());
  mu_assert (action.size () == 1);
  mu_assert (!action.involves_input (input2));
  mu_assert (!action.involves_input_automaton (input2_handle));
  mu_assert (!action.involves_binding (action, input2, binder_handle));
  mu_assert (!action.involves_aid_key (binder_handle, &input2));

  action.unbind_automaton (binder_handle);
  mu_assert (action.empty ());
  mu_assert (action.size () == 0);
  mu_assert (!action.involves_input (input1));
  mu_assert (!action.involves_input_automaton (input1_handle));
  mu_assert (!action.involves_binding (action, input1, binder_handle));
  mu_assert (!action.involves_aid_key (binder_handle, &input1));

  action.set_parameter (input1_handle);
  input1.set_parameter (h);
  action.bind (tss, tm, input1, binder_handle, &input1);
  action.set_parameter (input2_handle);
  input2.set_parameter (h);
  action.bind (tss, tm, input2, binder_handle, &input2);
  action.set_parameter (input3_handle);
  input3.set_parameter (h);
  action.bind (tss, tm, input3, binder_handle, &input3);
  mu_assert (!action.empty ());
  mu_assert (action.size () == 3);

  std::auto_ptr<ioa::output_executor_interface> clone (action.clone ());
  mu_assert (action == *clone);
  mu_assert (clone->empty ());
  mu_assert (clone->size () == 0);

  action.unbind_automaton (h);
  mu_assert (action.empty ());
  mu_assert (action.size () == 0);

  // Parameter didn't actually change.
  mu_assert (size_t (parameter) == action.get_pid ());

  return 0;
}

static const char*
valued_auto_parameterized_output_action ()
{
  std::cout << __func__ << std::endl;

  // Must exist whole time.
  test_model tm;
  test_system_scheduler tss;

  ioa::automaton_handle<automaton1> h (1);
  ioa::action_executor<automaton1, automaton1::v_ap_output_action> action (h, &automaton1::v_ap_output);
  mu_assert (action.get_aid () == h);
  automaton1* i = 0;
  mu_assert (&((*i).*(&automaton1::v_ap_output)) == action.get_member_ptr ());
  mu_assert (size_t (-1) == action.get_pid ());
  mu_assert (action == action);

  mu_assert (action.involves_output (action));

  ioa::automaton_handle<automaton1> input1_handle (2);
  ioa::automaton_handle<automaton1> binder_handle (5);

  ioa::action_executor<automaton1, automaton1::v_up_input_action> input1 (input1_handle, &automaton1::v_up_input);
  mu_assert (input1.fetch_instance (tm));

  mu_assert (action.empty ());
  mu_assert (action.size () == 0);
  mu_assert (!action.involves_input (input1));
  mu_assert (!action.involves_input_automaton (input1_handle));
  mu_assert (!action.involves_binding (action, input1, binder_handle));
  mu_assert (!action.involves_aid_key (binder_handle, &input1));

  action.set_parameter (input1_handle);
  input1.set_parameter (h);
  action.bind (tss, tm, input1, binder_handle, &input1);
  mu_assert (!action.empty ());
  mu_assert (action.size () == 1);
  mu_assert (action.involves_input (input1));
  mu_assert (action.involves_input_automaton (input1_handle));
  mu_assert (action.involves_binding (action, input1, binder_handle));
  mu_assert (action.involves_aid_key (binder_handle, &input1));

  mu_assert (size_t (input1_handle) == action.get_pid ());

  mu_assert (action.fetch_instance (tm));
  action (tm, tss);
  mu_assert (tm.instance.v_ap_output.state);
  mu_assert (tm.instance.v_ap_output.last_parameter == input1_handle);
  mu_assert (tm.instance.v_up_input.value == 9845);

  action.bound (tm, tss);
  mu_assert (tm.instance.v_ap_output.bound_);
  mu_assert (tm.instance.v_ap_output.bound_parameter == input1_handle);
  action.unbound (tm, tss);
  mu_assert (tm.instance.v_ap_output.unbound_);
  mu_assert (tm.instance.v_ap_output.unbound_parameter == input1_handle);

  action.unbind_automaton (h);
  mu_assert (action.empty ());
  mu_assert (action.size () == 0);

  return 0;
}

static const char*
unparameterized_internal_action ()
{
  std::cout << __func__ << std::endl;
  ioa::automaton_handle<automaton1> h;
  ioa::action_executor<automaton1, automaton1::up_internal_action> action (h, &automaton1::up_internal);
  mu_assert (action.get_aid () == h);
  automaton1* i = 0;
  mu_assert (&((*i).*(&automaton1::up_internal)) == action.get_member_ptr ());
  mu_assert (0U == action.get_pid ());
  mu_assert (action == action);

  test_model tm;
  test_system_scheduler tss;
  mu_assert (action.fetch_instance (tm));
  action (tm, tss);
  mu_assert (tm.instance.up_internal.state);

  return 0;
}

static const char*
parameterized_internal_action ()
{
  std::cout << __func__ << std::endl;
  int parameter = 345;
  ioa::automaton_handle<automaton1> h;
  ioa::action_executor<automaton1, automaton1::p_internal_action> action (h, &automaton1::p_internal, parameter);
  mu_assert (action.get_aid () == h);
  automaton1* i = 0;
  mu_assert (&((*i).*(&automaton1::p_internal)) == action.get_member_ptr ());
  mu_assert (size_t (parameter) == action.get_pid ());
  mu_assert (action == action);

  test_model tm;
  test_system_scheduler tss;
  mu_assert (action.fetch_instance (tm));
  action (tm, tss);
  mu_assert (tm.instance.p_internal.state);
  mu_assert (parameter == tm.instance.p_internal.last_parameter);

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
