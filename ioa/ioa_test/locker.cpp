#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE locker
#include <boost/test/unit_test.hpp>
#include <locker.hpp>

BOOST_AUTO_TEST_SUITE(locker_suite)

BOOST_AUTO_TEST_CASE(ctor)
{
  ioa::locker<float> l;
  BOOST_CHECK (l.empty ());
  BOOST_CHECK_EQUAL (l.size (), 0U);
}

BOOST_AUTO_TEST_CASE(insert)
{
  ioa::locker<float> l;
  ioa::locker_key<float> k = l.insert (3.14f);
  BOOST_CHECK (!l.empty ());
  BOOST_CHECK_EQUAL (l.size (), 1U);
  BOOST_CHECK (l.contains (3.14));
  BOOST_CHECK (l.contains (k));
}

BOOST_AUTO_TEST_CASE(erase)
{
  ioa::locker<float> l;
  ioa::locker_key<float> k = l.insert (3.14f);
  l.erase (k);
  BOOST_CHECK (l.empty ());
  BOOST_CHECK_EQUAL (l.size (), 0U);
  BOOST_CHECK (!l.contains (3.14));
  BOOST_CHECK (!l.contains (k));
}

BOOST_AUTO_TEST_CASE(clear)
{
  ioa::locker<float> l;
  ioa::locker_key<float> k = l.insert (3.14f);
  l.clear ();
  BOOST_CHECK (l.empty ());
  BOOST_CHECK_EQUAL (l.size (), 0U);
  BOOST_CHECK (!l.contains (3.14));
  BOOST_CHECK (!l.contains (k));
}

BOOST_AUTO_TEST_SUITE_END()
