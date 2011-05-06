#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE system
#include <boost/test/unit_test.hpp>

#include <system.hpp>
#include "automaton1.hpp"

class dummy_scheduler :
  public ioa::scheduler_interface
{
public:
  void set_current_handle (const ioa::generic_automaton_handle&) { }
  void clear_current_handle (void) { }
};

struct unbind_record
{
  ioa::generic_automaton_handle output_automaton;
  const void* output_member;
  ioa::generic_automaton_handle input_automaton;
  const void* input_member;
  ioa::generic_automaton_handle binder_automaton;

  unbind_record (const ioa::generic_automaton_handle& output_automaton,
		    const void* output_member,
		    const ioa::generic_automaton_handle& input_automaton,
		    const void* input_member,
		    const ioa::generic_automaton_handle& binder_automaton) :
    output_automaton (output_automaton),
    output_member (output_member),
    input_automaton (input_automaton),
    input_member (input_member),
    binder_automaton (binder_automaton)
  { }
  
  bool operator== (const unbind_record& odr) const {
    return output_automaton == odr.output_automaton &&
      output_member == odr.output_member &&
      input_automaton == odr.input_automaton &&
      input_member == odr.input_member &&
      binder_automaton == odr.binder_automaton;
  }
};

struct output_unbind_record
{
  ioa::generic_automaton_handle output_automaton;
  const void* output_member;
  ioa::generic_parameter_handle output_parameter;
  ioa::generic_automaton_handle input_automaton;
  const void* input_member;

  output_unbind_record (const ioa::generic_automaton_handle& output_automaton,
			   const void* output_member,
			   const ioa::generic_parameter_handle& output_parameter,
			   const ioa::generic_automaton_handle& input_automaton,
			   const void* input_member) :
    output_automaton (output_automaton),
    output_member (output_member),
    output_parameter (output_parameter),
    input_automaton (input_automaton),
    input_member (input_member)
  { }
  
  bool operator== (const output_unbind_record& odr) const {
    return output_automaton == odr.output_automaton &&
      output_member == odr.output_member &&
      output_parameter == odr.output_parameter &&
      input_automaton == odr.input_automaton &&
      input_member == odr.input_member;
  }
};

struct input_unbind_record
{
  ioa::generic_automaton_handle output_automaton;
  const void* output_member;
  ioa::generic_automaton_handle input_automaton;
  const void* input_member;
  ioa::generic_parameter_handle input_parameter;

  input_unbind_record (const ioa::generic_automaton_handle& output_automaton,
			  const void* output_member,
			  const ioa::generic_automaton_handle& input_automaton,
			  const void* input_member,
			  const ioa::generic_parameter_handle& input_parameter) :
    output_automaton (output_automaton),
    output_member (output_member),
    input_automaton (input_automaton),
    input_member (input_member),
    input_parameter (input_parameter)
  { }

  bool operator== (const input_unbind_record& odr) const {
    return output_automaton == odr.output_automaton &&
      output_member == odr.output_member &&
      input_automaton == odr.input_automaton &&
      input_member == odr.input_member &&
      input_parameter == odr.input_parameter;
  }
};

struct dummy_rescind_listener :
  public ioa::rescind_listener_interface
{
  std::vector<output_unbind_record> output_records;
  std::vector<input_unbind_record> input_records;

  void unbound (const ioa::generic_automaton_handle& output_automaton,
		const void* output_member,
		const ioa::generic_parameter_handle& output_parameter,
		const ioa::generic_automaton_handle& input_automaton,
		const void* input_member) {
    output_unbind_record odr (output_automaton, output_member, output_parameter, input_automaton, input_member);
    output_records.push_back (odr);
  }
  
  void unbound (const ioa::generic_automaton_handle& output_automaton,
		const void* output_member,
		const ioa::generic_automaton_handle& input_automaton,
		const void* input_member,
		const ioa::generic_parameter_handle& input_parameter) {
    input_unbind_record idr (output_automaton, output_member, input_automaton, input_member, input_parameter);
    input_records.push_back (idr);
  }
};

struct dummy_destroy_listener :
  public ioa::destroy_listener_interface
{
  std::vector<std::pair<ioa::generic_automaton_handle, ioa::generic_automaton_handle> > destroyed_records;
  std::vector<unbind_record> unbind_records;
  std::vector<output_unbind_record> output_records;
  std::vector<input_unbind_record> input_records;

  void destroyed (const ioa::generic_automaton_handle& parent,
		  const ioa::generic_automaton_handle& child) {
    destroyed_records.push_back (std::make_pair (parent, child));
  }

  void unbound (const ioa::generic_automaton_handle& output_automaton,
		const void* output_member,
		const ioa::generic_automaton_handle& input_automaton,
		const void* input_member,
		const ioa::generic_automaton_handle& binder_automaton) {
    unbind_record dr (output_automaton, output_member, input_automaton, input_member, binder_automaton);
    unbind_records.push_back (dr);
  }
  
  void unbound (const ioa::generic_automaton_handle& output_automaton,
		const void* output_member,
		const ioa::generic_parameter_handle& output_parameter,
		const ioa::generic_automaton_handle& input_automaton,
		const void* input_member) {
    output_unbind_record odr (output_automaton, output_member, output_parameter, input_automaton, input_member);
    output_records.push_back (odr);
  }

  void unbound (const ioa::generic_automaton_handle& output_automaton,
		const void* output_member,
		const ioa::generic_automaton_handle& input_automaton,
		const void* input_member,
		const ioa::generic_parameter_handle& input_parameter) {
    input_unbind_record idr (output_automaton, output_member, input_automaton, input_member, input_parameter);
    input_records.push_back (idr);
  }

};

BOOST_AUTO_TEST_SUITE(system_suite)

BOOST_AUTO_TEST_CASE (system_create_creator_dne)
{
  ioa::system system;
  ioa::system::create_result r1 = system.create (new automaton1 ());
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton1> creator_handle = ioa::cast_automaton<automaton1> (r1.automaton);
  dummy_destroy_listener listener;
  ioa::system::destroy_result d1 = system.destroy (creator_handle, listener);
  BOOST_CHECK_EQUAL (d1.type, ioa::system::DESTROY_SUCCESS);
  r1 = system.create (creator_handle, new automaton1 ());
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_CREATOR_DNE);
}

BOOST_AUTO_TEST_CASE (system_create_exists)
{
  ioa::system system;
  automaton1* a = new automaton1 ();
  ioa::system::create_result r1 = system.create (a);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  r1 = system.create (a);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_EXISTS);
}

BOOST_AUTO_TEST_CASE (system_create_success)
{
  ioa::system system;
  automaton1* a = new automaton1 ();
  ioa::system::create_result r1 = system.create (a);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
}

BOOST_AUTO_TEST_CASE (system_declare_automaton_dne)
{
  ioa::system system;
  int parameter;
  
  ioa::system::create_result r1 = system.create (new automaton1 ());
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton1> handle = ioa::cast_automaton<automaton1> (r1.automaton);
  dummy_destroy_listener listener;
  ioa::system::destroy_result d1 = system.destroy (handle, listener);
  BOOST_CHECK_EQUAL (d1.type, ioa::system::DESTROY_SUCCESS);
  ioa::system::declare_result x1 = system.declare (handle, &parameter);
  BOOST_CHECK_EQUAL (x1.type, ioa::system::DECLARE_AUTOMATON_DNE);
}

BOOST_AUTO_TEST_CASE (system_declare_exists)
{
  ioa::system system;
  int parameter;
  
  ioa::system::create_result r1 = system.create (new automaton1 ());
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::system::declare_result d1 = system.declare (r1.automaton, &parameter);
  BOOST_CHECK_EQUAL (d1.type, ioa::system::DECLARE_SUCCESS);
  d1 = system.declare (r1.automaton, &parameter);
  BOOST_CHECK_EQUAL (d1.type, ioa::system::DECLARE_EXISTS);
}

BOOST_AUTO_TEST_CASE (system_declare_success)
{
  ioa::system system;
  int parameter;
  
  ioa::system::create_result r1 = system.create (new automaton1 ());
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::system::declare_result d1 = system.declare (r1.automaton, &parameter);
  BOOST_CHECK_EQUAL (d1.type, ioa::system::DECLARE_SUCCESS);
  BOOST_CHECK_EQUAL (d1.parameter.value (), &parameter);
}

BOOST_AUTO_TEST_CASE (system_bind_binder_automaton_dne)
{
  ioa::system system;
  automaton1* binder_instance = new automaton1 ();
  automaton1* output_instance = new automaton1 ();
  automaton1* input_instance = new automaton1 ();
  
  ioa::system::create_result r1 = system.create (binder_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton1> binder = ioa::cast_automaton<automaton1> (r1.automaton);
  dummy_destroy_listener listener;
  ioa::system::destroy_result d1 = system.destroy (binder, listener);
  BOOST_CHECK_EQUAL (d1.type, ioa::system::DESTROY_SUCCESS);
  
  r1 = system.create (output_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton1> output = ioa::cast_automaton<automaton1> (r1.automaton);
  
  r1 = system.create (input_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton1> input = ioa::cast_automaton<automaton1> (r1.automaton);
  
  // ioa::system::bind_result c1 = system.bind (ioa::action<automaton::up_uv_output_action> (output, &automaton::up_uv_output),
  // 					     ioa::action<automaton::up_uv_input_action> (input, &automaton::up_uv_input),
  // 					     binder);
  ioa::system::bind_result c1 = system.bind (ioa::make_action (output, &automaton1::up_uv_output),
   					     ioa::make_action (input, &automaton1::up_uv_input),
 					     binder);

  BOOST_CHECK_EQUAL (c1.type, ioa::system::BIND_BINDER_AUTOMATON_DNE);
}

BOOST_AUTO_TEST_CASE (system_bind_output_automaton_dne)
{
  ioa::system system;
  automaton1* binder_instance = new automaton1 ();
  automaton1* output_instance = new automaton1 ();
  automaton1* input_instance = new automaton1 ();
  
  ioa::system::create_result r1 = system.create (binder_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton1> binder = ioa::cast_automaton<automaton1> (r1.automaton);
  
  r1 = system.create (output_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton1> output = ioa::cast_automaton<automaton1> (r1.automaton);
  dummy_destroy_listener listener;
  ioa::system::destroy_result d1 = system.destroy (output, listener);
  BOOST_CHECK_EQUAL (d1.type, ioa::system::DESTROY_SUCCESS);
  
  r1 = system.create (input_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton1> input = ioa::cast_automaton<automaton1> (r1.automaton);
  
  ioa::system::bind_result c1 = system.bind (ioa::make_action (output, &automaton1::up_uv_output),
					     ioa::make_action (input, &automaton1::up_uv_input),
					     binder);
  BOOST_CHECK_EQUAL (c1.type, ioa::system::BIND_OUTPUT_AUTOMATON_DNE);
}

BOOST_AUTO_TEST_CASE (system_bind_input_automaton_dne)
{
  ioa::system system;
  automaton1* binder_instance = new automaton1 ();
  automaton1* output_instance = new automaton1 ();
  automaton1* input_instance = new automaton1 ();
  
  ioa::system::create_result r1 = system.create (binder_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton1> binder = ioa::cast_automaton<automaton1> (r1.automaton);
  
  r1 = system.create (output_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton1> output = ioa::cast_automaton<automaton1> (r1.automaton);
  
  r1 = system.create (input_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton1> input = ioa::cast_automaton<automaton1> (r1.automaton);
  dummy_destroy_listener listener;
  ioa::system::destroy_result d1 = system.destroy (input, listener);
  BOOST_CHECK_EQUAL (d1.type, ioa::system::DESTROY_SUCCESS);
  
  ioa::system::bind_result c1 = system.bind (ioa::make_action (output, &automaton1::up_uv_output),
					     ioa::make_action (input, &automaton1::up_uv_input),
					     binder);
  BOOST_CHECK_EQUAL (c1.type, ioa::system::BIND_INPUT_AUTOMATON_DNE);
}

BOOST_AUTO_TEST_CASE (system_bind_output_parameter_dne)
{
  ioa::system system;
  int parameter;
  automaton1* binder_instance = new automaton1 ();
  automaton1* output_instance = new automaton1 ();
  automaton1* input_instance = new automaton1 ();

  ioa::system::create_result r1 = system.create (binder_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton1> binder = ioa::cast_automaton<automaton1> (r1.automaton);
  
  r1 = system.create (output_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton1> output = ioa::cast_automaton<automaton1> (r1.automaton);

  r1 = system.create (input_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton1> input = ioa::cast_automaton<automaton1> (r1.automaton);

  ioa::system::declare_result d1 = system.declare (output, &parameter);
  BOOST_CHECK_EQUAL (d1.type, ioa::system::DECLARE_SUCCESS);
  ioa::parameter_handle<int> param = ioa::cast_parameter<int> (d1.parameter);
  dummy_rescind_listener listener;
  ioa::system::rescind_result<int> k1 = system.rescind (output, param, listener);
  BOOST_CHECK_EQUAL (k1.type, ioa::system::RESCIND_SUCCESS);

  ioa::system::bind_result c1 = system.bind (ioa::make_action (output, &automaton1::p_uv_output, param),
					     ioa::make_action (input, &automaton1::up_uv_input));
  BOOST_CHECK_EQUAL (c1.type, ioa::system::BIND_OUTPUT_PARAMETER_DNE);
}

BOOST_AUTO_TEST_CASE (system_bind_input_parameter_dne)
{
  ioa::system system;
  int parameter;
  automaton1* binder_instance = new automaton1 ();
  automaton1* output_instance = new automaton1 ();
  automaton1* input_instance = new automaton1 ();

  ioa::system::create_result r1 = system.create (binder_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton1> binder = ioa::cast_automaton<automaton1> (r1.automaton);
  
  r1 = system.create (output_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton1> output = ioa::cast_automaton<automaton1> (r1.automaton);

  r1 = system.create (input_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton1> input = ioa::cast_automaton<automaton1> (r1.automaton);

  ioa::system::declare_result d1 = system.declare (input, &parameter);
  BOOST_CHECK_EQUAL (d1.type, ioa::system::DECLARE_SUCCESS);
  ioa::parameter_handle<int> param = ioa::cast_parameter<int> (d1.parameter);
  dummy_rescind_listener listener;
  ioa::system::rescind_result<int> k1 = system.rescind (input, param, listener);
  BOOST_CHECK_EQUAL (k1.type, ioa::system::RESCIND_SUCCESS);

  ioa::system::bind_result c1 = system.bind (ioa::make_action (output, &automaton1::up_uv_output),
					     ioa::make_action (input, &automaton1::p_uv_input, param));
  BOOST_CHECK_EQUAL (c1.type, ioa::system::BIND_INPUT_PARAMETER_DNE);
}

BOOST_AUTO_TEST_CASE (system_bind_exists)
{
  ioa::system system;
  automaton1* binder_instance = new automaton1 ();
  automaton1* output_instance = new automaton1 ();
  automaton1* input_instance = new automaton1 ();
  
  ioa::system::create_result r1 = system.create (binder_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton1> binder = ioa::cast_automaton<automaton1> (r1.automaton);
  
  r1 = system.create (output_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton1> output = ioa::cast_automaton<automaton1> (r1.automaton);
  
  r1 = system.create (input_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton1> input = ioa::cast_automaton<automaton1> (r1.automaton);
  
  ioa::system::bind_result c1 = system.bind (ioa::make_action (output, &automaton1::up_uv_output),
					     ioa::make_action (input, &automaton1::up_uv_input),
					     binder);
  BOOST_CHECK_EQUAL (c1.type, ioa::system::BIND_SUCCESS);

  c1 = system.bind (ioa::make_action (output, &automaton1::up_uv_output),
		    ioa::make_action (input, &automaton1::up_uv_input),
		    binder);
  BOOST_CHECK_EQUAL (c1.type, ioa::system::BIND_EXISTS);
}

BOOST_AUTO_TEST_CASE (system_bind_input_action_unavailable)
{
  ioa::system system;
  automaton1* binder_instance = new automaton1 ();
  automaton1* output1_instance = new automaton1 ();
  automaton1* output2_instance = new automaton1 ();
  automaton1* input_instance = new automaton1 ();
  
  ioa::system::create_result r1 = system.create (binder_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton1> binder = ioa::cast_automaton<automaton1> (r1.automaton);
    
  r1 = system.create (output1_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton1> output1 = ioa::cast_automaton<automaton1> (r1.automaton);
  
  r1 = system.create (output2_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton1> output2 = ioa::cast_automaton<automaton1> (r1.automaton);
  
  r1 = system.create (input_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton1> input = ioa::cast_automaton<automaton1> (r1.automaton);
  
  ioa::system::bind_result c1 = system.bind (ioa::make_action (output1, &automaton1::up_uv_output),
					     ioa::make_action (input, &automaton1::up_uv_input),
					     binder);
  BOOST_CHECK_EQUAL (c1.type, ioa::system::BIND_SUCCESS);

  c1 = system.bind (ioa::make_action (output1, &automaton1::up_uv_output),
		    ioa::make_action (input, &automaton1::up_uv_input),
		    binder);
  BOOST_CHECK_EQUAL (c1.type, ioa::system::BIND_EXISTS);

  c1 = system.bind (ioa::make_action (output2, &automaton1::up_uv_output),
		    ioa::make_action (input, &automaton1::up_uv_input),
		    binder);
  BOOST_CHECK_EQUAL (c1.type, ioa::system::BIND_INPUT_ACTION_UNAVAILABLE);
}

BOOST_AUTO_TEST_CASE (system_bind_output_action_unavailable)
{
  ioa::system system;
  int parameter;
  automaton1* binder_instance = new automaton1 ();
  automaton1* output_instance = new automaton1 ();
  automaton1* input_instance = new automaton1 ();

  ioa::system::create_result r1 = system.create (binder_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton1> binder = ioa::cast_automaton<automaton1> (r1.automaton);
  
  r1 = system.create (output_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton1> output = ioa::cast_automaton<automaton1> (r1.automaton);

  r1 = system.create (input_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton1> input = ioa::cast_automaton<automaton1> (r1.automaton);

  ioa::system::declare_result d1 = system.declare (input, &parameter);
  BOOST_CHECK_EQUAL (d1.type, ioa::system::DECLARE_SUCCESS);
  ioa::parameter_handle<int> param = ioa::cast_parameter<int> (d1.parameter);

  ioa::system::bind_result c1 = system.bind (ioa::make_action (output, &automaton1::up_uv_output),
					     ioa::make_action (input, &automaton1::up_uv_input),
					     binder);
  BOOST_CHECK_EQUAL (c1.type, ioa::system::BIND_SUCCESS);

  c1 = system.bind (ioa::make_action (output, &automaton1::up_uv_output),
		    ioa::make_action (input, &automaton1::p_uv_input, param));
  BOOST_CHECK_EQUAL (c1.type, ioa::system::BIND_OUTPUT_ACTION_UNAVAILABLE);
}

BOOST_AUTO_TEST_CASE (system_bind_success)
{
  ioa::system system;
  int parameter;
  automaton1* binder_instance = new automaton1 ();
  automaton1* output_instance = new automaton1 ();
  automaton1* input_instance = new automaton1 ();

  ioa::system::create_result r1 = system.create (binder_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton1> binder = ioa::cast_automaton<automaton1> (r1.automaton);
  
  r1 = system.create (output_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton1> output = ioa::cast_automaton<automaton1> (r1.automaton);

  r1 = system.create (input_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton1> input = ioa::cast_automaton<automaton1> (r1.automaton);

  ioa::system::declare_result d1 = system.declare (output, &parameter);
  BOOST_CHECK_EQUAL (d1.type, ioa::system::DECLARE_SUCCESS);
  ioa::parameter_handle<int> param = ioa::cast_parameter<int> (d1.parameter);

  ioa::system::bind_result c1 = system.bind (ioa::make_action (output, &automaton1::p_uv_output, param),
					     ioa::make_action (input, &automaton1::up_uv_input));
  BOOST_CHECK_EQUAL (c1.type, ioa::system::BIND_SUCCESS);

  dummy_scheduler scheduler;
  ioa::system::execute_result e1 = system.execute (ioa::make_action (output, &automaton1::p_uv_output, param), scheduler);
  BOOST_CHECK_EQUAL (e1.type, ioa::system::EXECUTE_SUCCESS);
  BOOST_CHECK (input_instance->up_uv_input.state);
}

BOOST_AUTO_TEST_CASE (system_unbind_binder_automaton_dne)
{
  ioa::system system;
  automaton1* binder_instance = new automaton1 ();
  automaton1* output_instance = new automaton1 ();
  automaton1* input_instance = new automaton1 ();
  
  ioa::system::create_result r1 = system.create (binder_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton1> binder = ioa::cast_automaton<automaton1> (r1.automaton);
  
  r1 = system.create (output_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton1> output = ioa::cast_automaton<automaton1> (r1.automaton);
  
  r1 = system.create (input_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton1> input = ioa::cast_automaton<automaton1> (r1.automaton);
  
  ioa::system::bind_result c1 = system.bind (ioa::make_action (output, &automaton1::up_uv_output),
					     ioa::make_action (input, &automaton1::up_uv_input),
					     binder);
  BOOST_CHECK_EQUAL (c1.type, ioa::system::BIND_SUCCESS);

  dummy_destroy_listener listener;
  ioa::system::destroy_result d1 = system.destroy (binder, listener);
  BOOST_CHECK_EQUAL (d1.type, ioa::system::DESTROY_SUCCESS);

  ioa::system::unbind_result x1 = system.unbind (ioa::make_action (output, &automaton1::up_uv_output),
						 ioa::make_action (input, &automaton1::up_uv_input),
						 binder);
  BOOST_CHECK_EQUAL (x1.type, ioa::system::UNBIND_BINDER_AUTOMATON_DNE);
}

BOOST_AUTO_TEST_CASE (system_unbind_output_automaton_dne)
{
  ioa::system system;
  automaton1* binder_instance = new automaton1 ();
  automaton1* output_instance = new automaton1 ();
  automaton1* input_instance = new automaton1 ();
  
  ioa::system::create_result r1 = system.create (binder_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton1> binder = ioa::cast_automaton<automaton1> (r1.automaton);
  
  r1 = system.create (output_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton1> output = ioa::cast_automaton<automaton1> (r1.automaton);
  
  r1 = system.create (input_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton1> input = ioa::cast_automaton<automaton1> (r1.automaton);
  
  ioa::system::bind_result c1 = system.bind (ioa::make_action (output, &automaton1::up_uv_output),
					     ioa::make_action (input, &automaton1::up_uv_input),
					     binder);
  BOOST_CHECK_EQUAL (c1.type, ioa::system::BIND_SUCCESS);

  dummy_destroy_listener listener;
  ioa::system::destroy_result d1 = system.destroy (output, listener);
  BOOST_CHECK_EQUAL (d1.type, ioa::system::DESTROY_SUCCESS);

  ioa::system::unbind_result x1 = system.unbind (ioa::make_action (output, &automaton1::up_uv_output), 
						 ioa::make_action (input, &automaton1::up_uv_input),
						 binder);
  BOOST_CHECK_EQUAL (x1.type, ioa::system::UNBIND_OUTPUT_AUTOMATON_DNE);
}

BOOST_AUTO_TEST_CASE (system_unbind_input_automaton_dne)
{
  ioa::system system;
  automaton1* binder_instance = new automaton1 ();
  automaton1* output_instance = new automaton1 ();
  automaton1* input_instance = new automaton1 ();
  
  ioa::system::create_result r1 = system.create (binder_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton1> binder = ioa::cast_automaton<automaton1> (r1.automaton);
  
  r1 = system.create (output_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton1> output = ioa::cast_automaton<automaton1> (r1.automaton);
  
  r1 = system.create (input_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton1> input = ioa::cast_automaton<automaton1> (r1.automaton);
  
  ioa::system::bind_result c1 = system.bind (ioa::make_action (output, &automaton1::up_uv_output),
					     ioa::make_action (input, &automaton1::up_uv_input),
					     binder);
  BOOST_CHECK_EQUAL (c1.type, ioa::system::BIND_SUCCESS);

  dummy_destroy_listener listener;
  ioa::system::destroy_result d1 = system.destroy (input, listener);
  BOOST_CHECK_EQUAL (d1.type, ioa::system::DESTROY_SUCCESS);

  ioa::system::unbind_result x1 = system.unbind (ioa::make_action (output, &automaton1::up_uv_output),
						 ioa::make_action (input, &automaton1::up_uv_input),
						 binder);
  BOOST_CHECK_EQUAL (x1.type, ioa::system::UNBIND_INPUT_AUTOMATON_DNE);
}

BOOST_AUTO_TEST_CASE (system_unbind_output_parameter_dne)
{
  ioa::system system;
  int parameter;
  automaton1* binder_instance = new automaton1 ();
  automaton1* output_instance = new automaton1 ();
  automaton1* input_instance = new automaton1 ();

  ioa::system::create_result r1 = system.create (binder_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton1> binder = ioa::cast_automaton<automaton1> (r1.automaton);
  
  r1 = system.create (output_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton1> output = ioa::cast_automaton<automaton1> (r1.automaton);

  r1 = system.create (input_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton1> input = ioa::cast_automaton<automaton1> (r1.automaton);

  ioa::system::declare_result d1 = system.declare (output, &parameter);
  BOOST_CHECK_EQUAL (d1.type, ioa::system::DECLARE_SUCCESS);
  ioa::parameter_handle<int> param = ioa::cast_parameter<int> (d1.parameter);
  dummy_rescind_listener listener;
  ioa::system::rescind_result<int> k1 = system.rescind (output, param, listener);
  BOOST_CHECK_EQUAL (k1.type, ioa::system::RESCIND_SUCCESS);

  ioa::system::unbind_result c1 = system.unbind (ioa::make_action (output, &automaton1::p_uv_output, param),
						 ioa::make_action (input, &automaton1::up_uv_input));
  BOOST_CHECK_EQUAL (c1.type, ioa::system::UNBIND_OUTPUT_PARAMETER_DNE);
}

BOOST_AUTO_TEST_CASE (system_unbind_input_parameter_dne)
{
  ioa::system system;
  int parameter;
  automaton1* binder_instance = new automaton1 ();
  automaton1* output_instance = new automaton1 ();
  automaton1* input_instance = new automaton1 ();

  ioa::system::create_result r1 = system.create (binder_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton1> binder = ioa::cast_automaton<automaton1> (r1.automaton);
  
  r1 = system.create (output_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton1> output = ioa::cast_automaton<automaton1> (r1.automaton);

  r1 = system.create (input_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton1> input = ioa::cast_automaton<automaton1> (r1.automaton);

  ioa::system::declare_result d1 = system.declare (input, &parameter);
  BOOST_CHECK_EQUAL (d1.type, ioa::system::DECLARE_SUCCESS);
  ioa::parameter_handle<int> param = ioa::cast_parameter<int> (d1.parameter);
  dummy_rescind_listener listener;
  ioa::system::rescind_result<int> k1 = system.rescind (input, param, listener);
  BOOST_CHECK_EQUAL (k1.type, ioa::system::RESCIND_SUCCESS);

  ioa::system::unbind_result c1 = system.unbind (ioa::make_action (output, &automaton1::up_uv_output), 
						 ioa::make_action (input, &automaton1::p_uv_input, param));
  BOOST_CHECK_EQUAL (c1.type, ioa::system::UNBIND_INPUT_PARAMETER_DNE);
}

BOOST_AUTO_TEST_CASE (system_unbind_exists)
{
  ioa::system system;
  automaton1* binder_instance = new automaton1 ();
  automaton1* output_instance = new automaton1 ();
  automaton1* input_instance = new automaton1 ();
  
  ioa::system::create_result r1 = system.create (binder_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton1> binder = ioa::cast_automaton<automaton1> (r1.automaton);
  
  r1 = system.create (output_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton1> output = ioa::cast_automaton<automaton1> (r1.automaton);
  
  r1 = system.create (input_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton1> input = ioa::cast_automaton<automaton1> (r1.automaton);
  
  ioa::system::unbind_result j1 = system.unbind (ioa::make_action (output, &automaton1::up_uv_output),
						 ioa::make_action (input, &automaton1::up_uv_input), binder);
  BOOST_CHECK_EQUAL (j1.type, ioa::system::UNBIND_EXISTS);
}

BOOST_AUTO_TEST_CASE (system_unbind_success)
{
  ioa::system system;
  int parameter;
  automaton1* binder_instance = new automaton1 ();
  automaton1* output_instance = new automaton1 ();
  automaton1* input_instance = new automaton1 ();

  ioa::system::create_result r1 = system.create (binder_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton1> binder = ioa::cast_automaton<automaton1> (r1.automaton);
  
  r1 = system.create (output_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton1> output = ioa::cast_automaton<automaton1> (r1.automaton);

  r1 = system.create (input_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton1> input = ioa::cast_automaton<automaton1> (r1.automaton);

  ioa::system::declare_result d1 = system.declare (output, &parameter);
  BOOST_CHECK_EQUAL (d1.type, ioa::system::DECLARE_SUCCESS);
  ioa::parameter_handle<int> param = ioa::cast_parameter<int> (d1.parameter);

  ioa::system::bind_result c1 = system.bind (ioa::make_action (output, &automaton1::p_uv_output, param), 
					     ioa::make_action (input, &automaton1::up_uv_input));
  BOOST_CHECK_EQUAL (c1.type, ioa::system::BIND_SUCCESS);

  ioa::system::unbind_result j1 = system.unbind (ioa::make_action (output, &automaton1::p_uv_output, param),
						 ioa::make_action (input, &automaton1::up_uv_input));
  BOOST_CHECK_EQUAL (j1.type, ioa::system::UNBIND_SUCCESS);
}

BOOST_AUTO_TEST_CASE (system_rescind_automaton_dne)
{
  ioa::system system;
  int parameter;
  
  ioa::system::create_result r1 = system.create (new automaton1 ());
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::system::declare_result d1 = system.declare (r1.automaton, &parameter);
  BOOST_CHECK_EQUAL (d1.type, ioa::system::DECLARE_SUCCESS);
  ioa::parameter_handle<int> param = ioa::cast_parameter<int> (d1.parameter);

  dummy_destroy_listener destroy_listener;
  ioa::system::destroy_result y1 = system.destroy (r1.automaton, destroy_listener);
  BOOST_CHECK_EQUAL (y1.type, ioa::system::DESTROY_SUCCESS);

  dummy_rescind_listener listener;
  ioa::system::rescind_result<int> k1 = system.rescind (r1.automaton, param, listener);
  BOOST_CHECK_EQUAL (k1.type, ioa::system::RESCIND_AUTOMATON_DNE);
}

BOOST_AUTO_TEST_CASE (system_rescind_exists)
{
  ioa::system system;
  int parameter;
  
  ioa::system::create_result r1 = system.create (new automaton1 ());
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::system::declare_result d1 = system.declare (r1.automaton, &parameter);
  BOOST_CHECK_EQUAL (d1.type, ioa::system::DECLARE_SUCCESS);
  ioa::parameter_handle<int> param = ioa::cast_parameter<int> (d1.parameter);
  dummy_rescind_listener listener;
  ioa::system::rescind_result<int> k1 = system.rescind (r1.automaton, param, listener);
  BOOST_CHECK_EQUAL (k1.type, ioa::system::RESCIND_SUCCESS);
  k1 = system.rescind (r1.automaton, param, listener);
  BOOST_CHECK_EQUAL (k1.type, ioa::system::RESCIND_EXISTS);
}

BOOST_AUTO_TEST_CASE (system_rescind_success)
{
  ioa::system system;
  int parameter;
  automaton1* binder_instance = new automaton1 ();
  automaton1* output_instance = new automaton1 ();
  automaton1* input_instance = new automaton1 ();

  ioa::system::create_result r1 = system.create (binder_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton1> binder = ioa::cast_automaton<automaton1> (r1.automaton);
  
  r1 = system.create (output_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton1> output = ioa::cast_automaton<automaton1> (r1.automaton);

  r1 = system.create (input_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton1> input = ioa::cast_automaton<automaton1> (r1.automaton);

  ioa::system::declare_result d1 = system.declare (binder, &parameter);
  BOOST_CHECK_EQUAL (d1.type, ioa::system::DECLARE_SUCCESS);
  ioa::parameter_handle<int> param = ioa::cast_parameter<int> (d1.parameter);

  ioa::system::bind_result c1 = system.bind (ioa::make_action (binder, &automaton1::p_uv_output, param),
					     ioa::make_action (input, &automaton1::up_uv_input));
  BOOST_CHECK_EQUAL (c1.type, ioa::system::BIND_SUCCESS);

  c1 = system.bind (ioa::make_action (output, &automaton1::up_uv_output),
		    ioa::make_action (binder, &automaton1::p_uv_input, param));
  BOOST_CHECK_EQUAL (c1.type, ioa::system::BIND_SUCCESS);

  dummy_rescind_listener listener;
  ioa::system::rescind_result<int> k1 = system.rescind (binder, param, listener);
  BOOST_CHECK_EQUAL (k1.type, ioa::system::RESCIND_SUCCESS);
  BOOST_CHECK (std::find (listener.output_records.begin (),
			  listener.output_records.end (),
			  output_unbind_record (binder,
						   &binder_instance->p_uv_output,
						   param,
						   input,
						   &input_instance->up_uv_input)) != listener.output_records.end ());
  BOOST_CHECK (std::find (listener.input_records.begin (),
			  listener.input_records.end (),
			  input_unbind_record (output,
						  &output_instance->up_uv_output,
						  binder,
						  &binder_instance->p_uv_input,
						  param)) != listener.input_records.end ());


}

BOOST_AUTO_TEST_CASE (system_destroy_destroyer_dne)
{
  ioa::system system;
  ioa::system::create_result r1 = system.create (new automaton1 ());
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton1> parent_handle = ioa::cast_automaton<automaton1> (r1.automaton);
  r1 = system.create (parent_handle, new automaton1 ());
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton1> child_handle = ioa::cast_automaton<automaton1> (r1.automaton);

  dummy_destroy_listener listener;
  ioa::system::destroy_result d1 = system.destroy (parent_handle, listener);
  BOOST_CHECK_EQUAL (d1.type, ioa::system::DESTROY_SUCCESS);
  d1 = system.destroy (parent_handle, child_handle, listener);
  BOOST_CHECK_EQUAL (d1.type, ioa::system::DESTROY_DESTROYER_DNE);
}

BOOST_AUTO_TEST_CASE (system_destroy_destroyer_not_creator)
{
   ioa::system system;
   ioa::system::create_result r1 = system.create (new automaton1 ());
   BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
   ioa::automaton_handle<automaton1> parent_handle = ioa::cast_automaton<automaton1> (r1.automaton);
   r1 = system.create (parent_handle, new automaton1 ());
   BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
   ioa::automaton_handle<automaton1> child_handle = ioa::cast_automaton<automaton1> (r1.automaton);
   r1 = system.create (new automaton1 ());
   BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
   ioa::automaton_handle<automaton1> third_party_handle = ioa::cast_automaton<automaton1> (r1.automaton);

   dummy_destroy_listener listener;
   ioa::system::destroy_result d1 = system.destroy (third_party_handle, child_handle, listener);
   BOOST_CHECK_EQUAL (d1.type, ioa::system::DESTROY_DESTROYER_NOT_CREATOR);
}

BOOST_AUTO_TEST_CASE (system_destroy_exists)
{
  ioa::system system;
  ioa::system::create_result r1 = system.create (new automaton1 ());
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);

  dummy_destroy_listener listener;
  ioa::system::destroy_result x1 = system.destroy (r1.automaton, listener);
  BOOST_CHECK_EQUAL (x1.type, ioa::system::DESTROY_SUCCESS);
  x1 = system.destroy (r1.automaton, listener);
  BOOST_CHECK_EQUAL (x1.type, ioa::system::DESTROY_EXISTS);
}

BOOST_AUTO_TEST_CASE (system_destroy_success)
{
  ioa::system system;
  int parameter;

  automaton1* alpha_instance = new automaton1 ();
  automaton1* beta_instance = new automaton1 ();
  automaton1* gamma_instance = new automaton1 ();

  ioa::system::create_result r1 = system.create (alpha_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton1> alpha = ioa::cast_automaton<automaton1> (r1.automaton);

  r1 = system.create (alpha, beta_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton1> beta = ioa::cast_automaton<automaton1> (r1.automaton);

  r1 = system.create (beta, gamma_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton1> gamma = ioa::cast_automaton<automaton1> (r1.automaton);

  ioa::system::declare_result d1 = system.declare (beta, &parameter);
  BOOST_CHECK_EQUAL (d1.type, ioa::system::DECLARE_SUCCESS);
  ioa::parameter_handle<int> param = ioa::cast_parameter<int> (d1.parameter);

  ioa::system::bind_result c1 = system.bind (ioa::make_action (alpha, &automaton1::up_uv_output),
					     ioa::make_action (beta, &automaton1::up_uv_input),
					     alpha);
  BOOST_CHECK_EQUAL (c1.type, ioa::system::BIND_SUCCESS);
  c1 = system.bind (ioa::make_action (beta, &automaton1::p_uv_output, param),
		    ioa::make_action (gamma, &automaton1::up_uv_input));
  BOOST_CHECK_EQUAL (c1.type, ioa::system::BIND_SUCCESS);
  c1 = system.bind (ioa::make_action (gamma, &automaton1::up_uv_output),
		    ioa::make_action (beta, &automaton1::p_uv_input, param));
  BOOST_CHECK_EQUAL (c1.type, ioa::system::BIND_SUCCESS);

  dummy_destroy_listener listener;
  ioa::system::destroy_result x1 = system.destroy (beta, listener);
  BOOST_CHECK_EQUAL (x1.type, ioa::system::DESTROY_SUCCESS);

  std::pair <ioa::generic_automaton_handle, ioa::generic_automaton_handle> p1 (alpha, beta);
  BOOST_CHECK (std::find (listener.destroyed_records.begin (),
			  listener.destroyed_records.end (),
			  p1) != listener.destroyed_records.end ());
  std::pair <ioa::generic_automaton_handle, ioa::generic_automaton_handle> p2 (beta, gamma);
  BOOST_CHECK (std::find (listener.destroyed_records.begin (),
			  listener.destroyed_records.end (),
			  p2) != listener.destroyed_records.end ());
  BOOST_CHECK (std::find (listener.unbind_records.begin (),
			  listener.unbind_records.end (),
			  unbind_record (alpha,
					    &alpha_instance->up_uv_output,
					    beta,
					    &beta_instance->up_uv_input,
					    alpha)) != listener.unbind_records.end ());
  BOOST_CHECK (std::find (listener.output_records.begin (),
			  listener.output_records.end (),
			  output_unbind_record (beta,
						   &beta_instance->p_uv_output,
						   param,
						   gamma,
						   &gamma_instance->up_uv_input)) != listener.output_records.end ());
  BOOST_CHECK (std::find (listener.input_records.begin (),
  			  listener.input_records.end (),
  			  input_unbind_record (gamma,
  						  &gamma_instance->up_uv_output,
  						  beta,
  						  &beta_instance->p_uv_input,
  						  param)) != listener.input_records.end ());



}

BOOST_AUTO_TEST_CASE (system_execute_output_automaton_dne)
{
  ioa::system system;
  automaton1* output_instance = new automaton1 ();

  ioa::system::create_result r1 = system.create (output_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton1> output = ioa::cast_automaton<automaton1> (r1.automaton);

  dummy_destroy_listener listener;
  ioa::system::destroy_result y1 = system.destroy (output, listener);
  BOOST_CHECK_EQUAL (y1.type, ioa::system::DESTROY_SUCCESS);

  dummy_scheduler scheduler;
  ioa::system::execute_result e1 = system.execute (ioa::make_action (output, &automaton1::up_uv_output), scheduler);
  BOOST_CHECK_EQUAL (e1.type, ioa::system::EXECUTE_AUTOMATON_DNE);
}

BOOST_AUTO_TEST_CASE (system_execute_output_parameter_dne)
{
  ioa::system system;
  int parameter;
  automaton1* output_instance = new automaton1 ();

  ioa::system::create_result r1 = system.create (output_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton1> output = ioa::cast_automaton<automaton1> (r1.automaton);

  ioa::system::declare_result d1 = system.declare (output, &parameter);
  BOOST_CHECK_EQUAL (d1.type, ioa::system::DECLARE_SUCCESS);
  ioa::parameter_handle<int> param = ioa::cast_parameter<int> (d1.parameter);

  dummy_rescind_listener listener;
  ioa::system::rescind_result<int> k1 = system.rescind (r1.automaton, param, listener);
  BOOST_CHECK_EQUAL (k1.type, ioa::system::RESCIND_SUCCESS);

  dummy_scheduler scheduler;
  ioa::system::execute_result e1 = system.execute (ioa::make_action (output, &automaton1::p_uv_output, param), scheduler);
  BOOST_CHECK_EQUAL (e1.type, ioa::system::EXECUTE_PARAMETER_DNE);
}

BOOST_AUTO_TEST_CASE (system_execute_output_success)
{
  ioa::system system;
  automaton1* output_instance = new automaton1 ();

  ioa::system::create_result r1 = system.create (output_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton1> output = ioa::cast_automaton<automaton1> (r1.automaton);

  dummy_scheduler scheduler;
  ioa::system::execute_result e1 = system.execute (ioa::make_action (output, &automaton1::up_uv_output), scheduler);
  BOOST_CHECK_EQUAL (e1.type, ioa::system::EXECUTE_SUCCESS);
  BOOST_CHECK (output_instance->up_uv_output.state);
}

BOOST_AUTO_TEST_CASE (system_deliver_event_automaton_dne)
{
  ioa::system system;
  automaton1* instance = new automaton1 ();

  ioa::system::create_result r1 = system.create (instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton1> handle = ioa::cast_automaton<automaton1> (r1.automaton);

  dummy_destroy_listener listener;
  ioa::system::destroy_result y1 = system.destroy (handle, listener);
  BOOST_CHECK_EQUAL (y1.type, ioa::system::DESTROY_SUCCESS);

  dummy_scheduler scheduler;
  ioa::system::execute_result e1 = system.execute (ioa::make_action (handle, &automaton1::uv_event), scheduler);
  BOOST_CHECK_EQUAL (e1.type, ioa::system::EXECUTE_AUTOMATON_DNE);

  e1 = system.execute (ioa::make_action (handle, &automaton1::v_event, 9845), scheduler);
  BOOST_CHECK_EQUAL (e1.type, ioa::system::EXECUTE_AUTOMATON_DNE);
}

BOOST_AUTO_TEST_CASE (system_deliver_event_success)
{
  ioa::system system;
  automaton1* instance = new automaton1 ();

  ioa::system::create_result r1 = system.create (instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton1> handle = ioa::cast_automaton<automaton1> (r1.automaton);

  dummy_scheduler scheduler;
  ioa::system::execute_result e1 = system.execute (ioa::make_action (handle, &automaton1::uv_event), scheduler);
  BOOST_CHECK_EQUAL (e1.type, ioa::system::EXECUTE_SUCCESS);
  BOOST_CHECK (instance->uv_event.state);

  e1 = system.execute (ioa::make_action (handle, &automaton1::v_event, 9845), scheduler);
  BOOST_CHECK_EQUAL (e1.type, ioa::system::EXECUTE_SUCCESS);
  BOOST_CHECK (instance->v_event.state);
  BOOST_CHECK_EQUAL (instance->v_event.last_value, 9845);
}

BOOST_AUTO_TEST_SUITE_END()
