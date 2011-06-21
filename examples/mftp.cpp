#include "simple_network.hpp"
#include "mftp_automaton.hpp"
#include "file.hpp"

#include <ioa/simple_scheduler.hpp>

using namespace std;

int main() {
  ioa::simple_scheduler ss;
  ioa::run (ss, ioa::make_generator<simple_network<mftp_automaton, message> > ());
  return 0;
}

