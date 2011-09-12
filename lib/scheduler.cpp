#include <ioa/scheduler.hpp>

namespace ioa {

  scheduler_interface* scheduler = 0;

  aid_t get_aid () {
    assert (scheduler != 0);
    return scheduler->get_current_aid ();
  }

  void close (int fd) {
    assert (scheduler != 0);
    scheduler->close (fd);
  }

}
