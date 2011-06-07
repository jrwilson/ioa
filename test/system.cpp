#include "minunit.h"

#include <ioa/system.hpp>
#include <ioa/generator.hpp>
#include "automaton1.hpp"

#include "instance_holder.hpp"
#include "test_system_scheduler.hpp"

ioa::aid_t create (std::auto_ptr<ioa::generator_interface> generator) {
  tss.reset ();
  ioa::aid_t handle = ioa::system::create (generator);
  assert (handle != -1);
  assert (tss.m_init_automaton == handle);

  return handle;
}

ioa::aid_t create (const ioa::aid_t creator,
		   std::auto_ptr<ioa::generator_interface> generator,
		   void* const key) {
  tss.reset ();
  ioa::aid_t handle = ioa::system::create (creator, generator, key);
  assert (handle != -1);
  assert (tss.m_init_automaton == handle);
  assert (tss.m_automaton_created_automaton == creator);
  assert (tss.m_automaton_created_key == key);
  assert (tss.m_automaton_created_child == handle);

  return handle;
}

template <class OI, class OM, class II, class IM>
void bind (ioa::system::bind_executor<OI, OM, II, IM>& exec,
	   const ioa::aid_t binder,
	   void* const key) {
  tss.reset ();
  assert (ioa::system::bind (exec, binder, key) == 0);
  assert (tss.m_bound_automaton == binder);
  assert (tss.m_bound_key == key);
}

void destroy (const ioa::aid_t automaton)
{
  tss.reset ();
  assert (ioa::system::destroy (automaton) == 0);
  // TODO:  Check something here.
}

static const char*
creator_dne ()
{
  // Reset the system.
  ioa::system::clear ();

  ioa::aid_t creator_handle = create (ioa::make_generator<automaton1> ());

  destroy (creator_handle);

  tss.reset ();
  ioa::aid_t handle = ioa::system::create (creator_handle, ioa::make_generator<automaton1> (), 0);
  mu_assert (handle == -1);

  return 0;
}

static const char*
create_key_exists ()
{
  // Reset the system.
  ioa::system::clear ();

  ioa::aid_t creator_handle = create (ioa::make_generator<automaton1> ());

  int key;
  create (creator_handle, ioa::make_generator<automaton1> (), &key);

  tss.reset ();
  ioa::aid_t handle = ioa::system::create (creator_handle, ioa::make_generator<automaton1> (), &key);
  mu_assert (handle == -1);
  mu_assert (tss.m_create_key_exists_automaton == creator_handle);
  mu_assert (tss.m_create_key_exists_key == &key);
  
  return 0;
}

static const char*
instance_exists ()
{
  // Reset the system.
  ioa::system::clear ();

  automaton1* instance2 = new automaton1 ();
  std::auto_ptr<ioa::generator_interface> holder2 (new instance_holder<automaton1> (instance2));
  std::auto_ptr<ioa::generator_interface> holder3 (new instance_holder<automaton1> (instance2));

  ioa::aid_t creator = create (ioa::make_generator<automaton1> ());

  int key1;
  create (creator, holder2, &key1);

  tss.reset ();
  int key2;
  ioa::aid_t h2 = ioa::system::create (creator, holder3, &key2);
  mu_assert (h2 == -1);
  mu_assert (tss.m_instance_exists_automaton == creator);
  mu_assert (tss.m_instance_exists_key == &key2);

  return 0;
}

static const char*
automaton_created ()
{
  // Reset the system.
  ioa::system::clear ();

  ioa::aid_t creator = create (ioa::make_generator<automaton1> ());

  int key;
  ioa::aid_t handle = ioa::system::create (creator, ioa::make_generator<automaton1> (), &key);
  mu_assert (handle != -1);
  mu_assert (tss.m_init_automaton == handle);
  mu_assert (tss.m_automaton_created_automaton == creator);
  mu_assert (tss.m_automaton_created_key == &key);
  mu_assert (tss.m_automaton_created_child == handle);

  return 0;
}

static const char*
binder_dne ()
{
  // Reset the system.
  ioa::system::clear ();
  
  ioa::aid_t binder = create (ioa::make_generator<automaton1> ());
  destroy (binder);
  
  ioa::automaton_handle<automaton1> output = create (ioa::make_generator<automaton1> ());
  
  ioa::automaton_handle<automaton1> input = create (ioa::make_generator<automaton1> ());
  
  ioa::system::bind_executor<automaton1, automaton1::up_uv_output_action,
			     automaton1, automaton1::up_uv_input_action> b (ioa::make_action (output, &automaton1::up_uv_output),
									    ioa::make_action (input, &automaton1::up_uv_input));
									    
  mu_assert (ioa::system::bind (b, binder, 0) == -1);
  
  return 0;
}

static const char*
bind_key_exists ()
{
  // Reset the system.
  ioa::system::clear ();
  
  ioa::aid_t binder = create (ioa::make_generator<automaton1> ());
  
  ioa::automaton_handle<automaton1> output1 = create (ioa::make_generator<automaton1> ());
  
  ioa::automaton_handle<automaton1> input1 = create (ioa::make_generator<automaton1> ());

  ioa::system::bind_executor<automaton1, automaton1::up_uv_output_action,
			     automaton1, automaton1::up_uv_input_action> b1 (ioa::make_action (output1, &automaton1::up_uv_output),
									     ioa::make_action (input1, &automaton1::up_uv_input));

  ioa::automaton_handle<automaton1> output2 = create (ioa::make_generator<automaton1> ());
  
  ioa::automaton_handle<automaton1> input2 = create (ioa::make_generator<automaton1> ());

  ioa::system::bind_executor<automaton1, automaton1::up_uv_output_action,
			     automaton1, automaton1::up_uv_input_action> b2 (ioa::make_action (output2, &automaton1::up_uv_output),
									     ioa::make_action (input2, &automaton1::up_uv_input));

  int key;
  bind (b1, binder, &key);
  
  tss.reset ();
  mu_assert (ioa::system::bind (b2, binder, &key) == -1);
  mu_assert (tss.m_bind_key_exists_automaton == binder);
  mu_assert (tss.m_bind_key_exists_key == &key);
  
  return 0;
}

static const char*
output_automaton_dne ()
{
  // Reset the system.
  ioa::system::clear ();

  ioa::aid_t binder = create (ioa::make_generator<automaton1> ());
  
  ioa::automaton_handle<automaton1> output = create (ioa::make_generator<automaton1> ());
  destroy (output);
  
  ioa::automaton_handle<automaton1> input = create (ioa::make_generator<automaton1> ());

  ioa::system::bind_executor<automaton1, automaton1::up_uv_output_action,
			     automaton1, automaton1::up_uv_input_action> b (ioa::make_action (output, &automaton1::up_uv_output),
									    ioa::make_action (input, &automaton1::up_uv_input));

  tss.reset ();
  int key;
  mu_assert (ioa::system::bind (b, binder, &key) == -1);
  mu_assert (tss.m_output_automaton_dne_automaton == binder);
  mu_assert (tss.m_output_automaton_dne_key == &key);
  
  return 0;
}

static const char*
input_automaton_dne ()
{
  // Reset the system.
  ioa::system::clear ();

  ioa::aid_t binder = create (ioa::make_generator<automaton1> ());
  
  ioa::automaton_handle<automaton1> output = create (ioa::make_generator<automaton1> ());
  
  ioa::automaton_handle<automaton1> input = create (ioa::make_generator<automaton1> ());

  ioa::system::bind_executor<automaton1, automaton1::up_uv_output_action,
			     automaton1, automaton1::up_uv_input_action> b (ioa::make_action (output, &automaton1::up_uv_output),
									    ioa::make_action (input, &automaton1::up_uv_input));

  destroy (input);
  
  tss.reset ();
  int key;
  mu_assert (ioa::system::bind (b, binder, &key) == -1);
  mu_assert (tss.m_input_automaton_dne_automaton == binder);
  mu_assert (tss.m_input_automaton_dne_key == &key);
  
  return 0;
}

static const char*
binding_exists ()
{
  // Reset the system.
  ioa::system::clear ();

  ioa::automaton_handle<automaton1> binder = create (ioa::make_generator<automaton1> ());
  
  ioa::automaton_handle<automaton1> output = create (ioa::make_generator<automaton1> ());
  
  ioa::automaton_handle<automaton1> input = create (ioa::make_generator<automaton1> ());

  ioa::system::bind_executor<automaton1, automaton1::up_uv_output_action,
			     automaton1, automaton1::up_uv_input_action> b (ioa::make_action (output, &automaton1::up_uv_output),
									    ioa::make_action (input, &automaton1::up_uv_input));

  int key1;
  bind (b, binder, &key1);
  
  tss.reset ();
  int key2;
  mu_assert (ioa::system::bind (b, binder, &key2) == -1);
  mu_assert (tss.m_binding_exists_automaton == binder);
  mu_assert (tss.m_binding_exists_key == &key2);

  return 0;
}

static const char*
input_action_unavailable ()
{
  // Reset the system.
  ioa::system::clear ();

  ioa::automaton_handle<automaton1> binder = create (ioa::make_generator<automaton1> ());
  
  ioa::automaton_handle<automaton1> output1 = create (ioa::make_generator<automaton1> ());

  ioa::automaton_handle<automaton1> output2 = create (ioa::make_generator<automaton1> ());
  
  ioa::automaton_handle<automaton1> input = create (ioa::make_generator<automaton1> ());

  ioa::system::bind_executor<automaton1, automaton1::up_uv_output_action,
			     automaton1, automaton1::up_uv_input_action> b1 (ioa::make_action (output1, &automaton1::up_uv_output),
									     ioa::make_action (input, &automaton1::up_uv_input));
  
  ioa::system::bind_executor<automaton1, automaton1::up_uv_output_action,
			     automaton1, automaton1::up_uv_input_action> b2 (ioa::make_action (output2, &automaton1::up_uv_output),
									     ioa::make_action (input, &automaton1::up_uv_input));


  int key1;
  bind (b1, binder, &key1);

  tss.reset ();
  int key2;
  mu_assert (ioa::system::bind (b2, binder, &key2) == -1);
  mu_assert (tss.m_input_action_unavailable_automaton == binder);
  mu_assert (tss.m_input_action_unavailable_key == &key2);

  return 0;
}

static const char*
output_action_unavailable ()
{
  // Reset the system.
  ioa::system::clear ();
  
  int parameter = 0;

  ioa::automaton_handle<automaton1> binder = create (ioa::make_generator<automaton1> ());
  
  ioa::automaton_handle<automaton1> output = create (ioa::make_generator<automaton1> ());

  ioa::automaton_handle<automaton1> input = create (ioa::make_generator<automaton1> ());

  ioa::system::bind_executor<automaton1, automaton1::up_uv_output_action,
			     automaton1, automaton1::up_uv_input_action> b1 (ioa::make_action (output, &automaton1::up_uv_output),
									     ioa::make_action (input, &automaton1::up_uv_input));
  
  ioa::system::bind_executor<automaton1, automaton1::up_uv_output_action,
			     automaton1, automaton1::p_uv_input_action> b2 (ioa::make_action (output, &automaton1::up_uv_output),
									    ioa::make_action (input, &automaton1::p_uv_input, parameter));
  
  int key1;
  bind (b1, binder, &key1);

  tss.reset ();
  int key2;
  mu_assert (ioa::system::bind (b2, input, &key2) == -1);
  mu_assert (tss.m_output_action_unavailable_automaton == input);
  mu_assert (tss.m_output_action_unavailable_key == &key2);

  return 0;
}

/*
  TODO:  Incorporate all of the tests in binding.cpp.
 */
static const char*
bound ()
{
  // Reset the system.
  ioa::system::clear ();
  
  int parameter = 0;

  ioa::automaton_handle<automaton1> output = create (ioa::make_generator<automaton1> ());

  automaton1* instance = new automaton1 ();
  std::auto_ptr<ioa::generator_interface> holder (new instance_holder<automaton1> (instance));

  ioa::automaton_handle<automaton1> input = create (holder);

  ioa::system::bind_executor<automaton1, automaton1::p_uv_output_action,
			     automaton1, automaton1::up_uv_input_action> b (ioa::make_action (output, &automaton1::p_uv_output, parameter),
									    ioa::make_action (input, &automaton1::up_uv_input));
  
  tss.reset ();
  int key;
  mu_assert (ioa::system::bind (b, output, &key) == 0);
  mu_assert (tss.m_bound_automaton == output);
  mu_assert (tss.m_bound_key == &key);

  ioa::system::action_executor<automaton1, automaton1::p_uv_output_action> out (ioa::make_action (output, &automaton1::p_uv_output, parameter));

  mu_assert (ioa::system::execute (out) == 0);
  mu_assert (instance->up_uv_input.state);

  return 0;
}

static const char*
unbinder_dne ()
{
  // Reset the system.
  ioa::system::clear ();
  
  ioa::automaton_handle<automaton1> binder = create (ioa::make_generator<automaton1> ());
  
  ioa::automaton_handle<automaton1> output = create (ioa::make_generator<automaton1> ());

  ioa::automaton_handle<automaton1> input = create (ioa::make_generator<automaton1> ());

  ioa::system::bind_executor<automaton1, automaton1::up_uv_output_action,
			     automaton1, automaton1::up_uv_input_action> b (ioa::make_action (output, &automaton1::up_uv_output),
									    ioa::make_action (input, &automaton1::up_uv_input));
  
  bind (b, binder, 0);
  
  destroy (binder);

  mu_assert (ioa::system::unbind (binder,
				  0) == -1);

  return 0;
}

static const char*
bind_key_dne ()
{
  // Reset the system.
  ioa::system::clear ();
  
  ioa::automaton_handle<automaton1> binder = create (ioa::make_generator<automaton1> ());
  
  ioa::automaton_handle<automaton1> output = create (ioa::make_generator<automaton1> ());

  ioa::automaton_handle<automaton1> input = create (ioa::make_generator<automaton1> ());

  ioa::system::bind_executor<automaton1, automaton1::up_uv_output_action,
			     automaton1, automaton1::up_uv_input_action> b (ioa::make_action (output, &automaton1::up_uv_output),
									    ioa::make_action (input, &automaton1::up_uv_input));

  int key;
  bind (b, binder, &key);
  
  tss.reset ();
  mu_assert (ioa::system::unbind (binder,
				  &key) == 0);
  mu_assert (tss.m_unbound.count (std::make_pair (binder, &key)) == 1);

  tss.reset ();
  mu_assert (ioa::system::unbind (binder,
				  &key) == -1);
  mu_assert (tss.m_bind_key_dne_automaton == binder);
  mu_assert (tss.m_bind_key_dne_key == &key);

  return 0;
}

static const char*
unbound ()
{
  // Reset the system.
  ioa::system::clear ();
  
  int parameter = 0;

  ioa::automaton_handle<automaton1> output = create (ioa::make_generator<automaton1> ());

  ioa::automaton_handle<automaton1> input = create (ioa::make_generator<automaton1> ());

  ioa::system::bind_executor<automaton1, automaton1::p_uv_output_action,
			     automaton1, automaton1::up_uv_input_action> b (ioa::make_action (output, &automaton1::p_uv_output, parameter),
									    ioa::make_action (input, &automaton1::up_uv_input));

  int key;
  bind (b, output, &key);
  
  tss.reset ();
  mu_assert (ioa::system::unbind (output,
				  &key) == 0);
  mu_assert (tss.m_unbound.count (std::make_pair (output, &key)) == 1);

  return 0;
}

static const char*
destroyer_dne ()
{
  // Reset the system.
  ioa::system::clear ();
  
  ioa::automaton_handle<automaton1> parent_handle = create (ioa::make_generator<automaton1> ());
  int key;
  ioa::automaton_handle<automaton1> child_handle = create (parent_handle, ioa::make_generator<automaton1> (), &key);

  destroy (parent_handle);
  
  mu_assert (ioa::system::destroy (parent_handle, &key) == -1);

  return 0;
}

static const char*
create_key_dne ()
{
  // Reset the system.
  ioa::system::clear ();

  ioa::aid_t parent_handle = create (ioa::make_generator<automaton1> ());
  
  int key;
  ioa::automaton_handle<automaton1> child_handle = create (parent_handle, ioa::make_generator<automaton1> (), &key);
  
  tss.reset ();
  mu_assert (ioa::system::destroy (parent_handle, &key) == 0);
  mu_assert (tss.m_automaton_destroyed.count (std::make_pair (parent_handle, &key)) == 1);

  tss.reset ();
  mu_assert (ioa::system::destroy (parent_handle, &key) == -1);
  mu_assert (tss.m_create_key_dne_automaton == parent_handle);
  mu_assert (tss.m_create_key_dne_key == &key);

  return 0;
}

static const char*
automaton_destroyed ()
{
  // Reset the system.
  ioa::system::clear ();
  
  int parameter = 0;

  ioa::automaton_handle<automaton1> alpha = create (ioa::make_generator<automaton1> ());

  int beta_key;
  ioa::automaton_handle<automaton1> beta = create (alpha, ioa::make_generator<automaton1> (), &beta_key);

  int gamma_key;
  ioa::automaton_handle<automaton1> gamma = create (beta, ioa::make_generator<automaton1> (), &gamma_key);

  ioa::system::bind_executor<automaton1, automaton1::p_uv_output_action,
			     automaton1, automaton1::up_uv_input_action> b1 (ioa::make_action (alpha, &automaton1::p_uv_output, parameter),
									     ioa::make_action (beta, &automaton1::up_uv_input));

  ioa::system::bind_executor<automaton1, automaton1::p_uv_output_action,
			     automaton1, automaton1::up_uv_input_action> b2 (ioa::make_action (beta, &automaton1::p_uv_output, parameter),
									     ioa::make_action (gamma, &automaton1::up_uv_input));

  ioa::system::bind_executor<automaton1, automaton1::up_uv_output_action,
			     automaton1, automaton1::p_uv_input_action> b3 (ioa::make_action (gamma, &automaton1::up_uv_output),
									    ioa::make_action (beta, &automaton1::p_uv_input, parameter));

  int bind1;
  bind (b1, alpha, &bind1);
  int bind2;
  bind (b2, beta, &bind2);
  int bind3;
  bind (b3, beta, &bind3);

  tss.reset ();
  mu_assert (ioa::system::destroy (beta) == 0);
  mu_assert (tss.m_automaton_destroyed.count (std::make_pair (alpha, &beta_key)) == 1);
  mu_assert (tss.m_automaton_destroyed.count (std::make_pair (beta, &gamma_key)) == 1);
  mu_assert (tss.m_unbound.count (std::make_pair (alpha, &bind1)) == 1);
  mu_assert (tss.m_unbound.count (std::make_pair (beta, &bind2)) == 1);
  mu_assert (tss.m_unbound.count (std::make_pair (beta, &bind3)) == 1);
  
  return 0;
}

static const char*
execute_automaton_dne ()
{
  // Reset the system.
  ioa::system::clear ();
  
  ioa::automaton_handle<automaton1> output = create (ioa::make_generator<automaton1> ());
  
  destroy (output);
  
  ioa::system::action_executor<automaton1, automaton1::up_uv_output_action> exec (ioa::make_action (output, &automaton1::up_uv_output));
  mu_assert (ioa::system::execute (exec) == -1);
  
  return 0;
}

static const char*
execute_output ()
{
  // Reset the system.
  ioa::system::clear ();
  
  automaton1* output_instance = new automaton1 ();
  std::auto_ptr<ioa::generator_interface> holder (new instance_holder<automaton1> (output_instance));

  ioa::automaton_handle<automaton1> output = create (holder);

  ioa::system::action_executor<automaton1, automaton1::up_uv_output_action> exec1 (ioa::make_action (output, &automaton1::up_uv_output));
  mu_assert (ioa::system::execute (exec1) == 0);
  mu_assert (output_instance->up_uv_output.state);

  ioa::system::action_executor<automaton1, automaton1::p_uv_output_action> exec2 (ioa::make_action (output, &automaton1::p_uv_output, 567));
  mu_assert (ioa::system::execute (exec2) == 0);
  mu_assert (output_instance->p_uv_output.state);
  mu_assert (output_instance->p_uv_output.last_parameter == 567);

  ioa::system::action_executor<automaton1, automaton1::up_v_output_action> exec3 (ioa::make_action (output, &automaton1::up_v_output));
  mu_assert (ioa::system::execute (exec3) == 0);
  mu_assert (output_instance->up_v_output.state);

  ioa::system::action_executor<automaton1, automaton1::p_v_output_action> exec4 (ioa::make_action (output, &automaton1::p_v_output, 123));
  mu_assert (ioa::system::execute (exec4) == 0);
  mu_assert (output_instance->p_v_output.state);
  mu_assert (output_instance->p_v_output.last_parameter == 123);

  return 0;
}

static const char*
execute_internal ()
{
  // Reset the system.
  ioa::system::clear ();
  
  automaton1* output_instance = new automaton1 ();
  std::auto_ptr<ioa::generator_interface> holder (new instance_holder<automaton1> (output_instance));

  ioa::automaton_handle<automaton1> output = create (holder);

  ioa::system::action_executor<automaton1, automaton1::up_internal_action> exec1 (ioa::make_action (output, &automaton1::up_internal));
  mu_assert (ioa::system::execute (exec1) == 0);
  mu_assert (output_instance->up_internal.state);

  ioa::system::action_executor<automaton1, automaton1::p_internal_action> exec2 (ioa::make_action (output, &automaton1::p_internal, 567));
  mu_assert (ioa::system::execute (exec2) == 0);
  mu_assert (output_instance->p_internal.state);
  mu_assert (output_instance->p_internal.last_parameter == 567);

  return 0;
}

static const char*
recipient_dne ()
{
  // Reset the system.
  ioa::system::clear ();
  
  ioa::automaton_handle<automaton1> from = create (ioa::make_generator<automaton1> ());
  ioa::automaton_handle<automaton1> to = create (ioa::make_generator<automaton1> ());
  
  destroy (to);

  tss.reset ();
  int key;
  ioa::system::action_executor<automaton1, automaton1::uv_event_action> exec (ioa::make_action (to, &automaton1::uv_event));
  mu_assert (ioa::system::execute (from, exec, &key) == -1);
  mu_assert (tss.m_recipient_dne_automaton == from);
  mu_assert (tss.m_recipient_dne_key == &key);

  return 0;
}

static const char*
event_delivered ()
{
  // Reset the system.
  ioa::system::clear ();
  
  automaton1* event_instance = new automaton1 ();
  std::auto_ptr<ioa::generator_interface> holder (new instance_holder<automaton1> (event_instance));

  ioa::automaton_handle<automaton1> from = create (ioa::make_generator<automaton1> ());
  ioa::automaton_handle<automaton1> to = create (holder);

  tss.reset ();
  int key1;
ioa::system::action_executor<automaton1, automaton1::uv_event_action> exec1 (ioa::make_action (to, &automaton1::uv_event));
  mu_assert (ioa::system::execute (from, exec1, &key1) == 0);
  mu_assert (tss.m_event_delivered_automaton == from);
  mu_assert (tss.m_event_delivered_key == &key1);
  mu_assert (event_instance->uv_event.state);

  tss.reset ();
  int key2;
  ioa::system::action_executor<automaton1, automaton1::v_event_action> exec2 (ioa::make_action (to, &automaton1::v_event, 37));
  mu_assert (ioa::system::execute (from, exec2, &key2) == 0);
  mu_assert (tss.m_event_delivered_automaton == from);
  mu_assert (tss.m_event_delivered_key == &key2);
  mu_assert (event_instance->v_event.state);
  mu_assert (event_instance->v_event.last_value == 37);

  return 0;
}

// static const char*
// execute_system_input ()
// {
//   // Reset the system.
//   ioa::system::clear ();
  
//   automaton1* event_instance = new automaton1 ();
//   std::auto_ptr<ioa::generator_interface> holder (new instance_holder<automaton1> (event_instance));

//   ioa::automaton_handle<automaton1> from = create (ioa::make_generator<automaton1> ());
//   ioa::automaton_handle<automaton1> to = create (holder);

//   // tss.reset ();
//   // int key1;
//   // mu_assert (ioa::system::execute (from, ioa::make_action (to, &automaton1::uv_event), &key1) == 0);
//   // mu_assert (tss.m_event_delivered_automaton == from);
//   // mu_assert (tss.m_event_delivered_key == &key1);
//   // mu_assert (event_instance->uv_event.state);

//   // tss.reset ();
//   // int key2;
//   // mu_assert (ioa::system::execute (from, ioa::make_action (to, &automaton1::v_event, 37), &key2) == 0);
//   // mu_assert (tss.m_event_delivered_automaton == from);
//   // mu_assert (tss.m_event_delivered_key == &key2);
//   // mu_assert (event_instance->v_event.state);
//   // mu_assert (event_instance->v_event.last_value == 37);

//   return 0;
// }

const char*
all_tests ()
{
  mu_run_test (creator_dne);
  mu_run_test (create_key_exists);
  mu_run_test (instance_exists);
  mu_run_test (automaton_created);
  mu_run_test (binder_dne);
  mu_run_test (bind_key_exists);
  mu_run_test (output_automaton_dne);
  mu_run_test (input_automaton_dne);
  mu_run_test (binding_exists);
  mu_run_test (input_action_unavailable);
  mu_run_test (output_action_unavailable);
  mu_run_test (bound);
  mu_run_test (unbinder_dne);
  mu_run_test (bind_key_dne);
  mu_run_test (unbound);
  mu_run_test (destroyer_dne);
  mu_run_test (create_key_dne);
  mu_run_test (automaton_destroyed);
  mu_run_test (execute_automaton_dne);
  mu_run_test (execute_output);
  mu_run_test (execute_internal);
  mu_run_test (recipient_dne);
  mu_run_test (event_delivered);
  // mu_run_test (execute_system_input);
  // mu_run_test (execute_create);
  // mu_run_test (execute_bind);
  // mu_run_test (execute_unbind);
  // mu_run_test (execute_destroy);

  return 0;
}
