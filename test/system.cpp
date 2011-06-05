#include "minunit.h"

#include <ioa/system.hpp>
#include <ioa/generator.hpp>
#include "automaton1.hpp"

#include "test_system_scheduler.hpp"
#include "instance_holder.hpp"

ioa::aid_t create (std::auto_ptr<ioa::generator_interface> generator) {
  ioa::aid_t handle = ioa::system::create (generator);
  assert (handle != -1);
  return handle;
}

ioa::aid_t create (const ioa::aid_t creator,
		   std::auto_ptr<ioa::generator_interface> generator) {
  ioa::aid_t handle = ioa::system::create (creator, generator, 0);
  assert (handle != -1);
  return handle;
}

template <class OI, class OM, class II, class IM>
ioa::bid_t bind (const ioa::action<OI, OM>& output_action,
		 const ioa::action<II, IM>& input_action,
		 const ioa::aid_t binder) {
  ioa::bid_t bid = ioa::system::bind (output_action, input_action, binder, 0);
  assert (bid != -1);
  return bid;
}

void destroy (const ioa::aid_t automaton)
{
  assert (ioa::system::destroy (automaton) == true);
}

static const char*
creator_dne ()
{
  // Reset the system.
  ioa::system::clear ();

  ioa::aid_t creator_handle = ioa::system::create (ioa::make_generator<automaton1> ());
  mu_assert (creator_handle != -1);
  destroy (creator_handle);
  ioa::aid_t handle = ioa::system::create (creator_handle, ioa::make_generator<automaton1> (), 0);
  mu_assert (handle == -1);

  return 0;
}

static const char*
instance_exists ()
{
  // Reset the system.
  ioa::system::clear ();

  automaton1* instance1 = new automaton1 ();
  std::auto_ptr<ioa::generator_interface> holder1 (new instance_holder<automaton1> (instance1));
  automaton1* instance2 = new automaton1 ();
  std::auto_ptr<ioa::generator_interface> holder2 (new instance_holder<automaton1> (instance2));
  std::auto_ptr<ioa::generator_interface> holder3 (new instance_holder<automaton1> (instance2));

  ioa::aid_t creator = ioa::system::create (holder1);
  mu_assert (creator != -1);

  ioa::aid_t h1 = ioa::system::create (creator, holder2, 0);
  mu_assert (h1 != -1);
  // TODO:  Enable this check.
  //mu_assert (instance1->created_handle == h1);

  ioa::automaton_handle<automaton1> h2 = ioa::system::create (creator, holder3, 0);
  mu_assert (-1 == h2);
  // TODO:  Enable this check.
  //mu_assert (instance1->existing_instance == instance2);

  return 0;
}

static const char*
automaton_created ()
{
  // Reset the system.
  ioa::system::clear ();

  automaton1* instance = new automaton1 ();
  std::auto_ptr<ioa::generator_interface> holder (new instance_holder<automaton1> (instance));

  ioa::automaton_handle<automaton1> handle = ioa::system::create (holder);
  mu_assert (handle != -1);
  // TODO:  Enable this check.
  //mu_assert (instance->inited);

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

  mu_assert (ioa::system::bind (ioa::make_action (output, &automaton1::up_uv_output),
				ioa::make_action (input, &automaton1::up_uv_input),
				binder, 0) == -1);

  return 0;
}

static const char*
output_automaton_dne ()
{
  // Reset the system.
  ioa::system::clear ();

  automaton1* instance = new automaton1 ();
  std::auto_ptr<ioa::generator_interface> holder (new instance_holder<automaton1> (instance));
  
  ioa::aid_t binder = create (holder);
  
  ioa::automaton_handle<automaton1> output = create (ioa::make_generator<automaton1> ());
  destroy (output);
  
  ioa::automaton_handle<automaton1> input = create (ioa::make_generator<automaton1> ());
  
  mu_assert (ioa::system::bind (ioa::make_action (output, &automaton1::up_uv_output),
				ioa::make_action (input, &automaton1::up_uv_input),
				binder,
				0) == -1);
  // TODO:  Enable this check.
  //mu_assert (instance->m_output_automaton_dne);

  return 0;
}

static const char*
input_automaton_dne ()
{
  // Reset the system.
  ioa::system::clear ();

  automaton1* instance = new automaton1 ();
  std::auto_ptr<ioa::generator_interface> holder (new instance_holder<automaton1> (instance));

  ioa::aid_t binder = create (holder);
  
  ioa::automaton_handle<automaton1> output = create (ioa::make_generator<automaton1> ());
  
  ioa::automaton_handle<automaton1> input = create (ioa::make_generator<automaton1> ());

  destroy (input);
  
  mu_assert (ioa::system::bind (ioa::make_action (output, &automaton1::up_uv_output),
				ioa::make_action (input, &automaton1::up_uv_input),
				binder,
				0) == -1);
  // TODO:  Enable this check.
  // mu_assert (instance->m_input_automaton_dne);
  
  return 0;
}

static const char*
binding_exists ()
{
  // Reset the system.
  ioa::system::clear ();

  automaton1* instance = new automaton1 ();
  std::auto_ptr<ioa::generator_interface> holder (new instance_holder<automaton1> (instance));

  ioa::automaton_handle<automaton1> binder = create (holder);
  
  ioa::automaton_handle<automaton1> output = create (ioa::make_generator<automaton1> ());
  
  ioa::automaton_handle<automaton1> input = create (ioa::make_generator<automaton1> ());
  
  
  ioa::bid_t bid = ioa::system::bind (ioa::make_action (output, &automaton1::up_uv_output),
				      ioa::make_action (input, &automaton1::up_uv_input),
				      binder,
				      0);
  mu_assert (bid != -1);
  mu_assert (instance->m_bid == bid);
  
  mu_assert (ioa::system::bind (ioa::make_action (output, &automaton1::up_uv_output),
				ioa::make_action (input, &automaton1::up_uv_input),
				binder,
				0) == -1);
  // TODO:  Enable this check.
  //mu_assert (instance->m_binding_exists);

  return 0;
}

static const char*
input_action_unavailable ()
{
  // Reset the system.
  ioa::system::clear ();

  automaton1* instance = new automaton1 ();
  std::auto_ptr<ioa::generator_interface> holder (new instance_holder<automaton1> (instance));

  ioa::automaton_handle<automaton1> binder = create (holder);
  
  ioa::automaton_handle<automaton1> output1 = create (ioa::make_generator<automaton1> ());

  ioa::automaton_handle<automaton1> output2 = create (ioa::make_generator<automaton1> ());
  
  ioa::automaton_handle<automaton1> input = create (ioa::make_generator<automaton1> ());

  
  mu_assert (ioa::system::bind (ioa::make_action (output1, &automaton1::up_uv_output),
				ioa::make_action (input, &automaton1::up_uv_input),
				binder,
				0) != -1);

  mu_assert (ioa::system::bind (ioa::make_action (output2, &automaton1::up_uv_output),
				ioa::make_action (input, &automaton1::up_uv_input),
				binder,
				0) == -1);
  // TODO:  Enable this check.
  //mu_assert (instance->m_input_action_unavailable);

  return 0;
}

static const char*
output_action_unavailable ()
{
  // Reset the system.
  ioa::system::clear ();
  
  int parameter = 0;

  automaton1* instance1 = new automaton1 ();
  std::auto_ptr<ioa::generator_interface> holder1 (new instance_holder<automaton1> (instance1));

  automaton1* instance2 = new automaton1 ();
  std::auto_ptr<ioa::generator_interface> holder2 (new instance_holder<automaton1> (instance2));

  ioa::automaton_handle<automaton1> binder = create (holder1);
  
  ioa::automaton_handle<automaton1> output = create (ioa::make_generator<automaton1> ());

  ioa::automaton_handle<automaton1> input = create (holder2);

  
  mu_assert (ioa::system::bind (ioa::make_action (output, &automaton1::up_uv_output),
				ioa::make_action (input, &automaton1::up_uv_input),
				binder,
				0) != -1);

  mu_assert (ioa::system::bind (ioa::make_action (output, &automaton1::up_uv_output),
				ioa::make_action (input, &automaton1::p_uv_input, parameter),
				input,
				0) == -1);
  // TODO:  Enable this check.
  //mu_assert (instance2->m_output_action_unavailable);

  return 0;
}

static const char*
bound ()
{
  // Reset the system.
  ioa::system::clear ();
  
  int parameter = 0;

  automaton1* output_instance = new automaton1 ();
  std::auto_ptr<ioa::generator_interface> output_holder (new instance_holder<automaton1> (output_instance));

  automaton1* input_instance = new automaton1 ();
  std::auto_ptr<ioa::generator_interface> input_holder (new instance_holder<automaton1> (input_instance));

  ioa::automaton_handle<automaton1> output = create (output_holder);

  ioa::automaton_handle<automaton1> input = create (input_holder);

  
  mu_assert (ioa::system::bind (ioa::make_action (output, &automaton1::p_uv_output, parameter),
				ioa::make_action (input, &automaton1::up_uv_input),
				output,
				0) != -1);

  ioa::system::execute (ioa::make_action (output, &automaton1::p_uv_output, parameter));
  mu_assert (input_instance->up_uv_input.state);

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
  
  ioa::bid_t bid = bind (ioa::make_action (output, &automaton1::up_uv_output),
			 ioa::make_action (input, &automaton1::up_uv_input),
			 binder);
  
  destroy (binder);
  
  mu_assert (ioa::system::unbind (bid,
				  binder,
				  0) == false);

  return 0;
}

static const char*
binding_dne ()
{
  // Reset the system.
  ioa::system::clear ();

  automaton1* instance = new automaton1 ();
  std::auto_ptr<ioa::generator_interface> holder (new instance_holder<automaton1> (instance));

  ioa::automaton_handle<automaton1> binder = create (holder);
  
  ioa::automaton_handle<automaton1> output = create (ioa::make_generator<automaton1> ());

  ioa::automaton_handle<automaton1> input = create (ioa::make_generator<automaton1> ());

  ioa::bid_t bid = bind (ioa::make_action (output, &automaton1::up_uv_output),
			 ioa::make_action (input, &automaton1::up_uv_input),
			 binder);
  
  
  mu_assert (ioa::system::unbind (bid,
				  binder,
				  0) == true);
  // TODO:  Enable this check.
  //mu_assert (instance->m_unbound);
    
  mu_assert (ioa::system::unbind (bid,
				  binder,
				  0) == false);
  // TODO:  Enable this check.
  // mu_assert (instance->m_binding_dne);

  return 0;
}

static const char*
unbound ()
{
  // Reset the system.
  ioa::system::clear ();
  
  int parameter = 0;

  automaton1* instance = new automaton1 ();
  std::auto_ptr<ioa::generator_interface> holder (new instance_holder<automaton1> (instance));

  ioa::automaton_handle<automaton1> output = create (holder);

  ioa::automaton_handle<automaton1> input = create (ioa::make_generator<automaton1> ());

  ioa::bid_t bid = bind (ioa::make_action (output, &automaton1::p_uv_output, parameter),
			 ioa::make_action (input, &automaton1::up_uv_input),
			 output);
  
  
  mu_assert (ioa::system::unbind (bid,
				  output,
				  0) == true);
  // TODO:  Enable this check.
  //mu_assert (instance->m_unbound);

  return 0;
}

static const char*
destroyer_dne ()
{
  // Reset the system.
  ioa::system::clear ();
  
  ioa::automaton_handle<automaton1> parent_handle = create (ioa::make_generator<automaton1> ());
  ioa::automaton_handle<automaton1> child_handle = create (parent_handle, ioa::make_generator<automaton1> ());

  destroy (parent_handle);
  
  mu_assert (ioa::system::destroy (parent_handle, child_handle, 0) == false);

  return 0;
}

static const char*
destroyer_not_creator ()
{
  // Reset the system.
  ioa::system::clear ();
  
  automaton1* instance = new automaton1 ();
  std::auto_ptr<ioa::generator_interface> holder (new instance_holder<automaton1> (instance));
  
  ioa::automaton_handle<automaton1> parent_handle = create (ioa::make_generator<automaton1> ());
  
  ioa::automaton_handle<automaton1> child_handle = create (parent_handle, ioa::make_generator<automaton1> ());
  
  ioa::automaton_handle<automaton1> third_party_handle = create (holder);
  
  mu_assert (ioa::system::destroy (third_party_handle, child_handle, 0) == false);
  // TODO:  Enable this check.
  //mu_assert (instance->m_destroyer_not_creator);

  return 0;
}

static const char*
target_automaton_dne ()
{
  // Reset the system.
  ioa::system::clear ();

  automaton1* instance = new automaton1 ();
  std::auto_ptr<ioa::generator_interface> holder (new instance_holder<automaton1> (instance));

  ioa::automaton_handle<automaton1> parent_handle = create (holder);
  
  ioa::automaton_handle<automaton1> child_handle = create (parent_handle, ioa::make_generator<automaton1> ());
  
  mu_assert (ioa::system::destroy (parent_handle, child_handle, 0) == true);
  mu_assert (ioa::system::destroy (parent_handle, child_handle, 0) == false);
  // TODO:  Enable this check.
  //mu_assert (instance->m_target_automaton_dne);

  return 0;
}

static const char*
automaton_destroyed ()
{
  // Reset the system.
  ioa::system::clear ();
  
  int parameter = 0;

  automaton1* instance = new automaton1 ();
  std::auto_ptr<ioa::generator_interface> holder (new instance_holder<automaton1> (instance));

  ioa::automaton_handle<automaton1> alpha = create (holder);

  ioa::automaton_handle<automaton1> beta = create (alpha, ioa::make_generator<automaton1> ());

  ioa::automaton_handle<automaton1> gamma = create (beta, ioa::make_generator<automaton1> ());

  bind (ioa::make_action (alpha, &automaton1::up_uv_output),
	ioa::make_action (beta, &automaton1::up_uv_input),
	alpha);
  bind (ioa::make_action (beta, &automaton1::p_uv_output, parameter),
	ioa::make_action (gamma, &automaton1::up_uv_input),
	beta);
  bind (ioa::make_action (gamma, &automaton1::up_uv_output),
	ioa::make_action (beta, &automaton1::p_uv_input, parameter),
	beta);

  mu_assert (ioa::system::destroy (beta) == true);
  // TODO:  Enable this check.
  //mu_assert (instance->m_automaton_destroyed);
  // TODO:  Enable this check.
  //mu_assert (instance->m_unbound);
  
  return 0;
}

static const char*
execute_output_automaton_dne ()
{
  // Reset the system.
  ioa::system::clear ();

  ioa::automaton_handle<automaton1> output = create (ioa::make_generator<automaton1> ());

  destroy (output);

  mu_assert (ioa::system::execute (ioa::make_action (output, &automaton1::up_uv_output)) == false);

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

  mu_assert (ioa::system::execute (ioa::make_action (output, &automaton1::up_uv_output)) == true);
  mu_assert (output_instance->up_uv_output.state);

  return 0;
}

static const char*
execute_event_automaton_dne ()
{
  // Reset the system.
  ioa::system::clear ();

  ioa::automaton_handle<automaton1> from = create (ioa::make_generator<automaton1> ());
  ioa::automaton_handle<automaton1> to = create (ioa::make_generator<automaton1> ());

  destroy (from);
  
  mu_assert (ioa::system::execute (from, ioa::make_action (to, &automaton1::uv_event), 0) == false);

  return 0;
}

static const char*
recipient_dne ()
{
  // Reset the system.
  ioa::system::clear ();

  automaton1* instance = new automaton1 ();
  std::auto_ptr<ioa::generator_interface> holder (new instance_holder<automaton1> (instance));

  ioa::automaton_handle<automaton1> from = create (holder);
  ioa::automaton_handle<automaton1> to = create (ioa::make_generator<automaton1> ());

  destroy (to);
  
  mu_assert (ioa::system::execute (from, ioa::make_action (to, &automaton1::uv_event), 0) == false);
  // TODO:  Enable this check.
  //mu_assert (instance->m_recipient_dne);

  return 0;
}

static const char*
execute_event ()
{
  // Reset the system.
  ioa::system::clear ();
  
  automaton1* event_instance = new automaton1 ();
  std::auto_ptr<ioa::generator_interface> holder (new instance_holder<automaton1> (event_instance));

  ioa::automaton_handle<automaton1> from = create (ioa::make_generator<automaton1> ());
  ioa::automaton_handle<automaton1> to = create (holder);
  
  mu_assert (ioa::system::execute (from, ioa::make_action (to, &automaton1::uv_event), 0) == true);
  mu_assert (event_instance->uv_event.state);

  return 0;
}

const char*
all_tests ()
{
  mu_run_test (creator_dne);
  mu_run_test (instance_exists);
  mu_run_test (automaton_created);
  mu_run_test (binder_dne);
  mu_run_test (output_automaton_dne);
  mu_run_test (input_automaton_dne);
  mu_run_test (binding_exists);
  mu_run_test (input_action_unavailable);
  mu_run_test (output_action_unavailable);
  mu_run_test (bound);
  mu_run_test (unbinder_dne);
  mu_run_test (binding_dne);
  mu_run_test (unbound);
  mu_run_test (destroyer_dne);
  mu_run_test (destroyer_not_creator);
  mu_run_test (target_automaton_dne);
  mu_run_test (automaton_destroyed);
  mu_run_test (execute_output_automaton_dne);
  mu_run_test (execute_output);
  mu_run_test (execute_event_automaton_dne);
  mu_run_test (recipient_dne);
  mu_run_test (execute_event);

  return 0;
}
