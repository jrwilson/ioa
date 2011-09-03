#include <ioa/global_fifo_scheduler.hpp>

typedef unidirectional_ring_leader_election<asynch_lcr_automaton, UID_t> T;

int main (int argc,
	  char* argv[]) {
  size_t node_count;
  /* Initialize node_count. */
  srand (time (0));

  ioa::global_fifo_scheduler sched;
  ioa::run (sched, ioa::make_generator<T> (node_count));

  return 0; 
}
