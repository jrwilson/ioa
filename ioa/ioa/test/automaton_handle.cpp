#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE AutomatonHandle
#include <boost/test/unit_test.hpp>
#include "../src/automaton.hpp"

BOOST_AUTO_TEST_SUITE(AutomatonHandleSuite)

BOOST_AUTO_TEST_CASE(DefaultCtor)
{
  ioa::automaton_handle<int> handle;
  BOOST_CHECK (!handle.valid());
}

BOOST_AUTO_TEST_CASE(Ctor)
{
  ioa::automaton<int>* automaton = new ioa::automaton<int>(new int());
  ioa::automaton_handle<int> handle(automaton);
  BOOST_CHECK (handle.valid());
  delete automaton;
  BOOST_CHECK (!handle.valid());
}

BOOST_AUTO_TEST_CASE(Copy)
{
  ioa::automaton<int>* automaton = new ioa::automaton<int>(new int());
  ioa::automaton_handle<int> handle(automaton);
  ioa::automaton_handle<int> handle2(handle);
  BOOST_CHECK (handle2.valid());
  delete automaton;
  BOOST_CHECK (!handle2.valid());
}

BOOST_AUTO_TEST_CASE(Assignment)
{
  ioa::automaton<int>* automaton = new ioa::automaton<int>(new int());
  ioa::automaton_handle<int> handle(automaton);
  ioa::automaton_handle<int> handle2;
  handle2 = handle;
  BOOST_CHECK (handle2.valid());
  delete automaton;
  BOOST_CHECK (!handle2.valid());
}

BOOST_AUTO_TEST_CASE(Dtor)
{
  ioa::automaton<int>* automaton = new ioa::automaton<int>(new int());
  ioa::automaton_handle<int>* handle = new ioa::automaton_handle<int>(automaton);
  BOOST_CHECK (automaton->is_handle (handle));
  delete handle;
  BOOST_CHECK (!automaton->is_handle (handle));
  delete automaton;
}

BOOST_AUTO_TEST_SUITE_END()
