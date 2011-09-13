#include "asynch_lcr_automaton.hpp"
#include "unidirectional_ring_leader_election.hpp"

#include <ioa/global_fifo_scheduler.hpp>

#define DEFAULT_NODE_COUNT 100

int main (int argc,
	  char* argv[]) {

  if (!(argc == 1 || argc == 2)) {
    std::cerr << "Usage: async_lcr [NODE_COUNT]" << std::endl;
    exit (EXIT_FAILURE);
  }

  size_t node_count = DEFAULT_NODE_COUNT;
  if (argc == 2) {
    node_count = atoi (argv[1]);
  }

  srand (time (0));

  ioa::global_fifo_scheduler sched;
  ioa::run (sched, ioa::make_generator<unidirectional_ring_leader_election<asynch_lcr_automaton<UID_t> > > (node_count));
  return 0; 
}
