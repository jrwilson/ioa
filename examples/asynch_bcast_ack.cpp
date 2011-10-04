/*
   Copyright 2011 Justin R. Wilson

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

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
  ioa::run (sched, ioa::make_allocator<bidirectional_network<asynch_bcast_ack_automaton, message_t> > (node_count, rho));
  return 0;
}
