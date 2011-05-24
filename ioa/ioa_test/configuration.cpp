#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE configuration
#include <boost/test/unit_test.hpp>

#include <ioa.hpp>
#include "automaton2.hpp"
#include "instance_holder.hpp"

BOOST_AUTO_TEST_SUITE(configuration_suite)

struct automaton_helper_automaton_destroyed :
  public ioa::dispatching_automaton
{
  typedef ioa::automaton_helper<automaton_helper_automaton_destroyed, automaton2_generator> helper;
  helper* m_helper;

  void transition_ () {
    if (m_helper->get_handle ().aid () != -1) {
      // Destroy once created.
      m_helper->destroy ();
      m_helper = 0;
    }
    else {
      // Poll.
      ioa::scheduler.schedule (this, &automaton_helper_automaton_destroyed::transition);
    }
  }
  ioa::internal_wrapper<automaton_helper_automaton_destroyed, &automaton_helper_automaton_destroyed::transition_> transition;

  automaton_helper_automaton_destroyed () :
    transition (*this)
  { }

  void init () {
    m_helper = new helper (this, automaton2_generator ()),
    ioa::scheduler.schedule (this, &automaton_helper_automaton_destroyed::transition);
  }

  ~automaton_helper_automaton_destroyed () {
    BOOST_CHECK (m_helper == 0);
  }
};

BOOST_AUTO_TEST_CASE (config_automaton_helper_automaton_destroyed)
{
  ioa::scheduler.run (ioa::instance_generator<automaton_helper_automaton_destroyed> ());
  ioa::scheduler.clear ();
}

struct automaton_helper_automaton_destroyed_fast :
  public ioa::dispatching_automaton
{
  typedef ioa::automaton_helper<automaton_helper_automaton_destroyed_fast, automaton2_generator> helper;
  helper* m_helper;

  void init () {
    m_helper = new helper (this, automaton2_generator ());
    m_helper->destroy ();
    m_helper = 0;
  }

  ~automaton_helper_automaton_destroyed_fast () {
    BOOST_CHECK (m_helper == 0);
  }
};

BOOST_AUTO_TEST_CASE (config_automaton_helper_automaton_destroyed_fast)
{
  ioa::scheduler.run (ioa::instance_generator<automaton_helper_automaton_destroyed_fast> ());
  ioa::scheduler.clear ();
}

struct parameter_helper_parameter_rescinded :
  public ioa::dispatching_automaton
{
  int m_parameter;
  typedef ioa::parameter_helper<parameter_helper_parameter_rescinded, int> helper;
  helper* m_helper;

  void transition_ () {
    if (m_helper->get_handle ().pid () != -1) {
      // Destroy once created.
      m_helper->rescind ();
      m_helper = 0;
    }
    else {
      // Poll.
      ioa::scheduler.schedule (this, &parameter_helper_parameter_rescinded::transition);
    }
  }
  ioa::internal_wrapper<parameter_helper_parameter_rescinded, &parameter_helper_parameter_rescinded::transition_> transition;

  parameter_helper_parameter_rescinded () :
    transition (*this)
  { }

  void init () {
    m_helper = new helper (this, &m_parameter),
    ioa::scheduler.schedule (this, &parameter_helper_parameter_rescinded::transition);
  }

  ~parameter_helper_parameter_rescinded () {
    BOOST_CHECK (m_helper == 0);
  }
};

BOOST_AUTO_TEST_CASE (config_parameter_helper_parameter_rescinded)
{
  ioa::scheduler.run (ioa::instance_generator<parameter_helper_parameter_rescinded> ());
  ioa::scheduler.clear ();
}

struct parameter_helper_parameter_rescinded_fast :
  public ioa::dispatching_automaton
{
  int m_parameter;
  typedef ioa::parameter_helper<parameter_helper_parameter_rescinded_fast, int> helper;
  helper* m_helper;

  void init () {
    m_helper = new helper (this, &m_parameter);
    m_helper->rescind ();
    m_helper = 0;
  }

  ~parameter_helper_parameter_rescinded_fast () {
    BOOST_CHECK (m_helper == 0);
  }
};

BOOST_AUTO_TEST_CASE (config_parameter_helper_parameter_rescinded_fast)
{
  ioa::scheduler.run (ioa::instance_generator<parameter_helper_parameter_rescinded_fast> ());
  ioa::scheduler.clear ();
}

BOOST_AUTO_TEST_SUITE_END()
