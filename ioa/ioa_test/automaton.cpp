#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE automaton
#include <boost/test/unit_test.hpp>
#include <automaton.hpp>

BOOST_AUTO_TEST_SUITE(automaton_suite)

BOOST_AUTO_TEST_CASE(ctor)
{
  int* x = new int ();
  ioa::automaton_record<int> automaton (x);
  BOOST_CHECK_EQUAL (x, automaton.get_instance ());
  BOOST_CHECK_EQUAL (x, automaton.get_typed_instance ());
}

BOOST_AUTO_TEST_CASE(declare_parameter)
{
  int parameter;
  ioa::automaton_record<int> automaton (new int ());

  BOOST_CHECK (!automaton.parameter_exists (&parameter));
  automaton.declare_parameter (&parameter);
  BOOST_CHECK (automaton.parameter_exists (&parameter));
}


BOOST_AUTO_TEST_SUITE_END()
