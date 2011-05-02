#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE system
#include <boost/test/unit_test.hpp>

#include <system.hpp>
#include "automaton.hpp"

struct output_decompose_record
{
  ioa::generic_automaton_handle output_automaton;
  const void* output_member;
  ioa::generic_parameter_handle output_parameter;
  ioa::generic_automaton_handle input_automaton;
  const void* input_member;

  output_decompose_record (const ioa::generic_automaton_handle& output_automaton,
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
  
  bool operator== (const output_decompose_record& odr) const {
    return output_automaton == odr.output_automaton &&
      output_member == odr.output_member &&
      output_parameter == odr.output_parameter &&
      input_automaton == odr.input_automaton &&
      input_member == odr.input_member;
  }
};

struct input_decompose_record
{
  ioa::generic_automaton_handle output_automaton;
  const void* output_member;
  ioa::generic_automaton_handle input_automaton;
  const void* input_member;
  ioa::generic_parameter_handle input_parameter;

  input_decompose_record (const ioa::generic_automaton_handle& output_automaton,
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

  bool operator== (const input_decompose_record& odr) const {
    return output_automaton == odr.output_automaton &&
      output_member == odr.output_member &&
      input_automaton == odr.input_automaton &&
      input_member == odr.input_member &&
      input_parameter == odr.input_parameter;
  }
};

struct dummy_decompose_listener :
  public ioa::decompose_listener_interface
{
  std::vector<output_decompose_record> output_records;
  std::vector<input_decompose_record> input_records;

  void decomposed (const ioa::generic_automaton_handle& output_automaton,
		   const void* output_member,
		   const ioa::generic_parameter_handle& output_parameter,
		   const ioa::generic_automaton_handle& input_automaton,
		   const void* input_member) {
    output_decompose_record odr (output_automaton, output_member, output_parameter, input_automaton, input_member);
    output_records.push_back (odr);
  }

  void decomposed (const ioa::generic_automaton_handle& output_automaton,
		   const void* output_member,
		   const ioa::generic_automaton_handle& input_automaton,
		   const void* input_member,
		   const ioa::generic_parameter_handle& input_parameter) {
    input_decompose_record idr (output_automaton, output_member, input_automaton, input_member, input_parameter);
    input_records.push_back (idr);
  }
};

BOOST_AUTO_TEST_SUITE(system_suite)

BOOST_AUTO_TEST_CASE (system_create_creator_dne)
{
  // ioa::system system;
  // automaton* a = new automaton ();
  // ioa::timestamp<ioa::typed_automaton<blah> > a (&y);
  // x = new blah ();
  // ioa::system::create_result<blah> r1 = system.create (a, x);
  // BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_CREATOR_DNE);
  // delete a;
  // 1. Create an automaton.
  // 2. Destroy the automaton.
  // 3. Create another automaton using handle of first automaton.
  BOOST_CHECK (false);
}

BOOST_AUTO_TEST_CASE (system_create_exists)
{
  ioa::system system;
  automaton* a = new automaton ();
  ioa::system::create_result<automaton> r1 = system.create (a);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  r1 = system.create (a);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_EXISTS);
}

BOOST_AUTO_TEST_CASE (system_create_success)
{
  ioa::system system;
  automaton* a = new automaton ();
  ioa::system::create_result<automaton> r1 = system.create (a);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
}

BOOST_AUTO_TEST_CASE (system_declare_automaton_dne)
{
//   ioa::system system;
//   int parameter;
  
//   blah* x = new blah ();
//   ioa::typed_automaton<blah> y (x);
//   ioa::timestamp<ioa::typed_automaton<blah> > a (&y);
//   ioa::system::declare_result d1 = system.declare (a, &parameter);
//   BOOST_CHECK_EQUAL (d1.type, ioa::system::DECLARE_AUTOMATON_DNE);
  BOOST_CHECK (false);
}

BOOST_AUTO_TEST_CASE (system_declare_exists)
{
  ioa::system system;
  int parameter;
  
  ioa::system::create_result<automaton> r1 = system.create (new automaton ());
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::system::declare_result<int> d1 = system.declare (r1.automaton, &parameter);
  BOOST_CHECK_EQUAL (d1.type, ioa::system::DECLARE_SUCCESS);
  d1 = system.declare (r1.automaton, &parameter);
  BOOST_CHECK_EQUAL (d1.type, ioa::system::DECLARE_EXISTS);
}

BOOST_AUTO_TEST_CASE (system_declare_success)
{
  ioa::system system;
  int parameter;
  
  ioa::system::create_result<automaton> r1 = system.create (new automaton ());
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::system::declare_result<int> d1 = system.declare (r1.automaton, &parameter);
  BOOST_CHECK_EQUAL (d1.type, ioa::system::DECLARE_SUCCESS);
  BOOST_CHECK_EQUAL (d1.parameter.value (), &parameter);
}

BOOST_AUTO_TEST_CASE (system_compose_compose_automaton_dne)
{
//   ioa::system system;

//   blah* x = new blah ();
//   ioa::typed_automaton<blah> y (x);
//   ioa::timestamp<ioa::typed_automaton<blah> > a (&y);

//   x = new blah ();
//   ioa::system::create_result<blah> r1 = system.create (x);
//   BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
//   ioa::timestamp<ioa::typed_automaton<blah> > output = r1.automaton;

//   r1 = system.create (new blah ());
//   BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
//   ioa::timestamp<ioa::typed_automaton<blah> > input = r1.automaton;

//   ioa::system::compose_result c1 = system.compose (output, &blah::output, input, &blah::input, a);
//   BOOST_CHECK_EQUAL (c1.type, ioa::system::COMPOSE_COMPOSER_AUTOMATON_DNE);
  BOOST_CHECK (false);
}

BOOST_AUTO_TEST_CASE (system_compose_output_automaton_dne)
{
//   ioa::system system;

//   blah* x = new blah ();
//   ioa::typed_automaton<blah> y (x);
//   ioa::timestamp<ioa::typed_automaton<blah> > a (&y);

//   x = new blah ();
//   ioa::system::create_result<blah> r1 = system.create (x);
//   BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
//   ioa::timestamp<ioa::automaton> composer = r1.automaton;

//   ioa::system::compose_result c1 = system.compose (a, &blah::output, a, &blah::input, composer);
//   BOOST_CHECK_EQUAL (c1.type, ioa::system::COMPOSE_OUTPUT_AUTOMATON_DNE);
  BOOST_CHECK (false);
}

BOOST_AUTO_TEST_CASE (system_compose_input_automaton_dne)
{
//   ioa::system system;

//   blah* x = new blah ();
//   ioa::typed_automaton<blah> y (x);
//   ioa::timestamp<ioa::typed_automaton<blah> > a (&y);

//   x = new blah ();
//   ioa::system::create_result<blah> r1 = system.create (x);
//   BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
//   ioa::timestamp<ioa::automaton> composer = r1.automaton;

//   r1 = system.create (new blah ());
//   BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
//   ioa::timestamp<ioa::typed_automaton<blah> > output = r1.automaton;

//   ioa::system::compose_result c1 = system.compose (output, &blah::output, a, &blah::input, composer);
//   BOOST_CHECK_EQUAL (c1.type, ioa::system::COMPOSE_INPUT_AUTOMATON_DNE);
  BOOST_CHECK (false);
}

BOOST_AUTO_TEST_CASE (system_compose_output_parameter_dne)
{
  ioa::system system;
  int parameter;
  automaton* composer_instance = new automaton ();
  automaton* output_instance = new automaton ();
  automaton* input_instance = new automaton ();

  ioa::system::create_result<automaton> r1 = system.create (composer_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton> composer = r1.automaton;
  
  r1 = system.create (output_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton> output = r1.automaton;

  r1 = system.create (input_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton> input = r1.automaton;

  ioa::system::declare_result<int> d1 = system.declare (output, &parameter);
  BOOST_CHECK_EQUAL (d1.type, ioa::system::DECLARE_SUCCESS);
  ioa::parameter_handle<int> param = d1.parameter;
  dummy_decompose_listener listener;
  ioa::system::rescind_result<int> k1 = system.rescind (output, param, listener);
  BOOST_CHECK_EQUAL (k1.type, ioa::system::RESCIND_SUCCESS);

  ioa::system::compose_result c1 = system.compose (output, &automaton::p_uv_output, param, input, &automaton::up_uv_input);
  BOOST_CHECK_EQUAL (c1.type, ioa::system::COMPOSE_OUTPUT_PARAMETER_DNE);
}

BOOST_AUTO_TEST_CASE (system_compose_input_parameter_dne)
{
  ioa::system system;
  int parameter;
  automaton* composer_instance = new automaton ();
  automaton* output_instance = new automaton ();
  automaton* input_instance = new automaton ();

  ioa::system::create_result<automaton> r1 = system.create (composer_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton> composer = r1.automaton;
  
  r1 = system.create (output_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton> output = r1.automaton;

  r1 = system.create (input_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton> input = r1.automaton;

  ioa::system::declare_result<int> d1 = system.declare (input, &parameter);
  BOOST_CHECK_EQUAL (d1.type, ioa::system::DECLARE_SUCCESS);
  ioa::parameter_handle<int> param = d1.parameter;
  dummy_decompose_listener listener;
  ioa::system::rescind_result<int> k1 = system.rescind (input, param, listener);
  BOOST_CHECK_EQUAL (k1.type, ioa::system::RESCIND_SUCCESS);

  ioa::system::compose_result c1 = system.compose (output, &automaton::up_uv_output, input, &automaton::p_uv_input, param);
  BOOST_CHECK_EQUAL (c1.type, ioa::system::COMPOSE_INPUT_PARAMETER_DNE);
}

BOOST_AUTO_TEST_CASE (system_compose_exists)
{
  ioa::system system;
  automaton* composer_instance = new automaton ();
  automaton* output_instance = new automaton ();
  automaton* input_instance = new automaton ();
  
  ioa::system::create_result<automaton> r1 = system.create (composer_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton> composer = r1.automaton;
  
  r1 = system.create (output_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton> output = r1.automaton;
  
  r1 = system.create (input_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton> input = r1.automaton;
  
  ioa::system::compose_result c1 = system.compose (output, &automaton::up_uv_output, input, &automaton::up_uv_input, composer);
  BOOST_CHECK_EQUAL (c1.type, ioa::system::COMPOSE_SUCCESS);

  c1 = system.compose (output, &automaton::up_uv_output, input, &automaton::up_uv_input, composer);
  BOOST_CHECK_EQUAL (c1.type, ioa::system::COMPOSE_EXISTS);
}

BOOST_AUTO_TEST_CASE (system_compose_input_action_unavailable)
{
  ioa::system system;
  automaton* composer_instance = new automaton ();
  automaton* output1_instance = new automaton ();
  automaton* output2_instance = new automaton ();
  automaton* input_instance = new automaton ();
  
  ioa::system::create_result<automaton> r1 = system.create (composer_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton> composer = r1.automaton;
    
  r1 = system.create (output1_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton> output1 = r1.automaton;
  
  r1 = system.create (output2_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton> output2 = r1.automaton;
  
  r1 = system.create (input_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton> input = r1.automaton;
  
  ioa::system::compose_result c1 = system.compose (output1, &automaton::up_uv_output, input, &automaton::up_uv_input, composer);
  BOOST_CHECK_EQUAL (c1.type, ioa::system::COMPOSE_SUCCESS);

  c1 = system.compose (output1, &automaton::up_uv_output, input, &automaton::up_uv_input, composer);
  BOOST_CHECK_EQUAL (c1.type, ioa::system::COMPOSE_EXISTS);

  c1 = system.compose (output2, &automaton::up_uv_output, input, &automaton::up_uv_input, composer);
  BOOST_CHECK_EQUAL (c1.type, ioa::system::COMPOSE_INPUT_ACTION_UNAVAILABLE);
}

BOOST_AUTO_TEST_CASE (system_compose_output_action_unavailable)
{
  ioa::system system;
  int parameter;
  automaton* composer_instance = new automaton ();
  automaton* output_instance = new automaton ();
  automaton* input_instance = new automaton ();

  ioa::system::create_result<automaton> r1 = system.create (composer_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton> composer = r1.automaton;
  
  r1 = system.create (output_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton> output = r1.automaton;

  r1 = system.create (input_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton> input = r1.automaton;

  ioa::system::declare_result<int> d1 = system.declare (input, &parameter);
  BOOST_CHECK_EQUAL (d1.type, ioa::system::DECLARE_SUCCESS);
  ioa::parameter_handle<int> param = d1.parameter;

  ioa::system::compose_result c1 = system.compose (output, &automaton::up_uv_output, input, &automaton::up_uv_input, composer);
  BOOST_CHECK_EQUAL (c1.type, ioa::system::COMPOSE_SUCCESS);

  c1 = system.compose (output, &automaton::up_uv_output, input, &automaton::p_uv_input, param);
  BOOST_CHECK_EQUAL (c1.type, ioa::system::COMPOSE_OUTPUT_ACTION_UNAVAILABLE);
}

BOOST_AUTO_TEST_CASE (system_compose_success)
{
  ioa::system system;
  int parameter;
  automaton* composer_instance = new automaton ();
  automaton* output_instance = new automaton ();
  automaton* input_instance = new automaton ();

  ioa::system::create_result<automaton> r1 = system.create (composer_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton> composer = r1.automaton;
  
  r1 = system.create (output_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton> output = r1.automaton;

  r1 = system.create (input_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton> input = r1.automaton;

  ioa::system::declare_result<int> d1 = system.declare (output, &parameter);
  BOOST_CHECK_EQUAL (d1.type, ioa::system::DECLARE_SUCCESS);
  ioa::parameter_handle<int> param = d1.parameter;

  ioa::system::compose_result c1 = system.compose (output, &automaton::p_uv_output, param, input, &automaton::up_uv_input);
  BOOST_CHECK_EQUAL (c1.type, ioa::system::COMPOSE_SUCCESS);

  ioa::system::execute_result e1 = system.execute_output (output, &automaton::p_uv_output, param);
  BOOST_CHECK_EQUAL (e1.type, ioa::system::EXECUTE_SUCCESS);
  BOOST_CHECK (input_instance->up_uv_input.state);
}

BOOST_AUTO_TEST_CASE (system_decompose_compose_automaton_dne)
{
//   ioa::system system;

//   blah* x = new blah ();
//   ioa::typed_automaton<blah> y (x);
//   ioa::timestamp<ioa::typed_automaton<blah> > a (&y);

//   x = new blah ();
//   ioa::system::create_result<blah> r1 = system.create (x);
//   BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
//   ioa::timestamp<ioa::typed_automaton<blah> > output = r1.automaton;

//   r1 = system.create (new blah ());
//   BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
//   ioa::timestamp<ioa::typed_automaton<blah> > input = r1.automaton;

//   ioa::system::compose_result c1 = system.compose (output, &blah::output, input, &blah::input, a);
//   BOOST_CHECK_EQUAL (c1.type, ioa::system::COMPOSE_COMPOSER_AUTOMATON_DNE);
  BOOST_CHECK (false);
}

BOOST_AUTO_TEST_CASE (system_decompose_output_automaton_dne)
{
//   ioa::system system;

//   blah* x = new blah ();
//   ioa::typed_automaton<blah> y (x);
//   ioa::timestamp<ioa::typed_automaton<blah> > a (&y);

//   x = new blah ();
//   ioa::system::create_result<blah> r1 = system.create (x);
//   BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
//   ioa::timestamp<ioa::automaton> composer = r1.automaton;

//   ioa::system::compose_result c1 = system.compose (a, &blah::output, a, &blah::input, composer);
//   BOOST_CHECK_EQUAL (c1.type, ioa::system::COMPOSE_OUTPUT_AUTOMATON_DNE);
  BOOST_CHECK (false);
}

BOOST_AUTO_TEST_CASE (system_decompose_input_automaton_dne)
{
//   ioa::system system;

//   blah* x = new blah ();
//   ioa::typed_automaton<blah> y (x);
//   ioa::timestamp<ioa::typed_automaton<blah> > a (&y);

//   x = new blah ();
//   ioa::system::create_result<blah> r1 = system.create (x);
//   BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
//   ioa::timestamp<ioa::automaton> composer = r1.automaton;

//   r1 = system.create (new blah ());
//   BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
//   ioa::timestamp<ioa::typed_automaton<blah> > output = r1.automaton;

//   ioa::system::compose_result c1 = system.compose (output, &blah::output, a, &blah::input, composer);
//   BOOST_CHECK_EQUAL (c1.type, ioa::system::COMPOSE_INPUT_AUTOMATON_DNE);
  BOOST_CHECK (false);
}

BOOST_AUTO_TEST_CASE (system_decompose_output_parameter_dne)
{
  ioa::system system;
  int parameter;
  automaton* composer_instance = new automaton ();
  automaton* output_instance = new automaton ();
  automaton* input_instance = new automaton ();

  ioa::system::create_result<automaton> r1 = system.create (composer_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton> composer = r1.automaton;
  
  r1 = system.create (output_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton> output = r1.automaton;

  r1 = system.create (input_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton> input = r1.automaton;

  ioa::system::declare_result<int> d1 = system.declare (output, &parameter);
  BOOST_CHECK_EQUAL (d1.type, ioa::system::DECLARE_SUCCESS);
  ioa::parameter_handle<int> param = d1.parameter;
  dummy_decompose_listener listener;
  ioa::system::rescind_result<int> k1 = system.rescind (output, param, listener);
  BOOST_CHECK_EQUAL (k1.type, ioa::system::RESCIND_SUCCESS);

  ioa::system::decompose_result c1 = system.decompose (output, &automaton::p_uv_output, param, input, &automaton::up_uv_input);
  BOOST_CHECK_EQUAL (c1.type, ioa::system::DECOMPOSE_OUTPUT_PARAMETER_DNE);
}

BOOST_AUTO_TEST_CASE (system_decompose_input_parameter_dne)
{
  ioa::system system;
  int parameter;
  automaton* composer_instance = new automaton ();
  automaton* output_instance = new automaton ();
  automaton* input_instance = new automaton ();

  ioa::system::create_result<automaton> r1 = system.create (composer_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton> composer = r1.automaton;
  
  r1 = system.create (output_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton> output = r1.automaton;

  r1 = system.create (input_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton> input = r1.automaton;

  ioa::system::declare_result<int> d1 = system.declare (input, &parameter);
  BOOST_CHECK_EQUAL (d1.type, ioa::system::DECLARE_SUCCESS);
  ioa::parameter_handle<int> param = d1.parameter;
  dummy_decompose_listener listener;
  ioa::system::rescind_result<int> k1 = system.rescind (input, param, listener);
  BOOST_CHECK_EQUAL (k1.type, ioa::system::RESCIND_SUCCESS);

  ioa::system::decompose_result c1 = system.decompose (output, &automaton::up_uv_output, input, &automaton::p_uv_input, param);
  BOOST_CHECK_EQUAL (c1.type, ioa::system::DECOMPOSE_INPUT_PARAMETER_DNE);
}

BOOST_AUTO_TEST_CASE (system_decompose_exists)
{
  ioa::system system;
  automaton* composer_instance = new automaton ();
  automaton* output_instance = new automaton ();
  automaton* input_instance = new automaton ();
  
  ioa::system::create_result<automaton> r1 = system.create (composer_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton> composer = r1.automaton;
  
  r1 = system.create (output_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton> output = r1.automaton;
  
  r1 = system.create (input_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton> input = r1.automaton;
  
  ioa::system::decompose_result j1 = system.decompose (output, &automaton::up_uv_output, input, &automaton::up_uv_input, composer);
  BOOST_CHECK_EQUAL (j1.type, ioa::system::DECOMPOSE_EXISTS);
}

BOOST_AUTO_TEST_CASE (system_decompose_success)
{
  ioa::system system;
  int parameter;
  automaton* composer_instance = new automaton ();
  automaton* output_instance = new automaton ();
  automaton* input_instance = new automaton ();

  ioa::system::create_result<automaton> r1 = system.create (composer_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton> composer = r1.automaton;
  
  r1 = system.create (output_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton> output = r1.automaton;

  r1 = system.create (input_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton> input = r1.automaton;

  ioa::system::declare_result<int> d1 = system.declare (output, &parameter);
  BOOST_CHECK_EQUAL (d1.type, ioa::system::DECLARE_SUCCESS);
  ioa::parameter_handle<int> param = d1.parameter;

  ioa::system::compose_result c1 = system.compose (output, &automaton::p_uv_output, param, input, &automaton::up_uv_input);
  BOOST_CHECK_EQUAL (c1.type, ioa::system::COMPOSE_SUCCESS);

  ioa::system::decompose_result j1 = system.decompose (output, &automaton::p_uv_output, param, input, &automaton::up_uv_input);
  BOOST_CHECK_EQUAL (j1.type, ioa::system::DECOMPOSE_SUCCESS);
}

BOOST_AUTO_TEST_CASE (system_rescind_automaton_dne)
{
//   ioa::system system;
//   int parameter;
  
//   blah* x = new blah ();
//   ioa::typed_automaton<blah> y (x);
//   ioa::timestamp<ioa::typed_automaton<blah> > a (&y);
//   ioa::system::declare_result d1 = system.declare (a, &parameter);
//   BOOST_CHECK_EQUAL (d1.type, ioa::system::DECLARE_AUTOMATON_DNE);
  BOOST_CHECK (false);
}

BOOST_AUTO_TEST_CASE (system_rescind_exists)
{
  ioa::system system;
  int parameter;
  
  ioa::system::create_result<automaton> r1 = system.create (new automaton ());
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::system::declare_result<int> d1 = system.declare (r1.automaton, &parameter);
  BOOST_CHECK_EQUAL (d1.type, ioa::system::DECLARE_SUCCESS);
  ioa::parameter_handle<int> param = d1.parameter;
  dummy_decompose_listener listener;
  ioa::system::rescind_result<int> k1 = system.rescind (r1.automaton, param, listener);
  BOOST_CHECK_EQUAL (k1.type, ioa::system::RESCIND_SUCCESS);
  k1 = system.rescind (r1.automaton, param, listener);
  BOOST_CHECK_EQUAL (k1.type, ioa::system::RESCIND_EXISTS);
}

BOOST_AUTO_TEST_CASE (system_rescind_success)
{
  ioa::system system;
  int parameter;
  automaton* composer_instance = new automaton ();
  automaton* output_instance = new automaton ();
  automaton* input_instance = new automaton ();

  ioa::system::create_result<automaton> r1 = system.create (composer_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton> composer = r1.automaton;
  
  r1 = system.create (output_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton> output = r1.automaton;

  r1 = system.create (input_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton> input = r1.automaton;

  ioa::system::declare_result<int> d1 = system.declare (composer, &parameter);
  BOOST_CHECK_EQUAL (d1.type, ioa::system::DECLARE_SUCCESS);
  ioa::parameter_handle<int> param = d1.parameter;

  ioa::system::compose_result c1 = system.compose (composer, &automaton::p_uv_output, param, input, &automaton::up_uv_input);
  BOOST_CHECK_EQUAL (c1.type, ioa::system::COMPOSE_SUCCESS);

  c1 = system.compose (output, &automaton::up_uv_output, composer, &automaton::p_uv_input, param);
  BOOST_CHECK_EQUAL (c1.type, ioa::system::COMPOSE_SUCCESS);

  dummy_decompose_listener listener;
  ioa::system::rescind_result<int> k1 = system.rescind (composer, param, listener);
  BOOST_CHECK_EQUAL (k1.type, ioa::system::RESCIND_SUCCESS);
  BOOST_CHECK (std::find (listener.output_records.begin (),
			  listener.output_records.end (),
			  output_decompose_record (composer,
						   &composer_instance->p_uv_output,
						   param,
						   input,
						   &input_instance->up_uv_input)) != listener.output_records.end ());
  BOOST_CHECK (std::find (listener.input_records.begin (),
			  listener.input_records.end (),
			  input_decompose_record (output,
						  &output_instance->up_uv_output,
						  composer,
						  &composer_instance->p_uv_input,
						  param)) != listener.input_records.end ());


}

BOOST_AUTO_TEST_CASE (system_execute_output_automaton_dne)
{
  // ioa::system system;
  // int parameter;
  // automaton* composer_instance = new automaton ();
  // automaton* output_instance = new automaton ();
  // automaton* input_instance = new automaton ();

  // ioa::system::create_result<automaton> r1 = system.create (composer_instance);
  // BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  // ioa::automaton_handle<automaton> composer = r1.automaton;
  
  // r1 = system.create (output_instance);
  // BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  // ioa::automaton_handle<automaton> output = r1.automaton;

  // r1 = system.create (input_instance);
  // BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  // ioa::automaton_handle<automaton> input = r1.automaton;

  // ioa::system::declare_result<int> d1 = system.declare (output, &parameter);
  // BOOST_CHECK_EQUAL (d1.type, ioa::system::DECLARE_SUCCESS);
  // ioa::parameter_handle<int> param = d1.parameter;

  // ioa::system::compose_result c1 = system.compose (output, &automaton::p_uv_output, param, input, &automaton::up_uv_input, composer);
  // BOOST_CHECK_EQUAL (c1.type, ioa::system::COMPOSE_SUCCESS);

  // ioa::system::execute_result e1 = system.execute_output (output, &automaton::p_uv_output, param);
  // BOOST_CHECK_EQUAL (e1.type, ioa::system::EXECUTE_SUCCESS);
  // BOOST_CHECK (input_instance->up_uv_input.state);
  BOOST_CHECK (false);
}

BOOST_AUTO_TEST_CASE (system_execute_output_parameter_dne)
{
  ioa::system system;
  int parameter;
  automaton* output_instance = new automaton ();

  ioa::system::create_result<automaton> r1 = system.create (output_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton> output = r1.automaton;

  ioa::system::declare_result<int> d1 = system.declare (output, &parameter);
  BOOST_CHECK_EQUAL (d1.type, ioa::system::DECLARE_SUCCESS);
  ioa::parameter_handle<int> param = d1.parameter;

  dummy_decompose_listener listener;
  ioa::system::rescind_result<int> k1 = system.rescind (r1.automaton, param, listener);
  BOOST_CHECK_EQUAL (k1.type, ioa::system::RESCIND_SUCCESS);

  ioa::system::execute_result e1 = system.execute_output (output, &automaton::p_uv_output, param);
  BOOST_CHECK_EQUAL (e1.type, ioa::system::EXECUTE_PARAMETER_DNE);
}

BOOST_AUTO_TEST_CASE (system_execute_output_success)
{
  ioa::system system;
  automaton* output_instance = new automaton ();

  ioa::system::create_result<automaton> r1 = system.create (output_instance);
  BOOST_CHECK_EQUAL (r1.type, ioa::system::CREATE_SUCCESS);
  ioa::automaton_handle<automaton> output = r1.automaton;

  ioa::system::execute_result e1 = system.execute_output (output, &automaton::up_uv_output);
  BOOST_CHECK_EQUAL (e1.type, ioa::system::EXECUTE_SUCCESS);
  BOOST_CHECK (output_instance->up_uv_output.state);
}


BOOST_AUTO_TEST_SUITE_END()
