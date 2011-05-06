#include <cstdlib>
#include <iostream>

#include <ioa.hpp>
#include "automaton2.hpp"

class create_success
{
private:
  void init_ () {
    ioa::scheduler.create (this, new automaton2 ());
  }

  void created_ (const ioa::system::create_result& r) {
    BOOST_ASSERT (r.type == ioa::system::CREATE_SUCCESS);
  }

public:
  ioa::internal_wrapper<create_success, &create_success::init_> init;
  ioa::system_event_wrapper<create_success, ioa::system::create_result, &create_success::created_> created;

public:
  create_success () :
    init (*this),
    created (*this)
  { }
};

int
main () {
  ioa::scheduler.run (new create_success ());
  return 0; 
}
