#include <ioa/ioa.hpp>
#include <ioa/simple_scheduler.hpp>

class null_automaton :
  public ioa::automaton
{ };

int main () {
  ioa::simple_scheduler ss;
  ioa::run (ss, ioa::make_generator<null_automaton> ());
  return 0;
}
