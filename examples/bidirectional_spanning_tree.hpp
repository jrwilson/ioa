#ifndef __bidirectional_spanning_tree_hpp__
#define __bidirectional_spanning_tree_hpp__

#include "channel_automaton.hpp"


#include <cstdlib>
#include <iostream>
#include <vector>

template <class T, size_t N, unsigned long NUMERATOR, unsigned long DENOMINATOR>
class bidirectional_spanning_tree :
  public ioa::dispatching_automaton
{
private:
  V_P_INPUT (bidirectional_spanning_tree, parent, size_t, j, size_t, i) {
    std::cout << "The parent of " << i << " is " << j << std::endl;
  }

  //self and automaton helpers
  //self helper for the class it is in (bidirectional_spanning_tree)
  typedef ioa::self_helper<bidirectional_spanning_tree> bidirectional_spanning_tree_helper_type;
  //automaton helper for the automatons of type T from the template, which in this case will be ast_automata
  typedef ioa::automaton_helper<bidirectional_spanning_tree, T> T_helper_type;
  //automaton helper for channel automata
  typedef ioa::automaton_helper<bidirectional_spanning_tree, channel_automaton<search_t> > channel_automaton_helper_type;

  typedef ioa::bind_helper<bidirectional_spanning_tree, T_helper_type, typename T::send_type, channel_automaton_helper_type, channel_automaton<search_t>::send_type> send_bind_helper_type;
  typedef ioa::bind_helper<bidirectional_spanning_tree, channel_automaton_helper_type, channel_automaton<search_t>::receive_type, T_helper_type, typename T::receive_type> receive_bind_helper_type;
  typedef ioa::bind_helper<bidirectional_spanning_tree, T_helper_type, typename T::parent_type, bidirectional_spanning_tree_helper_type, typename bidirectional_spanning_tree::parent_type> parent_bind_helper_type;

  bidirectional_spanning_tree_helper_type* bidirectional_spanning_tree_helper;

  std::vector<T_helper_type*> T_helpers;

public:
  bidirectional_spanning_tree () :
    ACTION (bidirectional_spanning_tree, parent)
  { }

  void init () {
    srand ((unsigned)time(0));
    size_t i0 = rand() % N;
    std::cout << "The root of the spanning tree is " << i0 << std::endl;
    bidirectional_spanning_tree_helper = new bidirectional_spanning_tree_helper_type (this);
    std::vector<std::set<size_t> > nbrhd(N); //nbrhd = neighborhood.  Collection of neighboring automata

    assert(DENOMINATOR != 0);
    assert(NUMERATOR <= DENOMINATOR);
    double rho = double(NUMERATOR) / double(DENOMINATOR);
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
      T_helpers.push_back (new T_helper_type (this, ioa::make_generator<T> (i, i0, nbrhd[i])));
      new parent_bind_helper_type (this,
                                   T_helpers[i],
                                   &T::parent,
                                   bidirectional_spanning_tree_helper,
                                   &bidirectional_spanning_tree::parent,
                                   i);
    }

    //Generate random network structure for the automata and bind channel automata to them
    for (size_t i = 0; i < N - 1; i++) {
      for(size_t j = i + 1; j < N; j++) {

        if(nbrhd[i].count(j) != 0) {
          // Link between i and j.
          channel_automaton_helper_type* i_to_j_channel = new channel_automaton_helper_type (this, ioa::make_generator<channel_automaton<search_t> > ());
          channel_automaton_helper_type* j_to_i_channel = new channel_automaton_helper_type (this, ioa::make_generator<channel_automaton<search_t> > ());

          new send_bind_helper_type (this,
                        T_helpers[i],
                        &T::send,
                        j,
                        i_to_j_channel,
                        &channel_automaton<search_t>::send);

          new receive_bind_helper_type (this,
                        i_to_j_channel,
                        &channel_automaton<search_t>::receive,
                        T_helpers[j],
                        &T::receive,
                        i);

          new send_bind_helper_type (this,
                        T_helpers[j],
                        &T::send,
                        i,
                        j_to_i_channel,
                        &channel_automaton<search_t>::send);

          new receive_bind_helper_type (this,
                        j_to_i_channel,
                        &channel_automaton<search_t>::receive,
                        T_helpers[i],
                        &T::receive,
                        j);

        }
      }
    }
  }
};


#endif
