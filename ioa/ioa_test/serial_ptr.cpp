#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE serial_ptr
#include <boost/test/unit_test.hpp>
#include <serial_ptr.hpp>

BOOST_AUTO_TEST_SUITE(serial_ptr_suite)

BOOST_AUTO_TEST_CASE(default_ctor)
{
  ioa::serial_ptr<int> ts;
  int* x = ts.get_ptr ();
  BOOST_CHECK (x == 0);
  BOOST_CHECK_EQUAL (ts.get_serial_number (), 0U);
  BOOST_CHECK (!ts.valid ());
}

BOOST_AUTO_TEST_CASE(ctor)
{
  int* x = new int ();
  ioa::serial_ptr<int> ts (x);
  BOOST_CHECK_EQUAL (ts.get_ptr (), x);
  BOOST_CHECK (ts.get_serial_number () != 0U);
  BOOST_CHECK (ts.valid ());
  delete x;
}

BOOST_AUTO_TEST_CASE(copy)
{
  int* x = new int ();
  ioa::serial_ptr<int> ts1 (x);
  ioa::serial_ptr<int> ts2 (ts1);
  BOOST_CHECK_EQUAL (ts2.get_ptr (), x);
  BOOST_CHECK (ts2.get_serial_number () != 0U);
  BOOST_CHECK (ts2.valid ());
  BOOST_CHECK_EQUAL (ts2.get_ptr (), ts1.get_ptr ());
  BOOST_CHECK_EQUAL (ts2.get_serial_number (), ts1.get_serial_number ());

  delete x;
}

BOOST_AUTO_TEST_CASE(assign)
{
  int* x = new int ();
  ioa::serial_ptr<int> ts1 (x);
  ioa::serial_ptr<int> ts2;
  ts2 = ts1;
  BOOST_CHECK_EQUAL (ts2.get_ptr (), x);
  BOOST_CHECK (ts2.get_serial_number () != 0U);
  BOOST_CHECK (ts2.valid ());
  BOOST_CHECK_EQUAL (ts2.get_ptr (), ts1.get_ptr ());
  BOOST_CHECK_EQUAL (ts2.get_serial_number (), ts1.get_serial_number ());

  delete x;
}

BOOST_AUTO_TEST_SUITE_END()
