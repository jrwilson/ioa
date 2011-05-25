#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE system
#include <boost/test/unit_test.hpp>

#include <system.hpp>
#include "automaton1.hpp"
#include "instance_holder.hpp"

class empty_class
{
};

class dummy_scheduler :
  public ioa::scheduler_interface
{
public:
  void set_current_aid (const ioa::aid_t) { }
  void set_current_aid (const ioa::aid_t,
			const ioa::automaton_interface*) { }
  void clear_current_aid (void) { }
};

template <class G>
ioa::automaton_handle<typename G::result_type> create (ioa::system& system,
						       G generator,
						       ioa::scheduler_interface& s) {
  ioa::automaton_handle<typename G::result_type> handle = system.create (generator, s);
  BOOST_CHECK (handle.aid () != -1);
  return handle;
}

template <class P, class G>
ioa::automaton_handle<typename G::result_type> create (ioa::system& system,
						       const ioa::automaton_handle<P>& creator,
						       G generator,
						       ioa::scheduler_interface& s) {
  empty_class d;
  ioa::automaton_handle<typename G::result_type> handle = system.create (creator, generator, s, d);
  BOOST_CHECK (handle.aid () != -1);
  return handle;
}

template <class I, class T>
ioa::parameter_handle<T> declare (ioa::system& system,
				  const ioa::automaton_handle<I>& automaton,
				  T* parameter,
				  ioa::scheduler_interface& s) {
  empty_class d;
  ioa::parameter_handle<T> handle = system.declare (automaton, parameter, s, d);
  BOOST_CHECK (handle.pid () != -1);
  return handle;
}

template <class OI, class OM, class II, class IM, class I>
ioa::bid_t bind (ioa::system& system,
		 const ioa::action<OI, OM>& output_action,
		 const ioa::action<II, IM>& input_action,
		 const ioa::automaton_handle<I>& binder,
		 ioa::scheduler_interface& s) {
  empty_class d;
  ioa::bid_t bid = system.bind (output_action, input_action, binder, s, d);
  BOOST_CHECK (bid != -1);
  return bid;
}

template <class OI, class OM, class II, class IM>
ioa::bid_t bind (ioa::system& system,
		 const ioa::action<OI, OM>& output_action,
		 const ioa::action<II, IM>& input_action,
	    ioa::scheduler_interface& s) {
  empty_class d;
  ioa::bid_t bid = system.bind (output_action, input_action, s, d);
  BOOST_CHECK (bid != -1);
  return bid;
}

template <class I, class P>
void rescind (ioa::system& system,
	      const ioa::automaton_handle<I>& automaton,
	      const ioa::parameter_handle<P>& parameter,
	      ioa::scheduler_interface& s) {
  empty_class d;
  BOOST_CHECK (system.rescind (automaton, parameter, s, d) == true);
}

template <class I>
void destroy (ioa::system& system,
	      const ioa::automaton_handle<I>& automaton)
{
  BOOST_CHECK (system.destroy (automaton) == true);
}

BOOST_AUTO_TEST_SUITE(system_suite)

BOOST_AUTO_TEST_CASE (system_create_creator_dne)
{
  ioa::system system;
  dummy_scheduler s;
  ioa::automaton_handle<automaton1> creator_handle = system.create (automaton1_generator (), s);
  BOOST_CHECK (creator_handle.aid () != -1);
  destroy (system, creator_handle);
  empty_class d;
  ioa::automaton_handle<automaton1> handle = system.create (creator_handle, automaton1_generator (), s, d);
  BOOST_CHECK (handle.aid () == -1);
}

BOOST_AUTO_TEST_CASE (system_create_exists)
{
  // Scheduler must outlive system.
  dummy_scheduler s;
  ioa::system system;

  automaton1* instance1 = new automaton1 ();
  instance_holder<automaton1> holder1 (instance1);
  automaton1* instance2 = new automaton1 ();
  instance_holder<automaton1> holder2 (instance2);

  ioa::automaton_handle<automaton1> creator = system.create (holder1, s);
  BOOST_CHECK (creator.aid () != -1);

  empty_class d;
  ioa::automaton_handle<automaton1> h1 = system.create (creator, holder2, s, d);
  BOOST_CHECK (h1.aid () != -1);
  BOOST_CHECK (instance1->created_handle == h1);

  ioa::automaton_handle<automaton1> h2 = system.create (creator, holder2, s, d);
  BOOST_CHECK (h2.aid () == -1);
  BOOST_CHECK (instance1->existing_instance == instance2);
}

BOOST_AUTO_TEST_CASE (system_create_success)
{
  ioa::system system;
  automaton1* instance = new automaton1 ();
  instance_holder<automaton1> holder (instance);
  dummy_scheduler s;
  ioa::automaton_handle<automaton1> handle = system.create (holder, s);
  BOOST_CHECK (handle.aid () != -1);
  BOOST_CHECK (instance->inited);
}

BOOST_AUTO_TEST_CASE (system_declare_automaton_dne)
{
  ioa::system system;
  dummy_scheduler s;
  int parameter;
  
  ioa::automaton_handle<automaton1> handle = create (system, automaton1_generator (), s);
  destroy (system, handle);
  empty_class d;
  ioa::parameter_handle<int> p = system.declare (handle, &parameter, s, d);
  BOOST_CHECK (p.pid () == -1);
}

BOOST_AUTO_TEST_CASE (system_declare_exists)
{
  dummy_scheduler s;
  ioa::system system;
  int parameter;

  automaton1* instance = new automaton1 ();
  instance_holder<automaton1> holder (instance);
  
  ioa::automaton_handle<automaton1> handle = create (system, holder, s);
  empty_class d;
  ioa::parameter_handle<int> p = system.declare (handle, &parameter, s, d);
  BOOST_CHECK (p.pid () != -1);
  p = system.declare (handle, &parameter, s, d);
  BOOST_CHECK (p.pid () == -1);
  BOOST_CHECK (instance->parameter_existed);
}

BOOST_AUTO_TEST_CASE (system_declare_success)
{
  dummy_scheduler s;
  ioa::system system;
  int parameter;

  automaton1* instance = new automaton1 ();
  instance_holder<automaton1> holder (instance);

  ioa::automaton_handle<automaton1> handle = create (system, holder, s);
  empty_class d;
  ioa::parameter_handle<int> p = system.declare (handle, &parameter, s, d);
  BOOST_CHECK (p.pid () != -1);
  BOOST_CHECK (instance->declared_handle == p);
}

BOOST_AUTO_TEST_CASE (system_bind_binder_automaton_dne)
{
  ioa::system system;
  dummy_scheduler s;

  ioa::automaton_handle<automaton1> binder = create (system, automaton1_generator (), s);
  destroy (system, binder);
  
  ioa::automaton_handle<automaton1> output = create (system, automaton1_generator (), s);
  
  ioa::automaton_handle<automaton1> input = create (system, automaton1_generator (), s);

  empty_class d;
  BOOST_CHECK (system.bind (ioa::make_action (output, &automaton1::up_uv_output),
			    ioa::make_action (input, &automaton1::up_uv_input),
			    binder,
			    s, d) == -1);
}

BOOST_AUTO_TEST_CASE (system_bind_output_automaton_dne)
{
  ioa::system system;
  dummy_scheduler s;

  automaton1* instance = new automaton1 ();
  instance_holder<automaton1> holder (instance);
  
  ioa::automaton_handle<automaton1> binder = create (system, holder, s);
  
  ioa::automaton_handle<automaton1> output = create (system, automaton1_generator (), s);
  destroy (system, output);
  
  ioa::automaton_handle<automaton1> input = create (system, automaton1_generator (), s);
  
  empty_class d;
  BOOST_CHECK (system.bind (ioa::make_action (output, &automaton1::up_uv_output),
			    ioa::make_action (input, &automaton1::up_uv_input),
			    binder,
			    s, d) == -1);
  BOOST_CHECK (instance->m_output_automaton_dne);
}

BOOST_AUTO_TEST_CASE (system_bind_input_automaton_dne)
{
  ioa::system system;
  dummy_scheduler s;

  automaton1* instance = new automaton1 ();
  instance_holder<automaton1> holder (instance);

  ioa::automaton_handle<automaton1> binder = create (system, holder, s);
  
  ioa::automaton_handle<automaton1> output = create (system, automaton1_generator (), s);
  
  ioa::automaton_handle<automaton1> input = create (system, automaton1_generator (), s);

  destroy (system, input);
  
  empty_class d;
  BOOST_CHECK (system.bind (ioa::make_action (output, &automaton1::up_uv_output),
			    ioa::make_action (input, &automaton1::up_uv_input),
			    binder,
			    s, d) == -1);
  BOOST_CHECK (instance->m_input_automaton_dne);
}

BOOST_AUTO_TEST_CASE (system_bind_output_parameter_dne)
{
  ioa::system system;
  dummy_scheduler s;
  int parameter;

  automaton1* instance = new automaton1 ();
  instance_holder<automaton1> holder (instance);

  ioa::automaton_handle<automaton1> output = create (system, holder, s);
  
  ioa::automaton_handle<automaton1> input = create (system, automaton1_generator (), s);

  ioa::parameter_handle<int> param = declare (system, output, &parameter, s);

  rescind (system, output, param, s);

  empty_class d;
  BOOST_CHECK (system.bind (ioa::make_action (output, &automaton1::p_uv_output, param),
			    ioa::make_action (input, &automaton1::up_uv_input),
			    s, d) == -1);
  BOOST_CHECK (instance->m_output_parameter_dne);
}

BOOST_AUTO_TEST_CASE (system_bind_input_parameter_dne)
{
  ioa::system system;
  dummy_scheduler s;
  int parameter;

  automaton1* instance = new automaton1 ();
  instance_holder<automaton1> holder (instance);

  ioa::automaton_handle<automaton1> output = create (system, automaton1_generator (), s);
  
  ioa::automaton_handle<automaton1> input = create (system, holder, s);

  ioa::parameter_handle<int> param = declare (system, input, &parameter, s);

  rescind (system, input, param, s);

  empty_class d;
  BOOST_CHECK (system.bind (ioa::make_action (output, &automaton1::up_uv_output),
			    ioa::make_action (input, &automaton1::p_uv_input, param),
			    s, d) == -1);
  BOOST_CHECK (instance->m_input_parameter_dne);
}

BOOST_AUTO_TEST_CASE (system_bind_exists)
{
  dummy_scheduler s;
  ioa::system system;

  automaton1* instance = new automaton1 ();
  instance_holder<automaton1> holder (instance);

  ioa::automaton_handle<automaton1> binder = create (system, holder, s);
  
  ioa::automaton_handle<automaton1> output = create (system, automaton1_generator (), s);
  
  ioa::automaton_handle<automaton1> input = create (system, automaton1_generator (), s);
  
  empty_class d;
  ioa::bid_t bid = system.bind (ioa::make_action (output, &automaton1::up_uv_output),
				ioa::make_action (input, &automaton1::up_uv_input),
				binder,
				s, d);
  BOOST_CHECK (bid != -1);
  BOOST_CHECK_EQUAL (instance->m_bid, bid);

  BOOST_CHECK (system.bind (ioa::make_action (output, &automaton1::up_uv_output),
			    ioa::make_action (input, &automaton1::up_uv_input),
			    binder,
			    s, d) == -1);
  BOOST_CHECK (instance->m_binding_exists);
}

BOOST_AUTO_TEST_CASE (system_bind_input_action_unavailable)
{
  dummy_scheduler s;
  ioa::system system;

  automaton1* instance = new automaton1 ();
  instance_holder<automaton1> holder (instance);

  ioa::automaton_handle<automaton1> binder = create (system, holder, s);
  
  ioa::automaton_handle<automaton1> output1 = create (system, automaton1_generator (), s);

  ioa::automaton_handle<automaton1> output2 = create (system, automaton1_generator (), s);
  
  ioa::automaton_handle<automaton1> input = create (system, automaton1_generator (), s);

  empty_class d;
  BOOST_CHECK (system.bind (ioa::make_action (output1, &automaton1::up_uv_output),
			    ioa::make_action (input, &automaton1::up_uv_input),
			    binder,
			    s, d) != -1);

  BOOST_CHECK (system.bind (ioa::make_action (output2, &automaton1::up_uv_output),
			    ioa::make_action (input, &automaton1::up_uv_input),
			    binder,
			    s, d) == -1);
  BOOST_CHECK (instance->m_input_action_unavailable);
}

BOOST_AUTO_TEST_CASE (system_bind_output_action_unavailable)
{
  dummy_scheduler s;
  ioa::system system;
  int parameter;

  automaton1* instance1 = new automaton1 ();
  instance_holder<automaton1> holder1 (instance1);

  automaton1* instance2 = new automaton1 ();
  instance_holder<automaton1> holder2 (instance2);

  ioa::automaton_handle<automaton1> binder = create (system, holder1, s);
  
  ioa::automaton_handle<automaton1> output = create (system, automaton1_generator (), s);

  ioa::automaton_handle<automaton1> input = create (system, holder2, s);

  ioa::parameter_handle<int> param = declare (system, input, &parameter, s);

  empty_class d;
  BOOST_CHECK (system.bind (ioa::make_action (output, &automaton1::up_uv_output),
			    ioa::make_action (input, &automaton1::up_uv_input),
			    binder,
			    s, d) != -1);

  BOOST_CHECK (system.bind (ioa::make_action (output, &automaton1::up_uv_output),
			    ioa::make_action (input, &automaton1::p_uv_input, param),
			    s, d) == -1);
  BOOST_CHECK (instance2->m_output_action_unavailable);
}

BOOST_AUTO_TEST_CASE (system_bind_success)
{
  dummy_scheduler s;
  ioa::system system;
  int parameter;

  automaton1* output_instance = new automaton1 ();
  instance_holder<automaton1> output_holder (output_instance);

  automaton1* input_instance = new automaton1 ();
  instance_holder<automaton1> input_holder (input_instance);

  ioa::automaton_handle<automaton1> output = create (system, output_holder, s);

  ioa::automaton_handle<automaton1> input = create (system, input_holder, s);

  ioa::parameter_handle<int> param = declare (system, output, &parameter, s);

  empty_class d;
  BOOST_CHECK (system.bind (ioa::make_action (output, &automaton1::p_uv_output, param),
			    ioa::make_action (input, &automaton1::up_uv_input),
			    s, d) != -1);

  system.execute (ioa::make_action (output, &automaton1::p_uv_output, param), s);
  BOOST_CHECK (input_instance->up_uv_input.state);
}

BOOST_AUTO_TEST_CASE (system_unbind_binder_automaton_dne)
{
  ioa::system system;
  dummy_scheduler s;
  
  ioa::automaton_handle<automaton1> binder = create (system, automaton1_generator (), s);
  
  ioa::automaton_handle<automaton1> output = create (system, automaton1_generator (), s);

  ioa::automaton_handle<automaton1> input = create (system, automaton1_generator (), s);
  
  ioa::bid_t bid = bind (system,
			 ioa::make_action (output, &automaton1::up_uv_output),
			 ioa::make_action (input, &automaton1::up_uv_input),
			 binder,
			 s);

  destroy (system, binder);

  empty_class d;
  BOOST_CHECK (system.unbind (bid,
			      binder,
			      s,
			      d) == false);
}

BOOST_AUTO_TEST_CASE (system_unbind_exists)
{
  dummy_scheduler s;
  ioa::system system;

  automaton1* instance = new automaton1 ();
  instance_holder<automaton1> holder (instance);

  ioa::automaton_handle<automaton1> binder = create (system, holder, s);
  
  ioa::automaton_handle<automaton1> output = create (system, automaton1_generator (), s);

  ioa::automaton_handle<automaton1> input = create (system, automaton1_generator (), s);

  ioa::bid_t bid = bind (system,
			 ioa::make_action (output, &automaton1::up_uv_output),
			 ioa::make_action (input, &automaton1::up_uv_input),
			 binder,
			 s);
  
  empty_class d;
  BOOST_CHECK (system.unbind (bid,
			      binder,
			      s,
			      d) == true);
  BOOST_CHECK (instance->m_unbound);
  

  BOOST_CHECK (system.unbind (bid,
			      binder,
			      s,
			      d) == false);
  BOOST_CHECK (instance->m_binding_dne);
}

BOOST_AUTO_TEST_CASE (system_unbind_success)
{
  dummy_scheduler s;
  ioa::system system;
  int parameter;

  automaton1* instance = new automaton1 ();
  instance_holder<automaton1> holder (instance);

  ioa::automaton_handle<automaton1> output = create (system, holder, s);

  ioa::automaton_handle<automaton1> input = create (system, automaton1_generator (), s);

  ioa::parameter_handle<int> param = declare (system, output, &parameter, s);

  ioa::bid_t bid = bind (system,
			 ioa::make_action (output, &automaton1::p_uv_output, param),
			 ioa::make_action (input, &automaton1::up_uv_input),
			 s);
  
  empty_class d;
  BOOST_CHECK (system.unbind (bid,
			      output,
			      s, d) == true);
  BOOST_CHECK (instance->m_unbound);
}

BOOST_AUTO_TEST_CASE (system_rescind_automaton_dne)
{
  dummy_scheduler s;
  ioa::system system;
  int parameter;
  
  ioa::automaton_handle<automaton1> automaton = create (system, automaton1_generator (), s);

  ioa::parameter_handle<int> param = declare (system, automaton, &parameter, s);

  destroy (system, automaton);

  empty_class d;
  BOOST_CHECK (system.rescind (automaton, param, s, d) == false);
}

BOOST_AUTO_TEST_CASE (system_rescind_exists)
{
  dummy_scheduler s;
  ioa::system system;
  int parameter;

  automaton1* instance = new automaton1 ();
  instance_holder<automaton1> holder (instance);
  
  ioa::automaton_handle<automaton1> automaton = create (system, holder, s);

  ioa::parameter_handle<int> param = declare (system, automaton, &parameter, s);

  empty_class d;
  BOOST_CHECK (system.rescind (automaton, param, s, d) == true);

  BOOST_CHECK (system.rescind (automaton, param, s, d) == false);
  BOOST_CHECK (instance->m_parameter_dne);
}

BOOST_AUTO_TEST_CASE (system_rescind_success)
{
  dummy_scheduler s;
  ioa::system system;
  int parameter;

  automaton1* instance = new automaton1 ();
  instance_holder<automaton1> holder (instance);

  ioa::automaton_handle<automaton1> binder = create (system, holder, s);
  
  ioa::automaton_handle<automaton1> output = create (system, automaton1_generator (), s);

  ioa::automaton_handle<automaton1> input = create (system, automaton1_generator (), s);

  ioa::parameter_handle<int> param = declare (system, binder, &parameter, s);

  bind (system,
	ioa::make_action (binder, &automaton1::p_uv_output, param),
	ioa::make_action (input, &automaton1::up_uv_input),
	s);

  bind (system,
	ioa::make_action (output, &automaton1::up_uv_output),
	ioa::make_action (binder, &automaton1::p_uv_input, param),
	s);

  empty_class d;
  BOOST_CHECK (system.rescind (binder, param, s, d) == true);
  BOOST_CHECK (instance->m_parameter_rescinded);
  BOOST_CHECK (instance->m_unbound);
}

BOOST_AUTO_TEST_CASE (system_destroy_destroyer_dne)
{
  dummy_scheduler s;
  ioa::system system;
  ioa::automaton_handle<automaton1> parent_handle = create (system, automaton1_generator (), s);
  ioa::automaton_handle<automaton1> child_handle = create (system, parent_handle, automaton1_generator (), s);

  destroy (system, parent_handle);
  
  empty_class d;
  BOOST_CHECK (system.destroy (parent_handle, child_handle, s, d) == false);
}

BOOST_AUTO_TEST_CASE (system_destroy_destroyer_not_creator)
{
  dummy_scheduler s;
  ioa::system system;

  automaton1* instance = new automaton1 ();
  instance_holder<automaton1> holder (instance);
  
  ioa::automaton_handle<automaton1> parent_handle = create (system, automaton1_generator (), s);
  
  ioa::automaton_handle<automaton1> child_handle = create (system, parent_handle, automaton1_generator (), s);

  ioa::automaton_handle<automaton1> third_party_handle = create (system, holder, s);

   empty_class d;
   BOOST_CHECK (system.destroy (third_party_handle, child_handle, s, d) == false);
   BOOST_CHECK (instance->m_destroyer_not_creator);
}

BOOST_AUTO_TEST_CASE (system_destroy_exists)
{
  dummy_scheduler s;
  ioa::system system;

  automaton1* instance = new automaton1 ();
  instance_holder<automaton1> holder (instance);

  ioa::automaton_handle<automaton1> parent_handle = create (system, holder, s);
  
  ioa::automaton_handle<automaton1> child_handle = create (system, parent_handle, automaton1_generator (), s);

  empty_class d;
  BOOST_CHECK (system.destroy (parent_handle, child_handle, s, d) == true);
  BOOST_CHECK (system.destroy (parent_handle, child_handle, s, d) == false);
  BOOST_CHECK (instance->m_target_automaton_dne);
}

BOOST_AUTO_TEST_CASE (system_destroy_success)
{
  dummy_scheduler s;
  ioa::system system;
  int parameter;

  automaton1* instance = new automaton1 ();
  instance_holder<automaton1> holder (instance);

  ioa::automaton_handle<automaton1> alpha = create (system, holder, s);

  ioa::automaton_handle<automaton1> beta = create (system, alpha, automaton1_generator (), s);

  ioa::automaton_handle<automaton1> gamma = create (system, beta, automaton1_generator (), s);

  ioa::parameter_handle<int> param = declare (system, beta, &parameter, s);

  bind (system,
	ioa::make_action (alpha, &automaton1::up_uv_output),
	ioa::make_action (beta, &automaton1::up_uv_input),
	alpha,
	s);
  bind (system,
	ioa::make_action (beta, &automaton1::p_uv_output, param),
	ioa::make_action (gamma, &automaton1::up_uv_input),
	s);
  bind (system,
	ioa::make_action (gamma, &automaton1::up_uv_output),
	ioa::make_action (beta, &automaton1::p_uv_input, param),
	s);

  BOOST_CHECK (system.destroy (beta) == true);
  BOOST_CHECK (instance->m_automaton_destroyed);
  BOOST_CHECK (instance->m_unbound);
}

BOOST_AUTO_TEST_CASE (system_execute_output_automaton_dne)
{
  dummy_scheduler s;
  ioa::system system;

  ioa::automaton_handle<automaton1> output = create (system, automaton1_generator (), s);

  destroy (system, output);

  BOOST_CHECK (system.execute (ioa::make_action (output, &automaton1::up_uv_output), s) == false);
}

BOOST_AUTO_TEST_CASE (system_execute_output_parameter_dne)
{
  dummy_scheduler s;
  ioa::system system;
  int parameter;

  ioa::automaton_handle<automaton1> output = create (system, automaton1_generator (), s);

  ioa::parameter_handle<int> param = declare (system, output, &parameter, s);

  rescind (system, output, param, s);

  BOOST_CHECK (system.execute (ioa::make_action (output, &automaton1::p_uv_output, param), s) == false);
}

BOOST_AUTO_TEST_CASE (system_execute_output_success)
{
  dummy_scheduler s;
  ioa::system system;
  automaton1* output_instance = new automaton1 ();
  instance_holder<automaton1> holder (output_instance);

  ioa::automaton_handle<automaton1> output = create (system, holder, s);

  BOOST_CHECK (system.execute (ioa::make_action (output, &automaton1::up_uv_output), s) == true);
  BOOST_CHECK (output_instance->up_uv_output.state);
}

BOOST_AUTO_TEST_CASE (system_deliver_event_automaton_dne)
{
  dummy_scheduler s;
  ioa::system system;

  ioa::automaton_handle<automaton1> handle = create (system, automaton1_generator (), s);

  destroy (system, handle);

  BOOST_CHECK (system.execute (ioa::make_action (handle, &automaton1::uv_event), s) == false);

  BOOST_CHECK (system.execute (ioa::make_action (handle, &automaton1::v_event, 9845), s) == false);
}

BOOST_AUTO_TEST_CASE (system_deliver_event_success)
{
  dummy_scheduler s;
  ioa::system system;

  automaton1* instance = new automaton1 ();
  instance_holder<automaton1> holder (instance);

  ioa::automaton_handle<automaton1> handle = create (system, holder, s);

  BOOST_CHECK (system.execute (ioa::make_action (handle, &automaton1::uv_event), s) == true);
  BOOST_CHECK (instance->uv_event.state);

  BOOST_CHECK (system.execute (ioa::make_action (handle, &automaton1::v_event, 9845), s) == true);
  BOOST_CHECK (instance->v_event.state);
  BOOST_CHECK_EQUAL (instance->v_event.last_value, 9845);
}

BOOST_AUTO_TEST_SUITE_END()
