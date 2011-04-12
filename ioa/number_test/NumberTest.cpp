#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE Number
#include <boost/test/unit_test.hpp>
#include <Number.hpp>

BOOST_AUTO_TEST_SUITE(NumberSuite)

BOOST_AUTO_TEST_CASE(checkPass)
{
  BOOST_CHECK_EQUAL(Number(2).add(2), Number(4));
}

BOOST_AUTO_TEST_CASE(checkFailure)
{
  BOOST_CHECK_EQUAL(Number(2).add(2), Number(5));
}

BOOST_AUTO_TEST_SUITE_END()
