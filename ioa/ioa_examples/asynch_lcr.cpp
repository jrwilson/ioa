#include "asynch_lcr_automaton.hpp"
#include "unidirectional_ring_leader_election.hpp"

int
main () {
  ioa::scheduler.run (ioa::instance_generator<unidirectional_ring_leader_election<asynch_lcr_automaton, 1000> > ());
  return 0; 
}
