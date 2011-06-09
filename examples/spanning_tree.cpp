#include "ast_automaton.hpp"
#include "bidirectional_spanning_tree.hpp"
#include <ioa/simple_scheduler.hpp>

/*
 * AsynchSpanningTree Algorithm
 * Distributed Algorithms, p. 497
*/


int main () {
  ioa::scheduler::run (ioa::make_generator<bidirectional_spanning_tree<ast_automaton, 20, 10, 100> > ());
  ioa::scheduler::clear ();
  return 0;
}

