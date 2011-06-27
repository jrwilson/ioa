#ifndef __weighted_bidirectional_network_hpp__
#define __weighted_bidirectional_network_hpp__

#include "channel_automaton.hpp"

#include <map>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <vector>
#include <utility>


template <class T, typename M, size_t N, unsigned long NUMERATOR, unsigned long DENOMINATOR>
class weighted_bidirectional_network :
  public ioa::automaton
{
private:

  std::vector<ioa::automaton_handle_interface<T>*> T_helpers;

  std::vector<std::set<size_t> > nbrhd;
  std::vector<std::map<size_t, size_t> > wghts;

public:
  weighted_bidirectional_network():
    nbrhd(N),
    wghts(N)
  {
    srand ((unsigned)time(0));   //initializes RNG for later calls to rand
    size_t i0 = rand() %N;
    std::cout << "The root is " << i0 << std::endl;

    assert(DENOMINATOR != 0);
    assert(NUMERATOR <= DENOMINATOR);
    double rho = double(NUMERATOR) / double(DENOMINATOR);

    //Randomly create links between nodes:
    for (size_t i=0; i<N-1; i++){
      for (size_t j = i+1; j<N; j++){
        if (rho != 0.0 && drand48() < rho){
          //Link between i and j:
          nbrhd[i].insert(j);
          nbrhd[j].insert(i);

          size_t w = rand () % N;
          wghts[i].insert(std::make_pair(j,w));
          wghts[j].insert(std::make_pair(i,w));

          std::cout << "Node " << i << " and node " << j << " are connected with weight " << w << std::endl;
        }
      }
    }

    for (size_t i=0; i<N; i++){
      T_helpers.push_back(new ioa::automaton_manager<T> (this, ioa::make_generator<T> (i, i0, nbrhd[i], wghts[i])));
    }

    for(size_t i=0; i<N; i++){
      for(size_t j = i+1; j<N; j++){
	if (nbrhd[i].count(j) != 0){
	  //Create channel automata to link i and j.
	  ioa::automaton_manager<channel_automaton<M> >* i_to_j_channel = new ioa::automaton_manager<channel_automaton<M> > (this, ioa::make_generator<channel_automaton<M> > ());
	  ioa::automaton_manager<channel_automaton<M> >* j_to_i_channel = new ioa::automaton_manager<channel_automaton<M> > (this, ioa::make_generator<channel_automaton<M> > ());
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
