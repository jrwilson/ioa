#include <ioa/ioa.hpp>
#include <ioa/global_fifo_scheduler.hpp>

class null_automaton :
  public ioa::automaton
{ };

int main () {
  ioa::global_fifo_scheduler sched;
  ioa::run (sched, ioa::make_allocator<null_automaton> ());
  return 0;
}
