#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE Automaton
#include <boost/test/unit_test.hpp>
#include <automaton.hpp>

BOOST_AUTO_TEST_SUITE(AutomatonSuite)

BOOST_AUTO_TEST_CASE(DefaultCtor)
{
  ioa::automaton<int> automaton;
  BOOST_CHECK (0 == automaton.get_instance());
}

BOOST_AUTO_TEST_CASE(Ctor)
{
  int* x = new int();
  ioa::automaton<int> automaton(x);
  BOOST_CHECK_EQUAL (x, automaton.get_instance());
}

BOOST_AUTO_TEST_CASE(Declare)
{
  int* x = new int();
  int* p = new int();
  ioa::automaton<int> automaton(x);
  BOOST_CHECK (!automaton.is_declared(p));
  automaton.declare(p);
  BOOST_CHECK (automaton.is_declared(p));
  delete p;
}

BOOST_AUTO_TEST_SUITE_END()
