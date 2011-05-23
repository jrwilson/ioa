#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE action
#include <boost/test/unit_test.hpp>

#include "automaton1.hpp"

BOOST_AUTO_TEST_SUITE(action_suite)

BOOST_AUTO_TEST_CASE(unparameterized_unvalued_input_action)
{
  std::auto_ptr<automaton1> z (new automaton1 ());
  ioa::automaton_handle<automaton1> h;
  ioa::concrete_action<automaton1, automaton1::up_uv_input_action> action (ioa::make_action (h, &automaton1::up_uv_input), z.get ());
  BOOST_CHECK_EQUAL (h.aid (), action.get_aid ());
  BOOST_CHECK_EQUAL (z.get (), action.get_instance ());
  BOOST_CHECK_EQUAL (-1, action.get_pid ());
  BOOST_CHECK (action == action);
  action ();
  BOOST_CHECK (z->up_uv_input.state);
  action.bound ();
  BOOST_CHECK (z->up_uv_input.bound_);
  action.unbound ();
  BOOST_CHECK (z->up_uv_input.unbound_);
}

BOOST_AUTO_TEST_CASE(parameterized_unvalued_input_action)
{
  std::auto_ptr<automaton1> z (new automaton1 ());
  ioa::automaton_handle<automaton1> h;
  int parameter;
  ioa::parameter_handle<int> p;
  ioa::concrete_action<automaton1, automaton1::p_uv_input_action> action (ioa::make_action (h, &automaton1::p_uv_input, p), z.get (), &parameter);
  BOOST_CHECK_EQUAL (h.aid (), action.get_aid ());
  BOOST_CHECK_EQUAL (z.get (), action.get_instance ());
  BOOST_CHECK_EQUAL (p.pid (), action.get_pid ());
  BOOST_CHECK (action == action);
  action ();
  BOOST_CHECK (z->p_uv_input.state);
  BOOST_CHECK_EQUAL (&parameter, z->p_uv_input.last_parameter);
  action.bound ();
  BOOST_CHECK (z->p_uv_input.bound_);
  BOOST_CHECK_EQUAL (&parameter, z->p_uv_input.bound_parameter);
  action.unbound ();
  BOOST_CHECK (z->p_uv_input.unbound_);
  BOOST_CHECK_EQUAL (&parameter, z->p_uv_input.unbound_parameter);
}

BOOST_AUTO_TEST_CASE(unparameterized_valued_input_action)
{
  std::auto_ptr<automaton1> z (new automaton1 ());
  ioa::automaton_handle<automaton1> h;
  ioa::concrete_action<automaton1, automaton1::up_v_input_action> action (ioa::make_action (h, &automaton1::up_v_input), z.get ());
  BOOST_CHECK_EQUAL (h.aid (), action.get_aid ());
  BOOST_CHECK_EQUAL (z.get (), action.get_instance ());
  BOOST_CHECK_EQUAL (-1, action.get_pid ());
  BOOST_CHECK (action == action);
  action (9845);
  BOOST_CHECK_EQUAL (9845, z->up_v_input.value);
  action.bound ();
  BOOST_CHECK (z->up_v_input.bound_);
  action.unbound ();
  BOOST_CHECK (z->up_v_input.unbound_);
}

BOOST_AUTO_TEST_CASE(parameterized_valued_input_action)
{
  std::auto_ptr<automaton1> z (new automaton1 ());
  ioa::automaton_handle<automaton1> h;
  int parameter;
  ioa::parameter_handle<int> p;
  ioa::concrete_action<automaton1, automaton1::p_v_input_action> action (ioa::make_action (h, &automaton1::p_v_input, p), z.get (), &parameter);
  BOOST_CHECK_EQUAL (h.aid (), action.get_aid ());
  BOOST_CHECK_EQUAL (z.get (), action.get_instance ());
  BOOST_CHECK_EQUAL (p.pid (), action.get_pid ());
  BOOST_CHECK (action == action);
  action (9845);
  BOOST_CHECK_EQUAL (9845, z->p_v_input.value);
  BOOST_CHECK_EQUAL (&parameter, z->p_v_input.last_parameter);
  action.bound ();
  BOOST_CHECK (z->p_v_input.bound_);
  BOOST_CHECK_EQUAL (&parameter, z->p_v_input.bound_parameter);
  action.unbound ();
  BOOST_CHECK (z->p_v_input.unbound_);
  BOOST_CHECK_EQUAL (&parameter, z->p_v_input.unbound_parameter);
}

BOOST_AUTO_TEST_CASE(unparameterized_unvalued_output_action)
{
  std::auto_ptr<automaton1> z (new automaton1 ());
  ioa::automaton_handle<automaton1> h;
  ioa::concrete_action<automaton1, automaton1::up_uv_output_action> action (ioa::make_action (h, &automaton1::up_uv_output), z.get ());
  BOOST_CHECK_EQUAL (h.aid (), action.get_aid ());
  BOOST_CHECK_EQUAL (z.get (), action.get_instance ());
  BOOST_CHECK_EQUAL (-1, action.get_pid ());
  BOOST_CHECK (action == action);
  bool r = action ();
  BOOST_CHECK (r);
  BOOST_CHECK (z->up_uv_output.state);
  action.bound ();
  BOOST_CHECK (z->up_uv_output.bound_);
  action.unbound ();
  BOOST_CHECK (z->up_uv_output.unbound_);
}

BOOST_AUTO_TEST_CASE(parameterized_unvalued_output_action)
{
  std::auto_ptr<automaton1> z (new automaton1 ());
  ioa::automaton_handle<automaton1> h;
  int parameter;
  ioa::parameter_handle<int> p;
  ioa::concrete_action<automaton1, automaton1::p_uv_output_action> action (ioa::make_action (h, &automaton1::p_uv_output, p), z.get (), &parameter);
  BOOST_CHECK_EQUAL (h.aid (), action.get_aid ());
  BOOST_CHECK_EQUAL (z.get (), action.get_instance ());
  BOOST_CHECK_EQUAL (p.pid (), action.get_pid ());
  BOOST_CHECK (action == action);
  bool r = action ();
  BOOST_CHECK (r);
  BOOST_CHECK (z->p_uv_output.state);
  BOOST_CHECK_EQUAL (&parameter, z->p_uv_output.last_parameter);
  action.bound ();
  BOOST_CHECK (z->p_uv_output.bound_);
  BOOST_CHECK_EQUAL (&parameter, z->p_uv_output.bound_parameter);
  action.unbound ();
  BOOST_CHECK (z->p_uv_output.unbound_);
  BOOST_CHECK_EQUAL (&parameter, z->p_uv_output.unbound_parameter);
}

BOOST_AUTO_TEST_CASE(unparameterized_valued_output_action)
{
  std::auto_ptr<automaton1> z (new automaton1 ());
  ioa::automaton_handle<automaton1> h;
  ioa::concrete_action<automaton1, automaton1::up_v_output_action> action (ioa::make_action (h, &automaton1::up_v_output), z.get ());
  BOOST_CHECK_EQUAL (h.aid (), action.get_aid ());
  BOOST_CHECK_EQUAL (z.get (), action.get_instance ());
  BOOST_CHECK_EQUAL (-1, action.get_pid ());
  BOOST_CHECK (action == action);
  std::pair<bool, int> r = action ();
  BOOST_CHECK (r.first);
  BOOST_CHECK_EQUAL (r.second, 9845);
  BOOST_CHECK (z->up_v_output.state);
  action.bound ();
  BOOST_CHECK (z->up_v_output.bound_);
  action.unbound ();
  BOOST_CHECK (z->up_v_output.unbound_);
}

BOOST_AUTO_TEST_CASE(parameterized_valued_output_action)
{
  std::auto_ptr<automaton1> z (new automaton1 ());
  ioa::automaton_handle<automaton1> h;
  int parameter;
  ioa::parameter_handle<int> p;
  ioa::concrete_action<automaton1, automaton1::p_v_output_action> action (ioa::make_action (h, &automaton1::p_v_output, p), z.get (), &parameter);
  BOOST_CHECK_EQUAL (h.aid (), action.get_aid ());
  BOOST_CHECK_EQUAL (z.get (), action.get_instance ());
  BOOST_CHECK_EQUAL (p.pid (), action.get_pid ());
  BOOST_CHECK (action == action);
  std::pair<bool, int> r = action ();
  BOOST_CHECK (r.first);
  BOOST_CHECK_EQUAL (r.second, 9845);
  BOOST_CHECK (z->p_v_output.state);
  BOOST_CHECK_EQUAL (&parameter, z->p_v_output.last_parameter);
  action.bound ();
  BOOST_CHECK (z->p_v_output.bound_);
  BOOST_CHECK_EQUAL (&parameter, z->p_v_output.bound_parameter);
  action.unbound ();
  BOOST_CHECK (z->p_v_output.unbound_);
  BOOST_CHECK_EQUAL (&parameter, z->p_v_output.unbound_parameter);
}

BOOST_AUTO_TEST_CASE(unparameterized_internal_action)
{
  std::auto_ptr<automaton1> z (new automaton1 ());
  ioa::automaton_handle<automaton1> h;
  ioa::concrete_action<automaton1, automaton1::up_internal_action> action (ioa::make_action (h, &automaton1::up_internal), z.get ());
  BOOST_CHECK_EQUAL (h.aid (), action.get_aid ());
  BOOST_CHECK_EQUAL (z.get (), action.get_instance ());
  BOOST_CHECK_EQUAL (-1, action.get_pid ());
  BOOST_CHECK (action == action);
  action.execute ();
  BOOST_CHECK (z->up_internal.state);
}

BOOST_AUTO_TEST_CASE(parameterized_internal_action)
{
  std::auto_ptr<automaton1> z (new automaton1 ());
  ioa::automaton_handle<automaton1> h;
  int parameter;
  ioa::parameter_handle<int> p;
  ioa::concrete_action<automaton1, automaton1::p_internal_action> action (ioa::make_action (h, &automaton1::p_internal, p), z.get (), &parameter);
  BOOST_CHECK_EQUAL (h.aid (), action.get_aid ());
  BOOST_CHECK_EQUAL (z.get (), action.get_instance ());
  BOOST_CHECK_EQUAL (p.pid (), action.get_pid ());
  BOOST_CHECK (action == action);
  action.execute ();
  BOOST_CHECK (z->p_internal.state);
  BOOST_CHECK_EQUAL (&parameter, z->p_internal.last_parameter);
}

BOOST_AUTO_TEST_CASE(unvalued_event_action)
{
  std::auto_ptr<automaton1> z (new automaton1 ());
  ioa::automaton_handle<automaton1> h;
  ioa::concrete_action<automaton1, automaton1::uv_event_action> action (ioa::make_action (h, &automaton1::uv_event), z.get ());
  BOOST_CHECK_EQUAL (h.aid (), action.get_aid ());
  BOOST_CHECK_EQUAL (z.get (), action.get_instance ());
  BOOST_CHECK_EQUAL (-1, action.get_pid ());
  BOOST_CHECK (action != action);
  action.execute ();
  BOOST_CHECK (z->uv_event.state);
}

BOOST_AUTO_TEST_CASE(valued_event_action)
{
  std::auto_ptr<automaton1> z (new automaton1 ());
  ioa::automaton_handle<automaton1> h;
  ioa::concrete_action<automaton1, automaton1::v_event_action> action (ioa::make_action (h, &automaton1::v_event, 9845), z.get ());
  BOOST_CHECK_EQUAL (h.aid (), action.get_aid ());
  BOOST_CHECK_EQUAL (z.get (), action.get_instance ());
  BOOST_CHECK_EQUAL (-1, action.get_pid ());
  BOOST_CHECK (action != action);
  action.execute ();
  BOOST_CHECK (z->v_event.state);
  BOOST_CHECK_EQUAL (z->v_event.last_value, 9845);
}

BOOST_AUTO_TEST_SUITE_END()
