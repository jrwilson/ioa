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

#ifndef __bidirectional_spanning_tree_hpp__
#define __bidirectional_spanning_tree_hpp__

#include "channel_automaton.hpp"

#include <map>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <vector>

template <class T, class M>
class bidirectional_spanning_tree :
  public ioa::automaton
{
private:
  void parent_effect(const size_t& j, size_t i) {
    parents.insert(std::make_pair(i,j));
    std::cout << i << " -> " << j << std::endl;
  }

  void parent_schedule (size_t) const { }

  V_P_INPUT (bidirectional_spanning_tree, parent, size_t, size_t);

  ioa::handle_manager<bidirectional_spanning_tree> self;
  std::vector<ioa::automaton_handle_interface<T>*> T_helpers;

  std::vector<std::set<size_t> > nbrhd; //nbrhd = neighborhood.  Collection of neighboring automata
  std::map<size_t, size_t> parents;

public:
  bidirectional_spanning_tree (const size_t N,
			       const double rho) :
    self (ioa::get_aid ()),
    nbrhd (N)
  {
    size_t i0 = rand() % N;
    std::cout << "Root: " << i0 << std::endl;

    for (size_t i = 0; i < N - 1; i++) {
      for(size_t j = i + 1; j < N; j++) {
        if(rho != 0.0 && drand48() < rho) {
          // Link between i and j.
          nbrhd[i].insert(j);
          nbrhd[j].insert(i);
        }
      }
    }

    for (size_t i = 0; i < N; ++i) {
      T_helpers.push_back (new ioa::automaton_manager<T> (this, ioa::make_generator<T> (i, i0, nbrhd[i])));
      make_binding_manager (this,
			T_helpers[i],
			&T::parent,
			&self,
			&bidirectional_spanning_tree::parent,
			i);
    }

    //Generate random network structure for the automata and bind channel automata to them
    for (size_t i = 0; i < N - 1; i++) {
      for (size_t j = i + 1; j < N; j++) {

        if (nbrhd[i].count (j) != 0) {
          // Link between i and j.
          ioa::automaton_manager<channel_automaton<M> >* i_to_j_channel = new ioa::automaton_manager<channel_automaton<M> > (this, ioa::make_generator<channel_automaton<M> > ());
          ioa::automaton_manager<channel_automaton<M> >* j_to_i_channel = new ioa::automaton_manager<channel_automaton<M> > (this, ioa::make_generator<channel_automaton<M> > ());

          make_binding_manager (this,
                            T_helpers[i],
                           &T::send,
                           j,
                           i_to_j_channel,
                           &channel_automaton<M>::send);

          make_binding_manager (this,
                             T_helpers[j],
                             &T::send,
                             i,
                             j_to_i_channel,
                             &channel_automaton<M>::send);


          make_binding_manager (this,
                            i_to_j_channel,
                            &channel_automaton<M>::receive,
                            T_helpers[j],
                            &T::receive,
                            i);

          make_binding_manager (this,
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
