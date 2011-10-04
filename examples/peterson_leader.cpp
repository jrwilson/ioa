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

#include "peterson_leader_automaton.hpp"
#include "unidirectional_ring_leader_election.hpp"

#include <ioa/global_fifo_scheduler.hpp>

#define DEFAULT_NODE_COUNT 100

int main (int argc,
	  char* argv[]) {

  if (!(argc == 1 || argc == 2)) {
    std::cerr << "Usage: peterson_leader [NODE_COUNT]" << std::endl;
    exit (EXIT_FAILURE);
  }

  size_t node_count = DEFAULT_NODE_COUNT;
  if (argc == 2) {
    node_count = atoi (argv[1]);
  }

  srand (time (0));

  ioa::global_fifo_scheduler sched;
  ioa::run (sched, ioa::make_allocator<unidirectional_ring_leader_election<peterson_leader_automaton<UID_t> > > (node_count));
  return 0; 
}
