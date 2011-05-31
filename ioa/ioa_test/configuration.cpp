#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE configuration
#include <boost/test/unit_test.hpp>

#include <ioa.hpp>
#include "automaton2.hpp"
#include "instance_holder.hpp"

BOOST_AUTO_TEST_SUITE(configuration_suite)

struct self_helper_destroy_before :
  public ioa::dispatching_automaton,
  public ioa::observer
{
  typedef ioa::self_helper<self_helper_destroy_before> helper_type;
  helper_type* m_helper;

  void init () {
    m_helper = new helper_type (this);
    BOOST_CHECK (ioa::scheduler.get_current_aid (this) == m_helper->get_handle ());
    m_helper->add_observer (this);
    m_helper->destroy ();
  }

  ~self_helper_destroy_before () {
    BOOST_CHECK (m_helper == 0);
  }

  void observe () {
    BOOST_CHECK (false);
  }

  void stop_observing () {
    m_helper = 0;
  }

};

BOOST_AUTO_TEST_CASE (config_self_helper_destroy_before)
{
  ioa::scheduler.run (ioa::instance_generator<self_helper_destroy_before> ());
  ioa::scheduler.clear ();
}

struct automaton_helper_destroy_after :
  public ioa::dispatching_automaton,
  public ioa::observer
{
  typedef ioa::automaton_helper<automaton_helper_destroy_after, automaton2_generator> helper_type;
  helper_type* m_helper;

  void init () {
    m_helper = new helper_type (this, automaton2_generator ());
    m_helper->add_observer (this);
  }

  ~automaton_helper_destroy_after () {
    BOOST_CHECK (m_helper == 0);
  }

  void observe () {
    BOOST_CHECK (m_helper->get_handle ().aid () != -1);
    m_helper->destroy ();
  }

  void stop_observing () {
    m_helper = 0;
  }

};

BOOST_AUTO_TEST_CASE (config_automaton_helper_destroy_after)
{
  ioa::scheduler.run (ioa::instance_generator<automaton_helper_destroy_after> ());
  ioa::scheduler.clear ();
}

struct automaton_helper_destroy_before :
  public ioa::dispatching_automaton,
  public ioa::observer
{
  typedef ioa::automaton_helper<automaton_helper_destroy_before, automaton2_generator> helper_type;
  helper_type* m_helper;

  void init () {
    m_helper = new helper_type (this, automaton2_generator ());
    m_helper->add_observer (this);
    m_helper->destroy ();
  }

  ~automaton_helper_destroy_before () {
    BOOST_CHECK (m_helper == 0);
  }

  void observe () {
    BOOST_CHECK (false);
  }

  void stop_observing () {
    m_helper = 0;
  }

};

BOOST_AUTO_TEST_CASE (config_automaton_helper_destroy_before)
{
  ioa::scheduler.run (ioa::instance_generator<automaton_helper_destroy_before> ());
  ioa::scheduler.clear ();
}

BOOST_AUTO_TEST_SUITE_END()
