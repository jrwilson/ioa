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

#ifndef __bidirectional_network_hpp__
#define __bidirectional_network_hpp__

#include "channel_automaton.hpp"

#include <map>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <vector>

template <class T, typename M>
class bidirectional_network :
  public ioa::automaton
{
private:
  std::vector<ioa::automaton_handle_interface<T>*> T_helpers;

  std::vector<std::set<size_t> > nbrhd;

public:
  bidirectional_network (const size_t N,
			 const double rho):
    nbrhd (N)
  {
    // Pick the root.
    const size_t i0 = rand() % N;
    std::cout << "Root: " << i0 << std::endl;

    //Randomly create links between nodes:
    for (size_t i=0; i<N-1; i++){
      for (size_t j = i+1; j<N; j++){
	if (rho != 0.0 && drand48() < rho){
	  //Link between i and j:
	  nbrhd[i].insert(j);
	  nbrhd[j].insert(i);
	}
      }
    }

    for (size_t i=0; i<N; i++){
      T_helpers.push_back(new ioa::automaton_manager<T> (this, ioa::make_allocator<T> (i, i0, nbrhd[i])));
    }

    for(size_t i=0; i<N; i++){
      for(size_t j = i+1; j<N; j++){
	if (nbrhd[i].count(j) != 0){
	  //Create channel automata to link i and j.
	  ioa::automaton_manager<channel_automaton<M> >* i_to_j_channel = new ioa::automaton_manager<channel_automaton<M> > (this, ioa::make_allocator<channel_automaton<M> > ());
	  ioa::automaton_manager<channel_automaton<M> >* j_to_i_channel = new ioa::automaton_manager<channel_automaton<M> > (this, ioa::make_allocator<channel_automaton<M> > ());
	  //Helper for send i,j:
	  make_binding_manager(this,
			   T_helpers[i],
			   &T::send,
			   j,
			   i_to_j_channel,
			   &channel_automaton<M>::send);
	  //Helper for send j,i:
	  make_binding_manager(this,
			   T_helpers[j],
			   &T::send,
			   i,
			   j_to_i_channel,
			   &channel_automaton<M>::send);
	  //Helper for receive i,j:
	  make_binding_manager(this,
			   i_to_j_channel,
			   &channel_automaton<M>::receive,
			   T_helpers[j],
			   &T::receive,
			   i);
	  //Helper for receive j,i:
	  make_binding_manager(this,
			   j_to_i_channel,
			   &channel_automaton<M>::receive,
			   T_helpers[i],
			   &T::receive,
			   j);
	}
      }
    }
    
  }


};
#endif
