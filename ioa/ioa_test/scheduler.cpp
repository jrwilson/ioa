#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE scheduler
#include <boost/test/unit_test.hpp>

#include <ioa.hpp>
#include "automaton2.hpp"

BOOST_AUTO_TEST_SUITE(scheduler_suite)

class create_exists
{
public:
  enum state_type {
    CREATE1,
    CREATE2,
    STOP
  };
  state_type m_state;

private:
  automaton2* m_instance;
  void init_ () {
    ioa::scheduler.create (this, m_instance);
  }

  void created_ (const ioa::system::create_result& r) {
    switch (m_state) {
    case CREATE1:
      BOOST_CHECK_EQUAL (r.type, ioa::system::CREATE_SUCCESS);
      ioa::scheduler.create (this, m_instance);
      m_state = CREATE2;
      break;
    case CREATE2:
      BOOST_CHECK_EQUAL (r.type, ioa::system::CREATE_EXISTS);
      m_state = STOP;
      break;
    case STOP:
      BOOST_CHECK (false);
      break;
    }
  }

public:
  ioa::internal_wrapper<create_exists, &create_exists::init_> init;
  ioa::system_event_wrapper<create_exists, ioa::system::create_result, &create_exists::created_> created;

public:
  create_exists () :
    m_state (CREATE1),
    m_instance (new automaton2 ()),
    init (*this),
    created (*this)
  { }
};

BOOST_AUTO_TEST_CASE (scheduler_create_exists)
{
  create_exists* instance = new create_exists ();
  ioa::scheduler.run (instance);
  BOOST_CHECK_EQUAL (instance->m_state, create_exists::STOP);
  ioa::scheduler.clear ();
}

class create_success
{
public:
  enum state_type {
    CREATE1,
    STOP
  };
  state_type m_state;

private:
  void init_ () {
    ioa::scheduler.create (this, new automaton2 ());
  }
  
  void created_ (const ioa::system::create_result& r) {
    switch (m_state) {
    case CREATE1:
      BOOST_CHECK_EQUAL (r.type, ioa::system::CREATE_SUCCESS);
      m_state = STOP;
      break;
    case STOP:
      BOOST_CHECK (false);
      break;
    }
  }
  
public:
  ioa::internal_wrapper<create_success, &create_success::init_> init;
  ioa::system_event_wrapper<create_success, ioa::system::create_result, &create_success::created_> created;
  
public:
  create_success () :
    m_state (CREATE1),
    init (*this),
    created (*this)
  { }
};

BOOST_AUTO_TEST_CASE (scheduler_create_success)
{
  create_success* instance = new create_success ();
  ioa::scheduler.run (instance);
  BOOST_CHECK_EQUAL (instance->m_state, create_success::STOP);
  ioa::scheduler.clear ();
}

BOOST_AUTO_TEST_SUITE_END()
