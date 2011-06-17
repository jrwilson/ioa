#include "aba_automaton.hpp"
#include "bidirectional_network.hpp"

#include <ioa/simple_scheduler.hpp>

/*
 * AsynchSpanningTree Algorithm
 * Distributed Algorithms, p. 497
*/


int main () {
  ioa::simple_scheduler ss;
  ioa::run (ss, ioa::make_generator<bidirectional_network<aba_automaton, message_t, 5, 100, 100> > ());
  return 0;
}

