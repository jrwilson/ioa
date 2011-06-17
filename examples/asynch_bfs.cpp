#include "bfs_automaton.hpp"
#include "bidirectional_network.hpp"

#include <ioa/simple_scheduler.hpp>

/*
 * AsynchSpanningTree Algorithm
 * Distributed Algorithms, p. 497
*/


int main () {
  ioa::simple_scheduler ss;
  ioa::run (ss, ioa::make_generator<bidirectional_network<bfs_automaton,size_t, 15, 30, 100> > ());
  return 0;
}
