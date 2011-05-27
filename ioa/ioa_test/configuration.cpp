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
  helper m_helper;

  UP_INTERNAL (automaton_helper_automaton_destroyed, transition) {
    if (m_helper.get_handle ().aid () != -1) {
      // Destroy once created.
      m_helper.destroy ();
    }
    else {
      // Poll.
      ioa::scheduler.schedule (this, &automaton_helper_automaton_destroyed::transition);
    }
  }

  automaton_helper_automaton_destroyed () :
    m_helper (this, automaton2_generator ()),
    ACTION (automaton_helper_automaton_destroyed, transition)
  { }

  void init () {
    m_helper.create ();
    ioa::scheduler.schedule (this, &automaton_helper_automaton_destroyed::transition);
  }

  ~automaton_helper_automaton_destroyed () {
    BOOST_CHECK (m_helper.get_handle ().aid () == -1);
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
  helper m_helper;


  automaton_helper_automaton_destroyed_fast () :
    m_helper (this, automaton2_generator ())
  { }

  void init () {
    m_helper.create ();
    m_helper.destroy ();
  }

  ~automaton_helper_automaton_destroyed_fast () {
    BOOST_CHECK (m_helper.get_handle ().aid () == -1);
  }
};

BOOST_AUTO_TEST_CASE (config_automaton_helper_automaton_destroyed_fast)
{
  ioa::scheduler.run (ioa::instance_generator<automaton_helper_automaton_destroyed_fast> ());
  ioa::scheduler.clear ();
}

BOOST_AUTO_TEST_SUITE_END()
