#ifndef __beast_tree_hpp__
#define __beast_tree_hpp__

#include "channel_automaton.hpp"

#include <map>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <vector>


template <class T, size_t N, unsigned long NUMERATOR, unsigned long DENOMINATOR>
class beast_tree :
  public ioa::automaton_interface
{
private:

  std::auto_ptr<ioa::self_helper<beast_tree> > self;
  std::vector<ioa::automaton_handle_interface<T>*> T_helpers;

  std::vector<std::set<size_t> > nbrhd;
  std::ofstream out;

public:
  beast_tree():
    self (new ioa::self_helper<beast_tree> ()),
    nbrhd(N){
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

	  if (debug) std::cout << "Node " << i << " and node " << j << " have a link" << std::endl;
	}
      }
    }

    for (size_t i=0; i<N; i++){
      T_helpers.push_back(new ioa::automaton_helper<T> (this, ioa::make_generator<T> (i, i0, nbrhd[i])));
    }

    for(size_t i=0; i<N; i++){
      for(size_t j = i+1; j<N; j++){
	if (nbrhd[i].count(j) != 0){
	  //Create channel automata to link i and j.
	  ioa::automaton_helper<channel_automaton<message_t> >* i_to_j_channel = new ioa::automaton_helper<channel_automaton<message_t> > (this, ioa::make_generator<channel_automaton<message_t> > ());
	  ioa::automaton_helper<channel_automaton<message_t> >* j_to_i_channel = new ioa::automaton_helper<channel_automaton<message_t> > (this, ioa::make_generator<channel_automaton<message_t> > ());
	  //Helper for send i,j:
	  make_bind_helper(this,
			   T_helpers[i],
			   &T::send,
			   j,
			   i_to_j_channel,
			   &channel_automaton<message_t>::send);
	  //Helper for send j,i:
	  make_bind_helper(this,
			   T_helpers[j],
			   &T::send,
			   i,
			   j_to_i_channel,
			   &channel_automaton<message_t>::send);
	  //Helper for receive i,j:
	  make_bind_helper(this,
			   i_to_j_channel,
			   &channel_automaton<message_t>::receive,
			   T_helpers[j],
			   &T::receive,
			   i);
	  //Helper for receive j,i:
	  make_bind_helper(this,
			   j_to_i_channel,
			   &channel_automaton<message_t>::receive,
			   T_helpers[i],
			   &T::receive,
			   j);
	}
      }
    }
    
  }


};
#endif
