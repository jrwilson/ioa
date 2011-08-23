#include "asynch_bcast_ack_automaton.hpp"
#include "bidirectional_network.hpp"

#include <ioa/global_fifo_scheduler.hpp>

#define DEFAULT_NODE_COUNT 50
#define DEFAULT_LINK_PROBABILITY .05

int main (int argc,
	  char *argv[]) {

  if (!(argc == 1 || argc == 3)) {
    std::cerr << "Usage: asynch_bcast_ack [NODE_COUNT LINK_PROBABILITY]" << std::endl;
    exit (EXIT_FAILURE);
  }

  size_t node_count = DEFAULT_NODE_COUNT;
  double rho = DEFAULT_LINK_PROBABILITY;

  if (argc == 3) {
    node_count = atoi (argv[1]);
    rho = strtod (argv[2], 0);
  }

  srand (time (0));

  ioa::global_fifo_scheduler sched;
  ioa::run (sched, ioa::make_generator<bidirectional_network<asynch_bcast_ack_automaton, message_t> > (node_count, rho));
  return 0;
}
