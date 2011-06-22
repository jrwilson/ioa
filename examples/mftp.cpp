#include "simple_network.hpp"
#include <ioa/global_fifo_scheduler.hpp>

int main() {
  ioa::global_fifo_scheduler sched;
  ioa::run (sched, ioa::make_generator<simple_network> ());
  return 0;
}

