#include <ioa/ioa.hpp>
#include <ioa/global_fifo_scheduler.hpp>

class null_automaton :
  public virtual ioa::automaton
{ };

int main () {
  ioa::global_fifo_scheduler sched;
  ioa::run (sched, ioa::make_generator<null_automaton> ());
  return 0;
}
