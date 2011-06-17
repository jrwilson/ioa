#include "weighted_bidirectional_network.hpp"
#include "bf_automaton.hpp"

#include <ioa/simple_scheduler.hpp>

/*
 * AsynchBellmanFord Algorithm
 * Distributed Algorithms, p. 507
 */

int main () {
  ioa::simple_scheduler ss;
  ioa::run (ss, ioa::make_generator<weighted_bidirectional_network<bf_automaton, size_t, 10, 50, 100> > ());
  return 0;
}
