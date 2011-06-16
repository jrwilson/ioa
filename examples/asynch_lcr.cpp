#include "asynch_lcr_automaton.hpp"
#include "unidirectional_ring_leader_election.hpp"

#include <ioa/simple_scheduler.hpp>

int
main () {
  ioa::simple_scheduler ss;
  ioa::run (ss, ioa::make_generator<unidirectional_ring_leader_election<asynch_lcr_automaton, 1000> > ());
  return 0; 
}
