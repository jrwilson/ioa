#include <cstdlib>
#include <iostream>

#include <ioa.hpp>
#include "automaton2.hpp"

class create_exists
{
private:
  enum state_type {
    STATE1,
    STATE2
  };
  state_type m_state;
  automaton2* m_instance;

  void init_ () {
    ioa::scheduler.create (this, m_instance);
  }

  void created_ (const ioa::system::create_result& r) {
    switch (m_state) {
    case STATE1:
      BOOST_ASSERT (r.type == ioa::system::CREATE_SUCCESS);
      ioa::scheduler.create (this, m_instance);
      m_state = STATE2;
      break;
    case STATE2:
      BOOST_ASSERT (r.type == ioa::system::CREATE_EXISTS);
      break;
    }
  }

public:
  ioa::internal_wrapper<create_exists, &create_exists::init_> init;
  ioa::system_event_wrapper<create_exists, ioa::system::create_result, &create_exists::created_> created;

public:
  create_exists () :
    m_state (STATE1),
    m_instance (new automaton2 ()),
    init (*this),
    created (*this)
  { }
};

int
main () {
  ioa::scheduler.run (new create_exists ());
  return 0; 
}
