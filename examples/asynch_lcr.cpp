#include "asynch_lcr_automaton.hpp"
#include "unidirectional_ring_leader_election.hpp"

#include <ioa/simple_scheduler.hpp>

int
main () {
  ioa::scheduler::run (ioa::make_generator<unidirectional_ring_leader_election<asynch_lcr_automaton, 1000> > ());
  return 0; 
}
