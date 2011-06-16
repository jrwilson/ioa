#include "aba_automaton.hpp"
#include "beast_tree.hpp"

#include <ioa/simple_scheduler.hpp>

/*
 * AsynchSpanningTree Algorithm
 * Distributed Algorithms, p. 497
*/


int main () {
  ioa::simple_scheduler ss;
  ioa::run (ss, ioa::make_generator<beast_tree<aba_automaton, 5, 100, 100> > ());
  return 0;
}

