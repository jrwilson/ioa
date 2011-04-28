#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE AutomatonHandle
#include <boost/test/unit_test.hpp>
#include <automaton.hpp>
#include "system.hpp"

BOOST_AUTO_TEST_SUITE(AutomatonHandleSuite)

BOOST_AUTO_TEST_CASE(DefaultCtor)
{
  ioa::automaton_handle<int> handle;
  BOOST_CHECK (!handle.valid());
}

BOOST_AUTO_TEST_CASE(Ctor)
{
  ioa::typed_automaton<int>* automaton = new ioa::typed_automaton<int>(0, ioa::null_sys, new int());
  ioa::automaton_handle<int> handle(automaton);
  BOOST_CHECK (handle.valid());
  delete automaton;
  BOOST_CHECK (!handle.valid());
}

BOOST_AUTO_TEST_CASE(Copy)
{
  ioa::typed_automaton<int>* automaton = new ioa::typed_automaton<int>(0, ioa::null_sys, new int());
  ioa::automaton_handle<int> handle(automaton);
  ioa::automaton_handle<int> handle2(handle);
  BOOST_CHECK (handle2.valid());
  delete automaton;
  BOOST_CHECK (!handle2.valid());
}

BOOST_AUTO_TEST_CASE(Assignment)
{
  ioa::typed_automaton<int>* automaton = new ioa::typed_automaton<int>(0, ioa::null_sys, new int());
  ioa::automaton_handle<int> handle(automaton);
  ioa::automaton_handle<int> handle2;
  handle2 = handle;
  BOOST_CHECK (handle2.valid());
  delete automaton;
  BOOST_CHECK (!handle2.valid());
}

BOOST_AUTO_TEST_CASE(Dtor)
{
  ioa::typed_automaton<int>* automaton = new ioa::typed_automaton<int>(0, ioa::null_sys, new int());
  ioa::automaton_handle<int>* handle = new ioa::automaton_handle<int>(automaton);
  BOOST_CHECK (automaton->is_handle (handle));
  delete handle;
  BOOST_CHECK (!automaton->is_handle (handle));
  delete automaton;
}

BOOST_AUTO_TEST_SUITE_END()
